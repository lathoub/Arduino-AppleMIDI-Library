parserReturn decodeMidiSection(uint8_t rtpMidi_Flags, Deque<byte, Settings::MaxBufferSize> &buffer, uint16_t commandLength, size_t i)
{
    int cmdCount = 0;

    uint8_t runningstatus = 0;

    printBuffer(buffer);
    
    /* Multiple MIDI-commands might follow - the exact number can only be discovered by really decoding the commands! */
    while (commandLength)
    {

        /* for the first command we only have a delta-time if Z-Flag is set */
        if ((cmdCount) || (rtpMidi_Flags & RTP_MIDI_CS_FLAG_Z))
        {
            auto consumed = decodeTime(buffer, i);

            buffer.erase(i, i + consumed - 1);

            commandLength -= consumed;
            updateCommandLength(buffer, commandLength);
  
            printBuffer(buffer);
            
            if (commandLength > 0 && i >= buffer.size())
                return parserReturn::NotEnoughData;
        }

        if (commandLength > 0)
        {
      //      assert(i < buffer.size());
            
            /* Decode a MIDI-command - if 0 is returned something went wrong */
            size_t consumed = decodeMidi(buffer, i, commandLength, runningstatus);
            if (consumed == 0)
            {
                E_DEBUG_PRINTLN(F("decodeMidi indicates it did not consumed bytes"));
                E_DEBUG_PRINT(F("decodeMidi commandLength is "));
                E_DEBUG_PRINTLN(commandLength);
            }
            else if (consumed > buffer.size())
            {
                // sysex split in decodeMidi
                return parserReturn::NotEnoughData;
            }
            
            buffer.erase(i, i + consumed - 1);
            
          //  assert(consumed <= commandLength); // can't consume more than given
            
            commandLength -= consumed;
            updateCommandLength(buffer, commandLength);

            printBuffer(buffer);

            if (commandLength > 0 && i >= buffer.size())
                return parserReturn::NotEnoughData;

            cmdCount++;
        }
    }
    
    return parserReturn::Processed;
}

size_t decodeTime(Deque<byte, Settings::MaxBufferSize> &buffer, size_t i)
{
    uint8_t consumed = 0;
    uint32_t deltatime = 0;

    /* RTP-MIDI deltatime is "compressed" using only the necessary amount of octets */
    for (uint8_t j = 0; j < 4; j++)
    {
        uint8_t octet = buffer[i + consumed];
        deltatime = (deltatime << 7) | (octet & RTP_MIDI_DELTA_TIME_OCTET_MASK);
        consumed++;

        if ((octet & RTP_MIDI_DELTA_TIME_EXTENSION) == 0)
            break;
    }
    return consumed;
}

size_t decodeMidi(Deque<byte, Settings::MaxBufferSize> &buffer, size_t i, size_t cmd_len, uint8_t &runningstatus)
{
    size_t consumed = 0;

    auto octet = buffer[i];
    bool using_rs;

    /* midi realtime-data -> one octet  -- unlike serial-wired MIDI realtime-commands in RTP-MIDI will
     * not be intermingled with other MIDI-commands, so we handle this case right here and return */
    if (octet >= 0xf8)
    {
        return 1;
    }

    /* see if this first octet is a status message */
    if ((octet & RTP_MIDI_COMMAND_STATUS_FLAG) == 0)
    {
        /* if we have no running status yet -> error */
        if (((runningstatus)&RTP_MIDI_COMMAND_STATUS_FLAG) == 0)
        {
            return 0;
        }
        /* our first octet is "virtual" coming from a preceding MIDI-command,
         * so actually we have not really consumed anything yet */
        octet = runningstatus;
        using_rs = true;
    }
    else
    {
        /* We have a "real" status-byte */
        using_rs = false;
        /* Let's see how this octet influences our running-status */
        /* if we have a "normal" MIDI-command then the new status replaces the current running-status */
        if (octet < 0xf0)
        {
            runningstatus = octet;
        }
        else
        {
            /* system-realtime-commands maintain the current running-status
             * other system-commands clear the running-status, since we
             * already handled realtime, we can reset it here */
            runningstatus = 0;
        }
        consumed++;
    }

    /* non-system MIDI-commands encode the command in the high nibble and the channel
     * in the low nibble - so we will take care of those cases next */
    if (octet < 0xf0)
    {
        uint8_t type = (octet & 0xf0);
#ifdef DEBUG
        uint8_t channel = (octet & 0x0f) + 1;
#endif
        
        switch (type)
        {
        case MIDI_NAMESPACE::MidiType::NoteOff:
            consumed += 2;
            break;
        case MIDI_NAMESPACE::MidiType::NoteOn:
            consumed += 2;
            break;
        case MIDI_NAMESPACE::MidiType::AfterTouchPoly:
            consumed += 2;
            break;
        case MIDI_NAMESPACE::MidiType::ControlChange:
            consumed += 2;
            break;
        case MIDI_NAMESPACE::MidiType::ProgramChange:
            consumed += 1;
            break;
        case MIDI_NAMESPACE::MidiType::AfterTouchChannel:
            consumed += 1;
            break;
        case MIDI_NAMESPACE::MidiType::PitchBend:
            consumed += 2;
            break;
        }

        #if DEBUG >= LOG_LEVEL_TRACE
            T_DEBUG_PRINT(F("MIDI data, consumed: "));
            T_DEBUG_PRINT(consumed);
            T_DEBUG_PRINT(F(" "));
            for (auto j = 0; j < consumed; j++)
            {
                T_DEBUG_PRINT(" 0x");
                T_DEBUG_PRINT(buffer[i + j], HEX);
            }
            T_DEBUG_PRINTLN();
        #endif
            
        for (size_t j = 0; j < consumed; j++)
            session->ReceivedMidi(buffer[i + j]);

        return consumed;
    }

    /* Here we catch the remaining system-common commands */
    switch (octet)
    {
    case MIDI_NAMESPACE::MidiType::SystemExclusiveStart:
    case MIDI_NAMESPACE::MidiType::SystemExclusiveEnd:
        consumed = decodeMidiSysEx(buffer, i, cmd_len);
        if (consumed > buffer.max_size())
            return consumed;
        break;
    case MIDI_NAMESPACE::MidiType::TimeCodeQuarterFrame:
        consumed += 1;
        break;
    case MIDI_NAMESPACE::MidiType::SongPosition:
        consumed += 2;
        break;
    case MIDI_NAMESPACE::MidiType::SongSelect:
        consumed += 1;
        break;
    case MIDI_NAMESPACE::MidiType::TuneRequest:
        break;
    }

#if DEBUG >= LOG_LEVEL_TRACE
    T_DEBUG_PRINT(F("MIDI data, consumed: "));
    T_DEBUG_PRINT(consumed);
    T_DEBUG_PRINT(F(" "));
    for (auto j = 0; j < consumed; j++)
    {
        T_DEBUG_PRINT(" 0x");
        T_DEBUG_PRINT(buffer[i + j], HEX);
    }
    T_DEBUG_PRINTLN();
#endif
    
    for (size_t j = 0; j < consumed; j++)
        session->ReceivedMidi(buffer[i + j]);

    return consumed;
}

size_t decodeMidiSysEx(Deque<byte, Settings::MaxBufferSize> &buffer, size_t i, size_t cmd_len)
{
    auto oi = i; // original index
    int consumed = 1; // beginning SysEx Token is not counted (as it could remain)
    auto octet = buffer[++i];

    while (i < buffer.size() && cmd_len--) {
        consumed++;
        octet = buffer[++i];
        if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveEnd) // Complete message
            return consumed + 1;
        else if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveStart) // Start
            return consumed;
    }
    
    T_DEBUG_PRINT(F("Partial SysEx. Consumed: "));
    T_DEBUG_PRINTLN(consumed);

    // begin of the SysEx is found, not the end.
    // so transmit what we have, add a stop-token at the end,
    // remove the byes, modify the length and indicate
    // not-enough data, so we buffer gets filled with the remaining bytes.
    
    // send midi data
    for (auto j = 0; j < consumed; j++)
        session->ReceivedMidi(buffer[oi + j]);
    session->ReceivedMidi(MIDI_NAMESPACE::MidiType::SystemExclusiveStart);
    
    // Remove the bytes that were submitted
    for (auto j = 0; j < consumed; j++)
        buffer.pop_back();
    buffer.push_back(MIDI_NAMESPACE::MidiType::SystemExclusiveEnd);

    // Recalc CommandLength: Substract consumed from the length
    uint8_t rtpMidi_Flags = buffer[12];
    uint16_t commandLength = rtpMidi_Flags & RTP_MIDI_CS_MASK_SHORTLEN;
    if (rtpMidi_Flags & RTP_MIDI_CS_FLAG_B)
    {
        uint8_t octet = buffer[13];
        commandLength = (commandLength << 8) | octet;
    }
    
    commandLength -= consumed;
    commandLength += 1; // adding the manual SysEx SystemExclusiveEnd

    updateCommandLength(buffer, commandLength);
                      
    printBuffer(buffer);
    
    // indicates split SysEx
    return buffer.max_size() + 1;
}

void updateCommandLength(Deque<byte, Settings::MaxBufferSize> &buffer, uint16_t value)
{
    uint8_t rtpMidi_Flags = buffer[12];
    
    const byte mask = 0x0F;
    if (rtpMidi_Flags & RTP_MIDI_CS_FLAG_B)
    {
        buffer[12] = (buffer[12] & ~mask) | (((value >> (1 * 8)) & 0xFF) & mask);
        buffer[13] = (value >> (0 * 8)) & 0xFF;
    }
    else
        buffer[12] = (buffer[12] & ~mask) | (((value >> (0 * 8)) & 0xFF) & mask);
}

void printBuffer(const Deque<byte, Settings::MaxBufferSize> &buffer)
{
    uint8_t i = 0;
    
    conversionBuffer cb;

    Rtp_t rtp;
    rtp.vpxcc    = buffer[i++];
    rtp.mpayload = buffer[i++];
    
    cb.buffer[0] = buffer[i++];
    cb.buffer[1] = buffer[i++];
    rtp.sequenceNr = ntohs(cb.value16);
    cb.buffer[0] = buffer[i++];
    cb.buffer[1] = buffer[i++];
    cb.buffer[2] = buffer[i++];
    cb.buffer[3] = buffer[i++];
    rtp.timestamp = ntohl(cb.value32);
    cb.buffer[0] = buffer[i++];
    cb.buffer[1] = buffer[i++];
    cb.buffer[2] = buffer[i++];
    cb.buffer[3] = buffer[i++];
    rtp.ssrc = ntohl(cb.value32);
            
    V_DEBUG_PRINT(F("Buffer: "));
        
    V_DEBUG_PRINT(F("Sequence Nr: "));
    V_DEBUG_PRINT(rtp.sequenceNr);
    
    /* RTP-MIDI starts with 4 bits of flags... */
    uint8_t rtpMidi_Flags = buffer[i++];

    // ...followed by a length-field of at least 4 bits
    uint16_t commandLength = rtpMidi_Flags & RTP_MIDI_CS_MASK_SHORTLEN;

    /* see if we have small or large len-field */
    if (rtpMidi_Flags & RTP_MIDI_CS_FLAG_B)
    {
        // long header
        uint8_t octet = buffer[i++];
        commandLength = (commandLength << 8) | octet;
    }

    V_DEBUG_PRINT(F(" Buffer Size: "));
    V_DEBUG_PRINTLN(buffer.size());

    size_t j = 0;
    T_DEBUG_PRINT(" 0x");
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(" 0x");
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(" 0x");
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(" 0x");
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(" 0x");
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINT(buffer[j++], HEX);
    T_DEBUG_PRINTLN(buffer[j++], HEX);

    T_DEBUG_PRINT(" 0x");
    T_DEBUG_PRINT(buffer[j++], HEX);
    if (rtpMidi_Flags & RTP_MIDI_CS_FLAG_B)
        T_DEBUG_PRINT(buffer[j++], HEX);
    V_DEBUG_PRINT(F(" (Command length: "));
    V_DEBUG_PRINT(commandLength);
    T_DEBUG_PRINT(")");
    T_DEBUG_PRINTLN();
    T_DEBUG_PRINT("\t");

    size_t column = 0;
    for (; j < buffer.size(); j++)
    {
        T_DEBUG_PRINT(" 0x");
        T_DEBUG_PRINT(buffer[j], HEX);
        if (++column % 10 == 0)
        {
            T_DEBUG_PRINTLN();
            T_DEBUG_PRINT("\t");
        }
    }
    T_DEBUG_PRINTLN();
}

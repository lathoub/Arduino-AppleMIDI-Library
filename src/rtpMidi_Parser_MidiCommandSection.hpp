void decodeMidiSection(uint8_t rtpMidi_Flags, Deque<byte, Settings::MaxBufferSize> &buffer, uint16_t commandLength, size_t i)
{
    int cmdCount = 0;

    uint8_t runningstatus = 0;

    /* Multiple MIDI-commands might follow - the exact number can only be discovered by really decoding the commands! */
    while (commandLength)
    {

        /* for the first command we only have a delta-time if Z-Flag is set */
        if ((cmdCount) || (rtpMidi_Flags & RTP_MIDI_CS_FLAG_Z))
        {
            auto consumed = decodeTime(buffer, i);
            commandLength -= consumed;
            i += consumed;
        }

        if (commandLength > 0)
        {
            /* Decode a MIDI-command - if 0 is returned something went wrong */
            size_t consumed = decodeMidi(buffer, i, commandLength, runningstatus);

            if (consumed == 0)
            {
                E_DEBUG_PRINTLN(F("decodeMidi indicates it did not consumed bytes"));
                E_DEBUG_PRINT(F("decodeMidi commandLength is "));
                E_DEBUG_PRINTLN(commandLength);
            }

            commandLength -= consumed;
            i += consumed;

            cmdCount++;
        }
    }
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

        return consumed;
    }

    /* Here we catch the remaining system-common commands */
    switch (octet)
    {
    case MIDI_NAMESPACE::MidiType::SystemExclusiveStart:
        consumed = decodeMidiSysEx(buffer, i , cmd_len);
        break;
    case MIDI_NAMESPACE::MidiType::SystemExclusiveEnd:
        while (cmd_len--) {
            consumed++;
            octet = buffer[++i];
            if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveEnd) // End
                return consumed;
            else if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveStart) // middle
                return consumed;
        }
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
    T_DEBUG_PRINT(F("MIDI data 0x"));
    for (auto j = 0; j < consumed; j++)
    {
        T_DEBUG_PRINT(buffer[i + j], HEX);
        T_DEBUG_PRINT(" ");
    }
    T_DEBUG_PRINTLN();
#endif
    
    for (size_t j = 0; j < consumed; j++)
        session->ReceivedMidi(buffer[i + j]);

    return consumed;
}

size_t decodeMidiSysEx(Deque<byte, Settings::MaxBufferSize> &buffer, size_t i, size_t cmd_len)
{
    int consumed = 0;
    auto octet = buffer[++i];

    while (i < buffer.size() && cmd_len--) {
        consumed++;
        octet = buffer[++i];
        if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveEnd) // Complete message
            return consumed;
        else if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveStart) // Start
            return consumed;
    }
    
    // TODO: remove from the end
 //   for (auto j = 0; j < consumed; j++)
 //       buffer.pop();
    
    // TODO: replace f0 with f7 sysex begin
    // TODO: add sysex end
    // TODO: fix length
    
    buffer.push_back(0xFF);
    
    // TODO: return with notEnough
    
    return 0;
}

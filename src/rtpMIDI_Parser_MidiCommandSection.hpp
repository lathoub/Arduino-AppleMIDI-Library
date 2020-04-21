parserReturn decodeMidiSection(RtpBuffer_t &buffer)
{
    int cmdCount = 0;

    uint8_t runningstatus = 0;
    
    /* Multiple MIDI-commands might follow - the exact number can only be discovered by really decoding the commands! */
    while (midiCommandLength)
    {
        /* for the first command we only have a delta-time if Z-Flag is set */
        if ((cmdCount) || (rtpMidi_Flags & RTP_MIDI_CS_FLAG_Z))
        {
            auto consumed = decodeTime(buffer);

            midiCommandLength -= consumed;

            while (consumed--)
                buffer.pop_front();
              
            if (midiCommandLength > 0 && 0 >= buffer.size())
                return parserReturn::NotEnoughData;
        }

        if (midiCommandLength > 0)
        {
            /* Decode a MIDI-command - if 0 is returned something went wrong */
            size_t consumed = decodeMidi(buffer, runningstatus);
            if (consumed == 0)
            {
            }
            else if (consumed > buffer.size())
            {
                // sysex split in decodeMidi
                return parserReturn::NotEnoughData;
            }

            midiCommandLength -= consumed;

            while (consumed--)
                buffer.pop_front();

            if (midiCommandLength > 0 && 0 >= buffer.size())
                return parserReturn::NotEnoughData;

            cmdCount++;
        }
    }
    
    return parserReturn::Processed;
}

size_t decodeTime(RtpBuffer_t &buffer)
{
    uint8_t consumed = 0;
    uint32_t deltatime = 0;

    /* RTP-MIDI deltatime is "compressed" using only the necessary amount of octets */
    for (uint8_t j = 0; j < 4; j++)
    {
        uint8_t octet = buffer[consumed];
        deltatime = (deltatime << 7) | (octet & RTP_MIDI_DELTA_TIME_OCTET_MASK);
        consumed++;

        if ((octet & RTP_MIDI_DELTA_TIME_EXTENSION) == 0)
            break;
    }
    return consumed;
}

size_t decodeMidi(RtpBuffer_t &buffer, uint8_t &runningstatus)
{
    size_t consumed = 0;

    auto octet = buffer[0];
    bool using_rs;

    /* MIDI realtime-data -> one octet  -- unlike serial-wired MIDI realtime-commands in RTP-MIDI will
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

        session->StartReceivedMidi();
        for (size_t j = 0; j < consumed; j++)
            session->ReceivedMidi(buffer[j]);
        session->EndReceivedMidi();
        
        return consumed;
    }

    /* Here we catch the remaining system-common commands */
    switch (octet)
    {
    case MIDI_NAMESPACE::MidiType::SystemExclusiveStart:
    case MIDI_NAMESPACE::MidiType::SystemExclusiveEnd:
        consumed = decodeMidiSysEx(buffer);
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

    session->StartReceivedMidi();
    for (size_t j = 0; j < consumed; j++)
        session->ReceivedMidi(buffer[j]);
    session->EndReceivedMidi();

    return consumed;
}

size_t decodeMidiSysEx(RtpBuffer_t &buffer)
{
    size_t consumed = 1; // beginning SysEx Token is not counted (as it could remain)
    size_t i = 0;
    auto octet = buffer[++i];

    while (i < buffer.size())
    {
        consumed++;
        octet = buffer[i++];
        if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveEnd) // Complete message
            return consumed;
        else if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveStart) // Start
            return consumed;
    }
    
    // begin of the SysEx is found, not the end.
    // so transmit what we have, add a stop-token at the end,
    // remove the byes, modify the length and indicate
    // not-enough data, so we buffer gets filled with the remaining bytes.
    
    // to compensate for adding the sysex at the end.
    consumed--;
    
    // send MIDI data
    session->StartReceivedMidi();
    for (size_t j = 0; j < consumed; j++)
        session->ReceivedMidi(buffer[j]);
    session->ReceivedMidi(MIDI_NAMESPACE::MidiType::SystemExclusiveStart);
    session->EndReceivedMidi();

    // Remove the bytes that were submitted
    for (size_t j = 0; j < consumed; j++)
        buffer.pop_front();
    buffer.push_front(MIDI_NAMESPACE::MidiType::SystemExclusiveEnd);

    midiCommandLength -= consumed;
    midiCommandLength += 1; // adding the manual SysEx SystemExclusiveEnd
                          
    // indicates split SysEx
    return buffer.max_size() + 1;
}

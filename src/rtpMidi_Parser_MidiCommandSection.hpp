size_t decodeMidiSection(uint8_t rtpMidiFlags, RingBuffer<byte, Settings::MaxBufferSize> &buffer, size_t i)
{
    // ...followed by a length-field of at least 4 bits
    size_t commandLength = rtpMidiFlags & RTP_MIDI_CS_MASK_SHORTLEN;

    /* see if we have small or large len-field */
    if (rtpMidiFlags & RTP_MIDI_CS_FLAG_B)
    {
        uint8_t octet = buffer.peek(i++);
        commandLength = (commandLength << 8) | octet;
    }

    /* if we have a command-section -> dissect it */
    if (commandLength > 0)
    {
        int cmdCount = 0;

        uint8_t runningstatus = 0;

        /* Multiple MIDI-commands might follow - the exact number can only be discovered by really decoding the commands! */
        while (commandLength)
        {

            /* for the first command we only have a delta-time if Z-Flag is set */
            if ((cmdCount) || (rtpMidiFlags & RTP_MIDI_CS_FLAG_Z))
            {
                auto consumed = decodeTime(buffer, i);
                commandLength -= consumed;
                i += consumed;
            }

            if (commandLength > 0)
            {
                /* Decode a MIDI-command - if 0 is returned something went wrong */
                size_t consumed = decodeMidi(buffer, i, runningstatus);

                if (consumed == 0)
                {
                    E_DEBUG_PRINTLN(F("decodeMidi indicates it did not consumed bytes"));
                    E_DEBUG_PRINT(F("decodeMidi commandLength is "));
                    E_DEBUG_PRINTLN(commandLength);
                }

#if DEBUG >= LOG_LEVEL_TRACE
                T_DEBUG_PRINT(F("MIDI data 0x"));
                for (auto j = 0; j < consumed; j++)
                {
                    T_DEBUG_PRINT(buffer.peek(i + j), HEX);
                    T_DEBUG_PRINT(" ");
                }
                T_DEBUG_PRINTLN();
#endif

                for (size_t j = 0; j < consumed; j++)
                    session->ReceivedMidi(buffer.peek(i + j));

                commandLength -= consumed;
                i += consumed;

                cmdCount++;
            }
        }
    }

    return i;
}

size_t decodeTime(RingBuffer<byte, Settings::MaxBufferSize> &buffer, size_t i)
{
    uint8_t consumed = 0;
    uint32_t deltatime = 0;

    /* RTP-MIDI deltatime is "compressed" using only the necessary amount of octets */
    for (uint8_t j = 0; j < 4; j++)
    {
        uint8_t octet = buffer.peek(i + consumed);
        deltatime = (deltatime << 7) | (octet & RTP_MIDI_DELTA_TIME_OCTET_MASK);
        consumed++;

        if ((octet & RTP_MIDI_DELTA_TIME_EXTENSION) == 0)
            break;
    }
    return consumed;
}

size_t decodeMidi(RingBuffer<byte, Settings::MaxBufferSize> &buffer, size_t i, uint8_t &runningstatus)
{
    size_t consumed = 0;

    uint8_t octet = buffer.peek(i);
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
        uint8_t channel = (octet & 0x0f) + 1;

        switch (type)
        {
        case MIDI_NAMESPACE::MidiType::NoteOff:
        {
            uint8_t note = buffer.peek(i + consumed);
            consumed++;
            uint8_t velocity = buffer.peek(i + consumed);
            consumed++;
            //ext_consumed = decode_note_off( tvb, pinfo, tree, cmd_count, offset,  cmd_len, octet, *rsoffset, using_rs );
        }
        break;
        case MIDI_NAMESPACE::MidiType::NoteOn:
        {
            uint8_t note = buffer.peek(i + consumed);
            consumed++;
            uint8_t velocity = buffer.peek(i + consumed);
            consumed++;
            //ext_consumed = decode_note_on( tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
        }
        break;
        case MIDI_NAMESPACE::MidiType::AfterTouchPoly:
        {
            consumed += 2;
            //ext_consumed = decode_poly_pressure(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
        }
        break;
        case MIDI_NAMESPACE::MidiType::ControlChange:
            consumed += 2;
            //ext_consumed = decode_control_change(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
            break;
        case MIDI_NAMESPACE::MidiType::ProgramChange:
            consumed += 1;
            //ext_consumed = decode_program_change(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
            break;
        case MIDI_NAMESPACE::MidiType::AfterTouchChannel:
            consumed += 1;
            //ext_consumed = decode_channel_pressure(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
            break;
        case MIDI_NAMESPACE::MidiType::PitchBend:
            consumed += 2;
            //ext_consumed = decode_pitch_bend_change(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
            break;
        }

        return consumed;
    }

    /* Here we catch the remaining system-common commands */
    switch (octet)
    {
    case MIDI_NAMESPACE::MidiType::SystemExclusiveStart:
        //ext_consumed =  decode_sysex_start( tvb, pinfo, tree, cmd_count, offset, cmd_len );
        break;
    case MIDI_NAMESPACE::MidiType::TimeCodeQuarterFrame:
        consumed += 1;
        //ext_consumed =  decode_mtc_quarter_frame( tvb, pinfo, tree, cmd_count, offset, cmd_len );
        break;
    case MIDI_NAMESPACE::MidiType::SongPosition:
        consumed += 2;
        //ext_consumed =  decode_song_position_pointer( tvb, pinfo, tree, cmd_count, offset, cmd_len );
        break;
    case MIDI_NAMESPACE::MidiType::SongSelect:
        consumed += 1;
        //ext_consumed =  decode_song_select( tvb, pinfo, tree, cmd_count, offset, cmd_len );
        break;
    case MIDI_NAMESPACE::MidiType::TuneRequest:
        //ext_consumed =  decode_tune_request( tvb, pinfo, tree, cmd_count, offset, cmd_len );
        break;
    case MIDI_NAMESPACE::MidiType::SystemExclusiveEnd:
        //ext_consumed =  decode_sysex_end( tvb, pinfo, tree, cmd_count, offset, cmd_len );
        break;
    }

    return consumed;
}

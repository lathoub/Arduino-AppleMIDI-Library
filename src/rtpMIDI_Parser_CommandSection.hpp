// https://www.ietf.org/rfc/rfc4695.html#section-3

parserReturn decodeMIDICommandSection(RtpBuffer_t &buffer)
{
    debugPrintBuffer(buffer);

    // https://www.ietf.org/rfc/rfc4695.html#section-3.2
    // 
    // The first MIDI channel command in the MIDI list MUST include a status
    // octet.Running status coding, as defined in[MIDI], MAY be used for
    // all subsequent MIDI channel commands in the list.As in[MIDI],
    // System Commonand System Exclusive messages(0xF0 ... 0xF7) cancel
    // the running status state, but System Real - time messages(0xF8 ...
    // 0xFF) do not affect the running status state. All System commands in
    // the MIDI list MUST include a status octet.

    // As we note above, the first channel command in the MIDI list MUST
    // include a status octet.However, the corresponding command in the
    // original MIDI source data stream might not have a status octet(in
    // this case, the source would be coding the command using running
    // status). If the status octet of the first channel command in the
    // MIDI list does not appear in the source data stream, the P(phantom)
    // header bit MUST be set to 1.  In all other cases, the P bit MUST be
    // set to 0.
    //
    // Note that the P bit describes the MIDI source data stream, not the
    // MIDI list encoding; regardless of the state of the P bit, the MIDI
    // list MUST include the status octet.
    // 
    // As receivers MUST be able to decode running status, sender
    // implementors should feel free to use running status to improve
    // bandwidth efficiency. However, senders SHOULD NOT introduce timing
    // jitter into an existing MIDI command stream through an inappropriate
    // use or removal of running status coding. This warning primarily
    // applies to senders whose RTP MIDI streams may be transcoded onto a
    // MIDI 1.0 DIN cable[MIDI] by the receiver : both the timestamps and
    // the command coding (running status or not) must comply with the
    // physical restrictions of implicit time coding over a slow serial
    // line.

    // (lathoub: RTP_MIDI_CS_FLAG_P((phantom) not implemented
    
    /* Multiple MIDI-commands might follow - the exact number can only be discovered by really decoding the commands! */
    while (midiCommandLength)
    {
        /* for the first command we only have a delta-time if Z-Flag is set */
        if ((cmdCount) || (rtpMidi_Flags & RTP_MIDI_CS_FLAG_Z))
        {
            size_t consumed = 0;
            auto retVal = decodeTime(buffer, consumed);
            if (retVal != parserReturn::Processed) return retVal;

            midiCommandLength -= consumed;
            while (consumed--)
                buffer.pop_front();
        }

        if (midiCommandLength > 0)
        {
            cmdCount++;

            size_t consumed = 0;
            auto retVal = decodeMidi(buffer, runningstatus, consumed);
            if (retVal == parserReturn::NotEnoughData) {
                cmdCount = 0; // avoid first command again
                return retVal;
            }

            midiCommandLength -= consumed;
            while (consumed--)
                buffer.pop_front();
        }
    }
    
    return parserReturn::Processed;
}

parserReturn decodeTime(RtpBuffer_t &buffer, size_t &consumed)
{
    debugPrintBuffer(buffer);

    uint32_t deltatime = 0;

    /* RTP-MIDI deltatime is "compressed" using only the necessary amount of octets */
    for (uint8_t j = 0; j < 4; j++)
    {
        if (buffer.size() < 1)
            return parserReturn::NotEnoughData;

        uint8_t octet = buffer[consumed];
        deltatime = (deltatime << 7) | (octet & RTP_MIDI_DELTA_TIME_OCTET_MASK);
        consumed++;

        if ((octet & RTP_MIDI_DELTA_TIME_EXTENSION) == 0)
            break;
    }

    return parserReturn::Processed;
}

parserReturn decodeMidi(RtpBuffer_t &buffer, uint8_t &runningstatus, size_t &consumed)
{
    debugPrintBuffer(buffer);

    if (buffer.size() < 1)
        return parserReturn::NotEnoughData;

    auto octet = buffer.front();

    /* MIDI realtime-data -> one octet  -- unlike serial-wired MIDI realtime-commands in RTP-MIDI will
     * not be intermingled with other MIDI-commands, so we handle this case right here and return */
    if (octet >= 0xf8)
    {
        consumed = 1;

        session->StartReceivedMidi();
        session->ReceivedMidi(octet);
        session->EndReceivedMidi();
        
        return parserReturn::Processed;
    }

    /* see if this first octet is a status message */
    if ((octet & RTP_MIDI_COMMAND_STATUS_FLAG) == 0)
    {
        /* if we have no running status yet -> error */
        if (((runningstatus)&RTP_MIDI_COMMAND_STATUS_FLAG) == 0)
        {
            return parserReturn::Processed;
        }
        /* our first octet is "virtual" coming from a preceding MIDI-command,
         * so actually we have not really consumed anything yet */
        octet = runningstatus;
    }
    else
    {
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
        switch (octet & 0xf0)
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

        if (buffer.size() < consumed) {
            return parserReturn::NotEnoughData;
        }

        session->StartReceivedMidi();
        for (size_t j = 0; j < consumed; j++)
            session->ReceivedMidi(buffer[j]);
        session->EndReceivedMidi();
        
        return parserReturn::Processed;
    }

    /* Here we catch the remaining system-common commands */
    switch (octet)
    {
    case MIDI_NAMESPACE::MidiType::SystemExclusiveStart:
    case MIDI_NAMESPACE::MidiType::SystemExclusiveEnd:
        decodeMidiSysEx(buffer, consumed);
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

    if (buffer.size() < consumed)
        return parserReturn::NotEnoughData;

    session->StartReceivedMidi();
    for (size_t j = 0; j < consumed; j++)
        session->ReceivedMidi(buffer[j]);
    session->EndReceivedMidi();

    return parserReturn::Processed;
}

parserReturn decodeMidiSysEx(RtpBuffer_t &buffer, size_t &consumed)
{
    debugPrintBuffer(buffer);

//    consumed = 1; // beginning SysEx Token is not counted (as it could remain)
    size_t i = 1; // 0 = start of SysEx, so we can start with 1
    while (i < buffer.size())
    {
        consumed++;
        auto octet = buffer[i++];

        Serial.print("0x");
        Serial.print(octet < 16 ? "0" : "");
        Serial.print(octet, HEX);
        Serial.print(" ");

        if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveEnd) // Complete message
        {
            return parserReturn::Processed;
        }
        else if (octet == MIDI_NAMESPACE::MidiType::SystemExclusiveStart) // Start
        {
            return parserReturn::Processed;
        }
    }
            
    // begin of the SysEx is found, not the end.
    // so transmit what we have, add a stop-token at the end,
    // remove the bytes, modify the length and indicate
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
    // Start a new SysEx train
    buffer.push_front(MIDI_NAMESPACE::MidiType::SystemExclusiveEnd);

    midiCommandLength -= consumed;
    midiCommandLength += 1; // for adding the manual SysEx SystemExclusiveEnd in front

    // indicates split SysEx
    consumed = buffer.max_size() + 1;

    return parserReturn::Processed;
}

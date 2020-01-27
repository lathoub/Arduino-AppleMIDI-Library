// The recovery journal is the default resiliency tool for unreliable
// transport. In this section, we normatively define the roles that
// senders and receivers play in the recovery journal system.
//
// This section introduces the structure of the recovery journal and
// defines the bitfields of recovery journal headers. Appendices A and
// B complete the bitfield definition of the recovery journal.
//
// The recovery journal has a three-level structure:
//
// o Top-level header.
//
// o Channel and system journal headers. These headers encode recovery
//   information for a single voice channel (channel journal) or for
//   all system commands (system journal).
//
// o Chapters. Chapters describe recovery information for a single
//   MIDI command type.
//
parserReturn decodeJournalSection(RingBuffer<byte, Settings::MaxBufferSize> &buffer, size_t &i, size_t &minimumLen)
{
    conversionBuffer cb;

    V_DEBUG_PRINTLN(F("Journal section"));

    minimumLen += 1;
    if (buffer.getLength() < minimumLen)
        return parserReturn::NotEnoughData;

    /* lets get the main flags from the recovery journal header */
    uint8_t flags = buffer.peek(i++);

    V_DEBUG_PRINT(F("flags: 0x"));
    V_DEBUG_PRINTLN(flags, HEX);

    // sequenceNr
    minimumLen += 2;
    if (buffer.getLength() < minimumLen)
        return parserReturn::NotEnoughData;

    if ((flags & RTP_MIDI_JS_FLAG_Y) == 0 && (flags & RTP_MIDI_JS_FLAG_A) == 0)
    {
        V_DEBUG_PRINTLN(F("empty journal section"));
        return parserReturn::Processed;
    }
    
    // The 16-bit Checkpoint Packet Seqnum header field codes the sequence
    // number of the checkpoint packet for this journal, in network byte
    // order (big-endian). The choice of the checkpoint packet sets the
    // depth of the checkpoint history for the journal (defined in Appendix A.1).
    //
    // Receivers may use the Checkpoint Packet Seqnum field of the packet
    // that ends a loss event to verify that the journal checkpoint history
    // covers the entire loss event. The checkpoint history covers the loss
    // event if the Checkpoint Packet Seqnum field is less than or equal to
    // one plus the highest RTP sequence number previously received on the
    // stream (modulo 2^16).
    cb.buffer[0] = buffer.peek(i++);
    cb.buffer[1] = buffer.peek(i++);
    uint16_t checkPoint = ntohs(cb.value16);

    V_DEBUG_PRINT(F("checkPoint: "));
    V_DEBUG_PRINTLN(checkPoint);
    
    // By default, the payload format does not use enhanced Chapter C
    // encoding. In this default case, the H bit MUST be set to 0 for all
    // packets in the stream.
    if (flags & RTP_MIDI_JS_FLAG_H)
    {
        // The H bit indicates if MIDI channels in the stream have been
        // configured to use the enhanced Chapter C encoding

        V_DEBUG_PRINTLN(F("enhanced Chapter C encoding NOT SUPPORTED"));
    }

    // The S (single-packet loss) bit appears in most recovery journal
    // structures, including the recovery journal header. The S bit helps
    // receivers efficiently parse the recovery journal in the common case
    // of the loss of a single packet.
    if (flags & RTP_MIDI_JS_FLAG_S)
    {
        V_DEBUG_PRINTLN(F("special encoding"));
        // special encoding
    }

    // If the Y header bit is set to 1, the system journal appears in the
    // recovery journal, directly following the recovery journal header.
    if (flags & RTP_MIDI_JS_FLAG_Y)
    {
        V_DEBUG_PRINTLN(F("System journal"));
        
        minimumLen += 2;
        if (buffer.getLength() < minimumLen)
            return parserReturn::NotEnoughData;

        cb.buffer[0] = buffer.peek(i++);
        cb.buffer[1] = buffer.peek(i++);
        uint16_t systemflags = ntohs(cb.value16);
        uint16_t sysjourlen = systemflags & RTP_MIDI_SJ_MASK_LENGTH;
        
        uint16_t remainingBytes = sysjourlen - 2;
        
        minimumLen += remainingBytes;
        if (buffer.getLength() < minimumLen)
            return parserReturn::NotEnoughData;

        i+=remainingBytes;
    }

    // If the A header bit is set to 1, the recovery journal ends with a
    // list of (TOTCHAN + 1) channel journals (the 4-bit TOTCHAN header
    // field is interpreted as an unsigned integer).
    if (flags & RTP_MIDI_JS_FLAG_A)
    {
        V_DEBUG_PRINTLN(F("Channel journal"));

        /* At the same place we find the total channels encoded in the channel journal */
        uint8_t totalChannels = (flags & RTP_MIDI_JS_MASK_TOTALCHANNELS) + 1;

        V_DEBUG_PRINT(F("totalChannels: "));
        V_DEBUG_PRINTLN(totalChannels);

        // iterate through all the channels specified in header
        while (totalChannels-- > 0)
        {
            minimumLen += 3;
            if (buffer.getLength() < minimumLen)
                return parserReturn::NotEnoughData;

            cb.buffer[0] = 0x00;
            cb.buffer[1] = buffer.peek(i++);
            cb.buffer[2] = buffer.peek(i++);
            cb.buffer[3] = buffer.peek(i++);
            uint32_t chanflags = ntohl(cb.value32);
            
            uint16_t chanjourlen = (chanflags & RTP_MIDI_CJ_MASK_LENGTH) >> 8;
            
            V_DEBUG_PRINT(F("Channel Flags: "));
            V_DEBUG_PRINTLN(chanflags);
            V_DEBUG_PRINT(F("Journal Length: "));
            V_DEBUG_PRINTLN(chanjourlen);

            auto j = i;
            i += (chanjourlen - 3); // -3 as the chanflags are included already
            
            /* Do we have a program change chapter? */
            if (chanflags & RTP_MIDI_CJ_FLAG_P)
            {
                V_DEBUG_PRINTLN(F("program change chapter"));

                minimumLen += 3;
                if (buffer.getLength() < minimumLen)
                    return parserReturn::NotEnoughData;
            }

            /* Do we have a control chapter? */
            if (chanflags & RTP_MIDI_CJ_FLAG_C)
            {
                V_DEBUG_PRINTLN(F("control chapter"));
            }

            /* Do we have a parameter changes? */
            if (chanflags & RTP_MIDI_CJ_FLAG_M)
            {
                V_DEBUG_PRINTLN(F("parameter changes"));
            }

            /* Do we have a pitch-wheel chapter? */
            if (chanflags & RTP_MIDI_CJ_FLAG_W)
            {
                V_DEBUG_PRINTLN(F("pitch-wheel chapter"));

                minimumLen += 2;
                if (buffer.getLength() < minimumLen)
                    return parserReturn::NotEnoughData;
            }

            /* Do we have a note on/off chapter? */
            if (chanflags & RTP_MIDI_CJ_FLAG_N)
            {
                V_DEBUG_PRINTLN(F("note on/off chapter"));

                minimumLen += 2;
                if (buffer.getLength() < minimumLen)
                    return parserReturn::NotEnoughData;

                cb.buffer[0] = buffer.peek(j++);
                cb.buffer[1] = buffer.peek(j++);
                const uint16_t header = ntohs(cb.value16);

                uint8_t logListCount = (header & RTP_MIDI_CJ_CHAPTER_N_MASK_LENGTH) >> 8;
                const uint8_t low = (header & RTP_MIDI_CJ_CHAPTER_N_MASK_LOW) >> 4;
                const uint8_t high = (header & RTP_MIDI_CJ_CHAPTER_N_MASK_HIGH);

                // how many offbits octets do we have?
                uint8_t offbitCount = 0;
                if (low <= high)
                    offbitCount = high - low + 1;
                else if ((low == 15) && (high == 0))
                    offbitCount = 0;
                else if ((low == 15) && (high == 1))
                    offbitCount = 0;
                else
                    return parserReturn::UnexpectedData;

                // special case -> no offbit octets, but 128 note-logs
                if ((logListCount == 127) && (low == 15) && (high == 0))
                {
                    logListCount = 128;
                    // offbitCount should be 0 (empty)
                }

                V_DEBUG_PRINT(F("offbitCount: "));
                V_DEBUG_PRINTLN(offbitCount);
                V_DEBUG_PRINT(F("logListCount: "));
                V_DEBUG_PRINTLN(logListCount);

                minimumLen += ((logListCount * 2) + offbitCount);
                if (buffer.getLength() < minimumLen)
                    return parserReturn::NotEnoughData;

                // // Log List
                //for (auto j = 0; j < logListCount; j++ ) {
                //     buffer.peek(j++);
                //     buffer.peek(j++);
                //}

                // // Offbit Octets
                //for (auto j = 0; j < offbitCount; j++ ) {
                //     buffer.peek(j++);
                //}
            }

            /* Do we have a note command extras chapter? */
            if (chanflags & RTP_MIDI_CJ_FLAG_E)
            {
                V_DEBUG_PRINTLN(F("note command extras chapter"));

                minimumLen += 1;
                if (buffer.getLength() < minimumLen)
                    return parserReturn::NotEnoughData;

                /* first we need to get the flags & length of this chapter */
                uint8_t header = buffer.peek(i++);
                uint8_t log_count = header & RTP_MIDI_CJ_CHAPTER_E_MASK_LENGTH;

                log_count++;

                minimumLen += (log_count * 2);
                if (buffer.getLength() < minimumLen)
                    return parserReturn::NotEnoughData;

                //for (auto k = 0; k < log_count; k++ ) {
                //    uint8_t note = buffer.peek(j++) & 0x7f;
                //    uint8_t octet = buffer.peek(j++);
                //    uint8_t count_vel = octet & 0x7f;
                //}
            }

            /* Do we have channel aftertouch chapter? */
            if (chanflags & RTP_MIDI_CJ_FLAG_T)
            {
                V_DEBUG_PRINTLN(F("channel aftertouch chapter"));

                minimumLen += 1;
                if (buffer.getLength() < minimumLen)
                    return parserReturn::NotEnoughData;
            }

            /* Do we have a poly aftertouch chapter? */
            if (chanflags & RTP_MIDI_CJ_FLAG_A)
            {
                V_DEBUG_PRINTLN(F("poly aftertouch chapter"));

                minimumLen += 2;
                if (buffer.getLength() < minimumLen)
                    return parserReturn::NotEnoughData;

                /* first we need to get the flags & length of this chapter */
                uint8_t flags = buffer.peek(j++);
                uint8_t log_count = flags & RTP_MIDI_CJ_CHAPTER_A_MASK_LENGTH;

                /* count is encoded n+1 */
                log_count++;

                //for (auto k = 0; k < log_count; k++ ) {
                //    uint8_t note = buffer.peek(j++);
                //    uint8_t pressure = buffer.peek(j++);
                //}
            }
        }
    }

    return parserReturn::Processed;
}

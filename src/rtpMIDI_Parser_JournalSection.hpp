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
parserReturn decodeJournalSection(RtpBuffer_t &buffer)
{
    size_t minimumLen = 0;

    conversionBuffer cb;

    if (false == _journalSectionComplete)
    {
        size_t i = 0;

        // Minimum size for the Journal section is 3
        minimumLen += 3;
        if (buffer.size() < minimumLen)
            return parserReturn::NotEnoughData;

        /* lets get the main flags from the recovery journal header */
        uint8_t flags = buffer[i++];

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
        cb.buffer[0] = buffer[i++];
        cb.buffer[1] = buffer[i++];
        uint16_t checkPoint = ntohs(cb.value16);
        
        // (RFC 4695, 5 Recovery Journal Format)
        // If A and Y are both zero, the recovery journal only contains its 3-
        // octet header and is considered to be an "empty" journal.
        if ((flags & RTP_MIDI_JS_FLAG_Y) == 0 && (flags & RTP_MIDI_JS_FLAG_A) == 0)
        {
            // Big fixed by @hugbug
            while (minimumLen-- > 0)
                buffer.pop_front();

            return parserReturn::Processed;
        }
        
        // By default, the payload format does not use enhanced Chapter C
        // encoding. In this default case, the H bit MUST be set to 0 for all
        // packets in the stream.
        if (flags & RTP_MIDI_JS_FLAG_H)
        {
            // The H bit indicates if MIDI channels in the stream have been
            // configured to use the enhanced Chapter C encoding
        }

        // The S (single-packet loss) bit appears in most recovery journal
        // structures, including the recovery journal header. The S bit helps
        // receivers efficiently parse the recovery journal in the common case
        // of the loss of a single packet.
        if (flags & RTP_MIDI_JS_FLAG_S)
        {
            // special encoding
        }

        // If the Y header bit is set to 1, the system journal appears in the
        // recovery journal, directly following the recovery journal header.
        if (flags & RTP_MIDI_JS_FLAG_Y)
        {
            minimumLen += 2;
            if (buffer.size() < minimumLen)
                return parserReturn::NotEnoughData;

            cb.buffer[0] = buffer[i++];
            cb.buffer[1] = buffer[i++];
            uint16_t systemflags = ntohs(cb.value16);
            uint16_t sysjourlen = systemflags & RTP_MIDI_SJ_MASK_LENGTH;
            
            uint16_t remainingBytes = sysjourlen - 2;
            
            minimumLen += remainingBytes;
            if (buffer.size() < minimumLen)
                return parserReturn::NotEnoughData;

            i += remainingBytes;
        }

        // If the A header bit is set to 1, the recovery journal ends with a
        // list of (TOTCHAN + 1) channel journals (the 4-bit TOTCHAN header
        // field is interpreted as an unsigned integer).
        if (flags & RTP_MIDI_JS_FLAG_A)
        {
            /* At the same place we find the total channels encoded in the channel journal */
            _journalTotalChannels = (flags & RTP_MIDI_JS_MASK_TOTALCHANNELS) + 1;

            // do channel reading below, so we can already purge bytes here
        }
        
        while (i-- > 0) // is that the same as while (i--) ??
            buffer.pop_front();
                
        _journalSectionComplete = true;
    }

    // iterate through all the channels specified in header
    while (_journalTotalChannels > 0)
    {
        if (buffer.size() < 3)
            return parserReturn::NotEnoughData;

        cb.buffer[0] = 0x00;
        cb.buffer[1] = buffer[0];
        cb.buffer[2] = buffer[1];
        cb.buffer[3] = buffer[2];
        uint32_t chanflags = ntohl(cb.value32);
        
        uint16_t chanjourlen = (chanflags & RTP_MIDI_CJ_MASK_LENGTH) >> 8;
        uint16_t channelNr   = (chanflags & RTP_MIDI_CJ_MASK_CHANNEL) >> RTP_MIDI_CJ_CHANNEL_SHIFT;

        // We have the most important bit of information - the length of the channel information
        // no more need to further parse.

        if (buffer.size() < chanjourlen)
        {
            return parserReturn::NotEnoughData;
        }
        
        _journalTotalChannels--;
        
        // advance the pointer
        while (chanjourlen--)
            buffer.pop_front();
        }
    
    return parserReturn::Processed;
}

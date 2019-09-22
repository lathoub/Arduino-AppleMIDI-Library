#pragma once

BEGIN_APPLEMIDI_NAMESPACE

template<class UdpClass>
void AppleMidiTransport<UdpClass>::run()
{
    if (!enabled)
        return;

    byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];

    auto packetSize = controlPort.parsePacket();
    if (packetSize > 0) {
        while (packetSize > 0)
        {
            auto bytesRead = controlPort.read(packetBuffer, sizeof(packetBuffer));
            controlBuffer.write(packetBuffer, bytesRead);
            packetSize -= bytesRead;
        }
        while (true) {
            auto retVal = controlParsers[0](controlBuffer, this, amPortType::Control);
            if (PARSER_NOT_ENOUGH_DATA == retVal)
                break;
            else if (PARSER_UNEXPECTED_DATA == retVal)
                controlBuffer.pop(1);
        }
    }

    // TODO: what with larger SysEx messages???

    packetSize = dataPort.parsePacket();
    if (packetSize > 0) {
        while (packetSize > 0)
        {
            auto bytesRead = dataPort.read(packetBuffer, sizeof(packetBuffer));
            dataBuffer.write(packetBuffer, bytesRead);
            packetSize -= bytesRead;
        }
        while (true) {
            auto retVal = dataParsers[0](dataBuffer, this, amPortType::Data);
            if (PARSER_NOT_ENOUGH_DATA == retVal)
                break; // we had the correct parser, but not enough data yet. 
            else if (PARSER_UNEXPECTED_DATA == retVal)
            {
                // data did not match the parser, they with another parser
                retVal = dataParsers[1](dataBuffer, this, amPortType::Data);
                if (PARSER_NOT_ENOUGH_DATA == retVal)
                    break; // we had the correct parser, but not enough data yet. 
                else if (PARSER_UNEXPECTED_DATA == retVal)
                    dataBuffer.pop(1); // non of the parsers worked, remove the leading byte and try again
            }
        }
    }

#ifdef APPLEMIDI_INITIATOR
		ManagePendingInvites();
		ManageTiming();
#endif
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedInvitation(AppleMIDI_Invitation& invitation, const amPortType& portType)
{
    //Serial.println("receivedInvitation");

    if (portType == amPortType::Control)
        ReceivedControlInvitation(invitation);
    else
        ReceivedDataInvitation(invitation);
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedControlInvitation(AppleMIDI_Invitation& invitation)
{
    //Serial.println("ReceivedControlInvitation");

    //Serial.print("initiator: 0x");
    //Serial.print(invitation.initiatorToken, HEX);
    //Serial.print(", senderSSRC: 0x");
    //Serial.print(invitation.ssrc, HEX);
    //Serial.print(", sessionName: ");
    //Serial.println(invitation.sessionName);

    // Do we know this ssrc already?
    // In case APPLEMIDI_INITIATOR reconnects (after a crash of some sort)

    auto slotIndex = getParticipant(invitation.ssrc);
    if (APPLEMIDI_PARTICIPANT_SSRC_NOTFOUND == slotIndex)
    {
        // No, not existing; must be a new APPLEMIDI_INITIATOR
        // Find a free slot to remember this participant in
        slotIndex = getParticipant(APPLEMIDI_PARTICIPANT_SLOT_FREE);
        if (APPLEMIDI_PARTICIPANT_SSRC_NOTFOUND == slotIndex)
        {
            //Serial.println("No free slots");
            // no free slots, we cant accept invite
            writeInvitation(controlPort, invitation, amInvitationRejected, ssrc);
            return;
        }

        // remember this participant
        participants[slotIndex] = invitation.ssrc;
    }
    else
    {
        // Participant already exists, do nothing
    }

    writeInvitation(controlPort, invitation, amInvitationAccepted, ssrc);
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedDataInvitation(AppleMIDI_Invitation& invitation)
{
    //Serial.println("ReceivedControlInvitation");

    //Serial.print("initiator: 0x");
    //Serial.print(invitation.initiatorToken, HEX);
    //Serial.print(", senderSSRC: 0x");
    //Serial.print(invitation.ssrc, HEX);
    //Serial.print(", sessionName: ");
    //Serial.println(invitation.sessionName);

    auto participant = getParticipant(invitation.ssrc);
    if (APPLEMIDI_PARTICIPANT_SSRC_NOTFOUND == participant)
    {
        writeInvitation(dataPort, invitation, amInvitationRejected, ssrc);
        return;
    }

    writeInvitation(dataPort, invitation, amInvitationAccepted, ssrc);

    if (NULL != _connectedCallback)
        _connectedCallback(invitation.ssrc, invitation.sessionName);
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedSyncronization(AppleMIDI_Syncronization& syncronization)
{
    //Serial.println("receivedSyncronization");

    auto now = rtpMidiClock.Now();

    switch (syncronization.count) {
    case SYNC_CK0: /* From session APPLEMIDI_INITIATOR */
        syncronization.count = SYNC_CK1;
        syncronization.timestamps[syncronization.count] = now;			
        break;
    case SYNC_CK1: /* From session responder */
        /* compute media delay */
        //auto diff = (now - syncronization.timestamps[0]) / 2;
        /* approximate time difference between peer and self */
        //diff = synchronization.timestamps[2] + diff - now;
        // Send CK2
        syncronization.count = SYNC_CK2;
        syncronization.timestamps[syncronization.count] = now;
        /* getting this message means that the responder is still alive! */
        /* remember the time, if it takes to long to respond, we can assume the responder is dead */
        /* not implemented at this stage*/
        //Sessions[index].syncronization.lastTime = now;
        //Sessions[index].syncronization.count++;
        break;
    case SYNC_CK2: /* From session APPLEMIDI_INITIATOR */
        /* compute average delay */
        //auto diff = (syncronization.timestamps[2] - syncsyncronizationhronization.timestamps[0]) / 2;
        /* approximate time difference between peer and self */
        //diff = syncronization.timestamps[2] + diff - now;
        syncronization.count = SYNC_CK0;
        syncronization.timestamps[syncronization.count] = now;
        break;
    }

    // only on the data port
    // Invitation Accepted
    if (dataPort.beginPacket(dataPort.remoteIP(), dataPort.remotePort()))
    {
        dataPort.write((uint8_t*)amSignature, sizeof(amSignature));
        dataPort.write((uint8_t*)amSyncronization, sizeof(amSyncronization));
        syncronization.ssrc = htonl(ssrc);
        syncronization.timestamps[0] = htonll(syncronization.timestamps[0]);
        syncronization.timestamps[1] = htonll(syncronization.timestamps[1]);
        syncronization.timestamps[2] = htonll(syncronization.timestamps[2]);
        dataPort.write(reinterpret_cast<uint8_t*>(&syncronization), sizeof(syncronization));
        dataPort.endPacket();
        dataPort.flush();
    }
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedEndSession(AppleMIDI_EndSession& endSession)
{
    //Serial.println("receivedEndSession");

    //Serial.print("initiator: 0x");
    //Serial.print(endSession.initiatorToken, HEX);
    //Serial.print(", senderSSRC: 0x");
    //Serial.println(endSession.ssrc, HEX);

    auto slotIndex = getParticipant(endSession.ssrc);
    if (slotIndex >= 0)
        participants[slotIndex] = APPLEMIDI_PARTICIPANT_SLOT_FREE;

    if (NULL != _disconnectedCallback)
        _disconnectedCallback(endSession.ssrc);
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedMidi(Rtp& rtp, RtpMIDI& rtpMidi, RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t cmdLen)
{
    /* if we have a command-section -> dissect it */
    if (cmdLen > 0) {
        if (rtpMidi.flags & RTP_MIDI_CS_FLAG_Z) {
            //int consumed = decodetime(appleMidi, buffer, offset, cmd_len);
        }
        while (cmdLen > 0) {
            inMidiBuffer.write(buffer.read());
            cmdLen--;
        }
    }
}

template<class UdpClass>
int8_t AppleMidiTransport<UdpClass>::getParticipant(const ssrc_t ssrc) const
{
    for (auto i = 0; i < APPLEMIDI_MAX_PARTICIPANTS; i++)
        if (ssrc == participants[i])
            return i;
    return APPLEMIDI_PARTICIPANT_SSRC_NOTFOUND;
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::writeInvitation(UdpClass& port, AppleMIDI_Invitation& invitation, uint8_t* command, ssrc_t ssrc)
{
    if (port.beginPacket(port.remoteIP(), port.remotePort())) {
        port.write((uint8_t*)amSignature,          sizeof(amSignature));
        port.write((uint8_t*)amInvitationRejected, sizeof(amInvitationRejected));
        port.write((uint8_t*)amProtocolVersion,    sizeof(amProtocolVersion));
        invitation.initiatorToken = htonl(invitation.initiatorToken);
        invitation.ssrc           = htonl(ssrc);
        port.write(reinterpret_cast<uint8_t*>(&invitation), invitation.getLength());
        port.endPacket();
        port.flush();
    }
}

#ifdef APPLEMIDI_INITIATOR

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ManagePendingInvites()
{
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ManageTiming()
{
}

#endif

END_APPLEMIDI_NAMESPACE
#pragma once

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

static byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];

template<class UdpClass>
void AppleMidiTransport<UdpClass>::readControlPackets()
{
    auto packetSize = controlPort.parsePacket();
    while (packetSize > 0)
    {
        auto bytesToRead = min(packetSize, sizeof(packetBuffer));
        auto bytesRead = controlPort.read(packetBuffer, bytesToRead);
        packetSize -= bytesRead;

        for (auto i = 0; i < bytesRead; i++)
            controlBuffer.write(packetBuffer[i]); // append

        uint8_t retVal = PARSER_UNEXPECTED_DATA;
        while ((PARSER_UNEXPECTED_DATA == retVal) && (controlBuffer.getLength() > 0)) {
            retVal = controlAppleMidiParser(controlBuffer, this, amPortType::Control);
            if (PARSER_NOT_ENOUGH_DATA == retVal)
                break;
            if (PARSER_UNEXPECTED_DATA == retVal) 
                controlBuffer.pop(1);
        }

        if ((PARSER_NOT_ENOUGH_DATA == retVal) && controlBuffer.isFull())
        {
            Serial.println("o-ow, what now??");
        }
    }
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::readDataPackets()
{
    auto packetSize = dataPort.parsePacket();
    while (packetSize > 0)
    {
        auto bytesToRead = min(packetSize, sizeof(packetBuffer));
        auto bytesRead = dataPort.read(packetBuffer, bytesToRead);
        packetSize -= bytesRead;

        for (auto i = 0; i < bytesRead; i++)
            dataBuffer.write(packetBuffer[i]); // append

        uint8_t retVal = PARSER_UNEXPECTED_DATA;
        while ((PARSER_UNEXPECTED_DATA == retVal) && (dataBuffer.getLength() > 0)) {
            retVal = dataRtpMidiParsers(dataBuffer, this);
            if (PARSER_NOT_ENOUGH_DATA == retVal) break;
            retVal = dataAppleMidiParsers(dataBuffer, this, amPortType::Data);
            if (PARSER_NOT_ENOUGH_DATA == retVal) break;

            if (PARSER_UNEXPECTED_DATA == retVal) 
                dataBuffer.pop(1);
        }

        if ((PARSER_NOT_ENOUGH_DATA == retVal) && dataBuffer.isFull())
        {
            Serial.println("o-ow, what now?? SysEx??");
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
    // Serial.println("receivedInvitation");

    if (portType == amPortType::Control)
        ReceivedControlInvitation(invitation);
    else
        ReceivedDataInvitation(invitation);
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedControlInvitation(AppleMIDI_Invitation& invitation)
{
    // Serial.println("ReceivedControlInvitation");

    // Serial.print("initiator: 0x");
    // Serial.print(invitation.initiatorToken, HEX);
    // Serial.print(", senderSSRC: 0x");
    // Serial.print(invitation.ssrc, HEX);
    // Serial.print(", sessionName: ");
    // Serial.println(invitation.sessionName);

    strncpy(invitation.sessionName, localName, APPLEMIDI_SESSION_NAME_MAX_LEN);
    invitation.sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN] = '\0';

    auto slotIndex = getParticipantIndex(participants, invitation.ssrc);
    if (APPLEMIDI_PARTICIPANT_SSRC_NOTFOUND == slotIndex)
    {
        slotIndex = getParticipantIndex(participants, APPLEMIDI_PARTICIPANT_SLOT_FREE);
        if (APPLEMIDI_PARTICIPANT_SSRC_NOTFOUND == slotIndex)
        {
            writeInvitation(controlPort, invitation, amInvitationRejected, ssrc);
            return;
        }
        participants[slotIndex] = invitation.ssrc;
    }

    writeInvitation(controlPort, invitation, amInvitationAccepted, ssrc);
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedDataInvitation(AppleMIDI_Invitation& invitation)
{
    // Serial.println("ReceivedControlInvitation");

    // Serial.print("initiator: 0x");
    // Serial.print(invitation.initiatorToken, HEX);
    // Serial.print(", senderSSRC: 0x");
    // Serial.print(invitation.ssrc, HEX);
    // Serial.print(", sessionName: ");
    // Serial.println(invitation.sessionName);

    strncpy(invitation.sessionName, localName, APPLEMIDI_SESSION_NAME_MAX_LEN);
    invitation.sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN] = '\0';

    auto participant = getParticipantIndex(participants, invitation.ssrc);
    if (APPLEMIDI_PARTICIPANT_SSRC_NOTFOUND == participant)
    {
        writeInvitation(dataPort, invitation, amInvitationRejected, ssrc);
        return;
    }

    writeInvitation(dataPort, invitation, amInvitationAccepted, ssrc);

    // callback in IDE
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

    auto slotIndex = getParticipantIndex(participants, endSession.ssrc);
    if (slotIndex >= 0)
        participants[slotIndex] = APPLEMIDI_PARTICIPANT_SLOT_FREE;

    if (NULL != _disconnectedCallback)
        _disconnectedCallback(endSession.ssrc);
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedMidi(byte data)
{
    inMidiBuffer.write(data);
}

template<class UdpClass>
int8_t AppleMidiTransport<UdpClass>::getParticipantIndex(const uint32_t participants[], const ssrc_t ssrc)
{
    for (auto i = 0; i < APPLEMIDI_MAX_PARTICIPANTS; i++)
        if (ssrc == participants[i])
            return i;
    return APPLEMIDI_PARTICIPANT_SSRC_NOTFOUND;
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::writeInvitation(UdpClass& port, AppleMIDI_Invitation& invitation, const byte* command, ssrc_t ssrc)
{
    // Serial.println("writeInvitation");

    if (port.beginPacket(port.remoteIP(), port.remotePort())) {
        port.write((uint8_t*)amSignature,       sizeof(amSignature));
        port.write((uint8_t*)command,           sizeof(amInvitationRejected));
        port.write((uint8_t*)amProtocolVersion, sizeof(amProtocolVersion));
        invitation.initiatorToken = htonl(invitation.initiatorToken);
        invitation.ssrc           = htonl(ssrc);
        port.write(reinterpret_cast<uint8_t*>(&invitation), invitation.getLength());
        port.endPacket();
        port.flush();
    }
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::writeRtpMidiBuffer(UdpClass& port, RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, uint16_t sequenceNr, ssrc_t ssrc)
{
    if (!port.beginPacket(port.remoteIP(), port.remotePort()))
        return;

    Rtp rtp;
    rtp.vpxcc      = 0b10000000; // TODO: fun with flags
    rtp.mpayload   = PAYLOADTYPE_RTPMIDI; // TODO: set or unset marker
    rtp.ssrc       = htonl(ssrc);
    // https://developer.apple.com/library/ios/documentation/CoreMidi/Reference/MIDIServices_Reference/#//apple_ref/doc/uid/TP40010316-CHMIDIServiceshFunctions-SW30
    // The time at which the events occurred, if receiving MIDI, or, if sending MIDI,
    // the time at which the events are to be played. Zero means "now." The time stamp
    // applies to the first MIDI byte in the packet.
    rtp.timestamp  = htonl(0UL);
    rtp.sequenceNr = htons(sequenceNr);
    port.write((uint8_t*)&rtp, sizeof(rtp));

    // only now the length is known
    uint16_t bufferLen = buffer.getLength();

    RtpMIDI rtpMidi;

    if (bufferLen <= 0x0F)
    {   // Short header
        rtpMidi.flags = (uint8_t)bufferLen;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_B;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_J;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_Z;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_P;
        port.write(rtpMidi.flags);
    }
    else
    {	// Long header
        rtpMidi.flags = (uint8_t)(bufferLen >> 8);
        rtpMidi.flags |= RTP_MIDI_CS_FLAG_B;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_J;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_Z;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_P;
        port.write(rtpMidi.flags);
        port.write((uint8_t)(bufferLen));
    }

    // from local buffer onto the network
    while (!buffer.isEmpty())
        port.write(buffer.read());

    port.endPacket();
    port.flush();
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
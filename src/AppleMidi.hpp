#pragma once

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

static byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::readControlPackets()
{
    uint32_t packetSize = controlPort.parsePacket();
    
    // Only proceed if new data comes in
    if (packetSize == 0)
        return;

    while (packetSize > 0)
    {
        auto bytesToRead = min(packetSize, sizeof(packetBuffer));
        auto bytesRead = controlPort.read(packetBuffer, bytesToRead);
        packetSize -= bytesRead;

        for (size_t i = 0; i < bytesRead; i++)
            controlBuffer.write(packetBuffer[i]); // append
    }

#if DEBUG >= LOG_LEVEL_TRACE
    if (controlBuffer.getLength() > 0)
    {
        T_DEBUG_PRINT(F("From control socket, Len: "));
        T_DEBUG_PRINT(controlBuffer.getLength());
        T_DEBUG_PRINT(F(" 0x"));
        for (auto i = 0; i < controlBuffer.getLength(); i++)
        {
            T_DEBUG_PRINT(controlBuffer.peek(i), HEX);
            T_DEBUG_PRINT(" ");
        }
        T_DEBUG_PRINTLN();
    }
#endif

    while (controlBuffer.getLength() > 0)
    {
        int retVal = _appleMIDIParser.parse(controlBuffer, amPortType::Control);
        if (retVal > 0)
        {
            T_DEBUG_PRINTLN(F("OK"));
            break;
        }
        
        if (retVal == PARSER_NOT_ENOUGH_DATA)
        {
            T_DEBUG_PRINTLN(F("control PARSER_NOT_ENOUGH_DATA"));
            break;
        }
        
        // we got something in the buffer that was unexpected.
        // remove first byte and try again
		T_DEBUG_PRINTLN(F("control buffer, parse error, popping 1 byte "));
		dataBuffer.pop(1);
    }
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::readDataPackets()
{
    // TODO: what if packetSize is larger than our buffer
    
    uint32_t packetSize = dataPort.parsePacket();
    
    // Only proceed if new data comes in
    if (packetSize == 0)
        return;
    
    while (packetSize > 0)
    {
        auto bytesToRead = min(packetSize, sizeof(packetBuffer));
        auto bytesRead = dataPort.read(packetBuffer, bytesToRead);
        packetSize -= bytesRead;

        for (size_t i = 0; i < bytesRead; i++)
            dataBuffer.write(packetBuffer[i]);
    }
    
#if DEBUG >= LOG_LEVEL_TRACE
    if (dataBuffer.getLength() > 0)
    {
        T_DEBUG_PRINT(F("From data socket, Len: "));
        T_DEBUG_PRINT(dataBuffer.getLength());
        T_DEBUG_PRINT(F(" 0x"));
        for (auto i = 0; i < dataBuffer.getLength(); i++)
        {
            T_DEBUG_PRINT(dataBuffer.peek(i), HEX);
            T_DEBUG_PRINT(" ");
        }
        T_DEBUG_PRINTLN();
    }
#endif

    while (dataBuffer.getLength() > 0)
    {
        int retVal1 = _rtpMIDIParser.parse(dataBuffer);
        if (retVal1 > 0)
            break;
        int retVal2 = _appleMIDIParser.parse(dataBuffer, amPortType::Data);
        if (retVal2 > 0)
            break;

		if ((retVal1 == PARSER_NOT_ENOUGH_DATA) && (retVal2 == PARSER_NOT_ENOUGH_DATA))
		{
			F_DEBUG_PRINTLN(F("either buffers have enough data"));

			// both have not enough data
			if (dataBuffer.isFull())
			{
				// if it is a SysEx, we can shop it up....

				F_DEBUG_PRINTLN(F("Not enough buffer space to read entire message."));
				F_DEBUG_PRINT(F("Increase the current size in MaxBufferSize from "));
				F_DEBUG_PRINT(Settings::MaxBufferSize);
				F_DEBUG_PRINTLN(F(" to (at least) the next Power-of-Two in Settings"));
			}
			break;
		}

		if (retVal1 == PARSER_NOT_ENOUGH_DATA || retVal2 == PARSER_NOT_ENOUGH_DATA)
        {
            T_DEBUG_PRINTLN(F("data PARSER_NOT_ENOUGH_DATA"));

            break; // one or the other buffer does not have enough data
        }

		T_DEBUG_PRINTLN(F("data buffer, parse error, popping 1 byte "));
		dataBuffer.pop(1);
    }

#ifdef APPLEMIDI_INITIATOR
    ManagePendingInvites();
    ManageTiming();
#endif
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::ReceivedInvitation(AppleMIDI_Invitation &invitation, const amPortType &portType)
{
    if (portType == amPortType::Control)
        ReceivedControlInvitation(invitation);
    else
        ReceivedDataInvitation(invitation);
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::ReceivedControlInvitation(AppleMIDI_Invitation &invitation)
{
    T_DEBUG_PRINTLN(F("Received Control Invitation"));
    T_DEBUG_PRINT("initiator: 0x");
    T_DEBUG_PRINT(invitation.initiatorToken, HEX);
    T_DEBUG_PRINT(", senderSSRC: 0x");
    T_DEBUG_PRINT(invitation.ssrc, HEX);
    T_DEBUG_PRINT(", sessionName: ");
    T_DEBUG_PRINTLN(invitation.sessionName);

    strncpy(invitation.sessionName, localName, APPLEMIDI_SESSION_NAME_MAX_LEN);
    invitation.sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN] = '\0';

    auto participant = getParticipant(invitation.ssrc);
    if (NULL == participant)
    {
        participant = getParticipant(APPLEMIDI_PARTICIPANT_SLOT_FREE);
        if (NULL == participant)
        {
            T_DEBUG_PRINTLN(F("Not free slot found, rejecting"));
            writeInvitation(controlPort, invitation, amInvitationRejected, ssrc);
            return;
        }
        participant->ssrc = invitation.ssrc;
        strncpy(participant->sessionName, invitation.sessionName, APPLEMIDI_SESSION_NAME_MAX_LEN);
    }

    writeInvitation(controlPort, invitation, amInvitationAccepted, ssrc);
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::ReceivedDataInvitation(AppleMIDI_Invitation &invitation)
{
    T_DEBUG_PRINTLN(F("Received Data Invitation"));
    T_DEBUG_PRINT("initiator: 0x");
    T_DEBUG_PRINT(invitation.initiatorToken, HEX);
    T_DEBUG_PRINT(", senderSSRC: 0x");
    T_DEBUG_PRINT(invitation.ssrc, HEX);
    T_DEBUG_PRINT(", sessionName: ");
    T_DEBUG_PRINTLN(invitation.sessionName);

    auto participant = getParticipant(invitation.ssrc);
    if (NULL == participant)
    {
        N_DEBUG_PRINTLN(F("Not free particiants slot, rejecting invitation"));
        writeInvitation(dataPort, invitation, amInvitationRejected, ssrc);
        return;
    }

    writeInvitation(dataPort, invitation, amInvitationAccepted, ssrc);

    // callback in IDE
    if (NULL != _connectedCallback)
        _connectedCallback(invitation.ssrc, invitation.sessionName);
}

/*! \brief .

From: http://en.wikipedia.org/wiki/RTP_MIDI

The session initiator sends a first message (named CK0) to the remote partner, giving its local time on
64 bits (Note that this is not an absolute time, but a time related to a local reference, generally given
in microseconds since the startup of operating system kernel). This time is expressed on 10 kHz sampling
clock basis (100 microseconds per increment) The remote partner must answer to this message with a CK1 message,
containing its own local time on 64 bits. Both partners then know the difference between their respective clocks
and can determine the offset to apply to Timestamp and Deltatime fields in RTP-MIDI protocol. The session
initiator finishes this sequence by sending a last message called CK2, containing the local time when it
received the CK1 message. This technique allows to compute the average latency of the network, and also to
compensate a potential delay introduced by a slow starting thread (this situation can occur with non-realtime
operating systems like Linux, Windows or OS X)

Apple recommends to repeat this sequence a few times just after opening the session, in order to get better
synchronization accuracy (in case of one of the sequence has been delayed accidentally because of a temporary
network overload or a latency peak in a thread activation)

This sequence must repeat cyclically (between 2 and 6 times per minute typically), and always by the session
initiator, in order to maintain long term synchronization accuracy by compensation of local clock drift, and also
to detect a loss of communication partner. A partner not answering to multiple CK0 messages shall consider that
the remote partner is disconnected. In most cases, session initiators switch their state machine into "Invitation"
state in order to re-establish communication automatically as soon as the distant partner reconnects to the
network. Some implementations (especially on personal computers) display also an alert message and offer to the
user to choose between a new connection attempt or closing the session.
*/
template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::ReceivedSynchronization(AppleMIDI_Synchronization &synchronization)
{
    T_DEBUG_PRINT(F("received Synchronization for ssrc 0x"));
    T_DEBUG_PRINTLN(synchronization.ssrc, HEX);

    auto now = rtpMidiClock.Now(); // units of 100 microseconds

    auto participant = getParticipant(synchronization.ssrc);
    if (NULL == participant)
    {
        participant = getParticipant(APPLEMIDI_PARTICIPANT_SLOT_FREE);
        if (NULL == participant)
        {
            V_DEBUG_PRINTLN(F("participant not found, no more space on stack"));
            return;
        }
    }

    switch (synchronization.count)
    {
    case SYNC_CK0: /* From session APPLEMIDI_INITIATOR */
        synchronization.count = SYNC_CK1;
        synchronization.timestamps[synchronization.count] = now;
        break;
    case SYNC_CK1: /* From session responder */
        /* compute media delay */
        //participant->diff = (now - synchronization.timestamps[0]) / 2;
        /* approximate time difference between peer and self */
        //participant->diff = synchronization.timestamps[2] + diff - now;
        // V_DEBUG_PRINT(F("SYNC_CK1 "));
        // V_DEBUG_PRINTLN((uint32_t)(diff));
        // Send CK2
        synchronization.count = SYNC_CK2;
        synchronization.timestamps[synchronization.count] = now;
        /* getting this message means that the responder is still alive! */
        /* remember the time, if it takes to long to respond, we can assume the responder is dead */
        /* not implemented at this stage*/
        //Sessions[index].synchronization.lastTime = now;
        //Sessions[index].synchronization.count++;
        break;
    case SYNC_CK2: /* From session APPLEMIDI_INITIATOR */
        /* compute average delay */
        // participant->diff = (synchronization.timestamps[2] - synchronization.timestamps[0]) / 2;
        /* approximate time difference between peer and self */
        // participant->diff = synchronization.timestamps[2] + diff - now;
        // V_DEBUG_PRINT(F("SYNC_CK2 "));
        // V_DEBUG_PRINTLN((uint32_t)(diff));
        synchronization.count = SYNC_CK0;
        synchronization.timestamps[synchronization.count] = now;
        break;
    }

    // only on the data port
    // Invitation Accepted
    if (dataPort.beginPacket(dataPort.remoteIP(), dataPort.remotePort()))
    {
        dataPort.write((uint8_t *)amSignature, sizeof(amSignature));
        dataPort.write((uint8_t *)amSynchronization, sizeof(amSynchronization));
        synchronization.ssrc = htonl(ssrc);
        synchronization.timestamps[0] = htonll(synchronization.timestamps[0]);
        synchronization.timestamps[1] = htonll(synchronization.timestamps[1]);
        synchronization.timestamps[2] = htonll(synchronization.timestamps[2]);
        dataPort.write(reinterpret_cast<uint8_t *>(&synchronization), sizeof(synchronization));
        dataPort.endPacket();
        dataPort.flush();
    }
    else
    {
        E_DEBUG_PRINTLN("error in dataPort.beginPacket");
    }
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::ReceivedReceiverFeedback(AppleMIDI_ReceiverFeedback &receiverFeedback)
{
    T_DEBUG_PRINTLN(F("ReceivedReceiverFeedback"));
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::ReceivedEndSession(AppleMIDI_EndSession &endSession)
{
    T_DEBUG_PRINTLN(F("receivedEndSession"));
    T_DEBUG_PRINT("initiator: 0x");
    T_DEBUG_PRINT(endSession.initiatorToken, HEX);
    T_DEBUG_PRINT(", senderSSRC: 0x");
    T_DEBUG_PRINTLN(endSession.ssrc, HEX);

    auto participant = getParticipant(endSession.ssrc);
    if (participant)
        participant->ssrc = APPLEMIDI_PARTICIPANT_SLOT_FREE;

    if (NULL != _disconnectedCallback)
        _disconnectedCallback(endSession.ssrc);
}

template <class UdpClass, class Settings>
Participant<Settings> *AppleMidiTransport<UdpClass, Settings>::getParticipant(const ssrc_t ssrc)
{
    for (auto i = 0; i < Settings::MaxNumberOfParticipants; i++)
        if (ssrc == participants[i].ssrc)
            return &participants[i];
    return NULL;
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::writeInvitation(UdpClass &port, AppleMIDI_Invitation &invitation, const byte *command, ssrc_t ssrc)
{
    T_DEBUG_PRINTLN(F("writeInvitation"));

    if (port.beginPacket(port.remoteIP(), port.remotePort()))
    {
        port.write((uint8_t *)amSignature, sizeof(amSignature));
        port.write((uint8_t *)command, sizeof(amInvitationRejected));
        port.write((uint8_t *)amProtocolVersion, sizeof(amProtocolVersion));
        invitation.initiatorToken = htonl(invitation.initiatorToken);
        invitation.ssrc = htonl(ssrc);
        port.write(reinterpret_cast<uint8_t *>(&invitation), invitation.getLength());
        port.endPacket();
        port.flush();
    }
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::writeRtpMidiBuffer(UdpClass &port, RingBuffer<byte, Settings::MaxBufferSize> &buffer, uint16_t sequenceNr, ssrc_t ssrc)
{
    T_DEBUG_PRINT(F("writeRtpMidiBuffer "));

#if DEBUG >= LOG_LEVEL_TRACE
    if (buffer.getLength() > 0)
    {
        T_DEBUG_PRINT(F("to data socket, Len: "));
        T_DEBUG_PRINT(buffer.getLength());
        T_DEBUG_PRINT(F(" 0x"));
        for (auto i = 0; i < buffer.getLength(); i++)
        {
            T_DEBUG_PRINT(buffer.peek(i), HEX);
            T_DEBUG_PRINT(" ");
        }
        T_DEBUG_PRINTLN();
    }
#endif

    T_DEBUG_PRINT(" sequenceNr: ");
    T_DEBUG_PRINTLN(sequenceNr);

    if (!port.beginPacket(port.remoteIP(), port.remotePort()))
    {
        E_DEBUG_PRINTLN(F("Error port.beginPacket host: "));
        E_DEBUG_PRINTLN(port.remoteIP());
        E_DEBUG_PRINTLN(F(", port: "));
        E_DEBUG_PRINTLN(port.remotePort());
        return;
    }

    Rtp rtp;
    rtp.vpxcc = 0b10000000;             // TODO: fun with flags
    rtp.mpayload = PAYLOADTYPE_RTPMIDI; // TODO: set or unset marker
    rtp.ssrc = htonl(ssrc);
    // https://developer.apple.com/library/ios/documentation/CoreMidi/Reference/MIDIServices_Reference/#//apple_ref/doc/uid/TP40010316-CHMIDIServiceshFunctions-SW30
    // The time at which the events occurred, if receiving MIDI, or, if sending MIDI,
    // the time at which the events are to be played. Zero means "now." The time stamp
    // applies to the first MIDI byte in the packet.
    rtp.timestamp = htonl(0UL);
    rtp.sequenceNr = htons(sequenceNr);
    port.write((uint8_t *)&rtp, sizeof(rtp));

    // only now the length is known
    auto bufferLen = buffer.getLength();

    RtpMIDI_t rtpMidi;

    if (bufferLen <= 0x0F)
    { // Short header
        rtpMidi.flags = (uint8_t)bufferLen;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_B;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_J;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_Z;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_P;
        port.write(rtpMidi.flags);
    }
    else
    { // Long header
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

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::ManagePendingInvites()
{
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::ManageTiming()
{
}

template <class UdpClass, class Settings>
void AppleMidiTransport<UdpClass, Settings>::ReceivedMidi(byte data)
{
    inMidiBuffer.write(data);
}

END_APPLEMIDI_NAMESPACE

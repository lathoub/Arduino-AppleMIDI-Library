#pragma once

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

static byte packetBuffer[UDP_TX_PACKET_MAX_SIZE];

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::readControlPackets()
{
    size_t packetSize = controlPort.available();
    if (packetSize == 0)
        packetSize = controlPort.parsePacket();
    
    while (packetSize > 0 && !controlBuffer.full())
    {
        auto bytesToRead = min( min(packetSize, controlBuffer.free()), sizeof(packetBuffer));
        auto bytesRead = controlPort.read(packetBuffer, bytesToRead);
        packetSize -= bytesRead;

        for (auto i = 0; i < bytesRead; i++)
            controlBuffer.push_back(packetBuffer[i]);
    }

    while (controlBuffer.size() > 0)
    {
        auto retVal = _appleMIDIParser.parse(controlBuffer, amPortType::Control);
        if (retVal == parserReturn::UnexpectedData)
        {
            if (NULL != _errorCallback)
                _errorCallback(ssrc, -2);
            dataBuffer.pop_front();
        }
    }
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::readDataPackets()
{
    size_t packetSize = dataPort.available();
    if (packetSize == 0)
        packetSize = dataPort.parsePacket();
    
    while (packetSize > 0 && !dataBuffer.full())
    {
        auto bytesToRead = min( min(packetSize, dataBuffer.free()), sizeof(packetBuffer));
        auto bytesRead = dataPort.read(packetBuffer, bytesToRead);
        packetSize -= bytesRead;

        for (auto i = 0; i < bytesRead; i++)
            dataBuffer.push_back(packetBuffer[i]);
    }

    while (dataBuffer.size() > 0)
    {
        auto retVal1 = _rtpMIDIParser.parse(dataBuffer);
        if (retVal1 == parserReturn::Processed
        ||  retVal1 == parserReturn::NotEnoughData)
            break;
        auto retVal2 = _appleMIDIParser.parse(dataBuffer, amPortType::Data);
        if (retVal2 == parserReturn::Processed
        ||  retVal2 == parserReturn::NotEnoughData)
            break;

        if (retVal1 == parserReturn::NotSureGiveMeMoreData
        &&  retVal2 == parserReturn::NotSureGiveMeMoreData)
			break;

        if (retVal1 == parserReturn::NotSureGiveMeMoreData
        ||  retVal2 == parserReturn::NotSureGiveMeMoreData)
        {
            T_DEBUG_PRINTLN(F("data PARSER_NOT_ENOUGH_DATA"));
            break; // one or the other buffer does not have enough data
        }

        if (NULL != _errorCallback)
            _errorCallback(ssrc, -3);

        T_DEBUG_PRINTLN(F("data buffer, parse error, popping 1 byte "));
		dataBuffer.pop_front();
    }

#ifdef APPLEMIDI_INITIATOR
    managePendingInvites();
    manageTiming();
#endif
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedInvitation(AppleMIDI_Invitation &invitation, const amPortType &portType)
{
    if (portType == amPortType::Control)
        ReceivedControlInvitation(invitation);
    else
        ReceivedDataInvitation(invitation);
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedControlInvitation(AppleMIDI_Invitation &invitation)
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
            if (NULL != _errorCallback)
                _errorCallback(ssrc, -3);

            T_DEBUG_PRINTLN(F("Not free slot found, rejecting"));
            writeInvitation(controlPort, invitation, amInvitationRejected, ssrc);
            return;
        }
        
        participant->ssrc = invitation.ssrc;
        strncpy(participant->sessionName, invitation.sessionName, APPLEMIDI_SESSION_NAME_MAX_LEN);
    }
    else
    {
        T_DEBUG_PRINTLN(F("Received Invitation from unknown ssrc"));
    }
    
    writeInvitation(controlPort, invitation, amInvitationAccepted, ssrc);
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedDataInvitation(AppleMIDI_Invitation &invitation)
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
        if (NULL != _errorCallback)
            _errorCallback(ssrc, -4);

        T_DEBUG_PRINTLN(F("Not free particiants slot, rejecting invitation"));
        writeInvitation(dataPort, invitation, amInvitationRejected, ssrc);
        return;
    }

    writeInvitation(dataPort, invitation, amInvitationAccepted, ssrc);

    // Inform that we have an established connection
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
void AppleMidiSession<UdpClass, Settings>::ReceivedSynchronization(AppleMIDI_Synchronization &synchronization)
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

    // The session initiator sends a first message (named CK0) to the remote partner, giving its local time in
    // 64 bits (Note that this is not an absolute time, but a time related to a local reference,
    // generally given in microseconds since the startup of operating system kernel). This time
    // is expressed on a 10 kHz sampling clock basis (100 microseconds per increment). The remote
    // partner must answer this message with a CK1 message, containing its own local time in 64 bits.
    // Both partners then know the difference between their respective clocks and can determine the
    // offset to apply to Timestamp and Deltatime fields in the RTP-MIDI protocol.
    //
    // The session initiator finishes this sequence by sending a last message called CK2,
    // containing the local time when it received the CK1 message. This technique makes it
    // possible to compute the average latency of the network, and also to compensate for a
    // potential delay introduced by a slow starting thread, which can occur with non-realtime
    // operating systems like Linux, Windows or OS X.

    // -----
    
    // The original initiator initiates clock synchronization after the end of the initial invitation handshake packets.
    // A full clock synchronization exchange is as follows:
    //
    // Initiator sends sync packet with count = 0, current time in timestamp 1
    // Responder sends sync packet with count = 1, current time in timestamp 2, timestamp 1 copied from received packet
    // Initiator sends sync packet with count = 2, current time in timestamp 3, timestamps 1 and 2 copied from received packet
    // At the end of this exchange, each party can estimate the offset between the two clocks using the following formula:
    //
    // offset_estimate = ((timestamp3 + timestamp1) / 2) - timestamp2
    //
    // Furthermore, by maintaining a history of synchronization exchanges, each party can calculate a rate at which the clock offset is changing.
    //
    // The initiator must initiate a new sync exchange at least once every 60 seconds;
    // otherwise the responder may assume that the initiator has died and terminate the session.

    switch (synchronization.count)
    {
    case SYNC_CK0: /* From session APPLEMIDI_INITIATOR */
        V_DEBUG_PRINTLN(F("SYNC_CK0"));
        synchronization.count = SYNC_CK1;
        synchronization.timestamps[synchronization.count] = now;
        break;
    case SYNC_CK1: // From session responder
        V_DEBUG_PRINTLN(F("SYNC_CK1 n/a for the moments"));
        break;
    case SYNC_CK2: /* From session APPLEMIDI_INITIATOR */
        V_DEBUG_PRINTLN(F("SYNC_CK2"));
        // each party can estimate the offset between the two clocks using the following formula
        auto offset_estimate = ((synchronization.timestamps[2] + synchronization.timestamps[0]) / 2) - synchronization.timestamps[1];
        auto diff            = ((synchronization.timestamps[2] + synchronization.timestamps[0]) / 2);
                        diff = synchronization.timestamps[2] + diff - now;
 //       N_DEBUG_PRINT((uint32_t)(diff << 32));
 //       N_DEBUG_PRINTLN((uint32_t)(diff));
        break;
    }

    switch (synchronization.count)
    {
    case SYNC_CK1:
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
        break;
    case SYNC_CK0:
    case SYNC_CK2:
        break;
    }
}

// The recovery journal mechanism requires that the receiver periodically
// inform the sender of the sequence number of the most recently received packet.
// This allows the sender to reduce the size of the recovery journal, to
// encapsulate only those changes to the MIDI stream state occurring after
// the specified packet number.
//
template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedReceiverFeedback(AppleMIDI_ReceiverFeedback &receiverFeedback)
{
    T_DEBUG_PRINTLN(F("ReceivedReceiverFeedback"));
    T_DEBUG_PRINT(F("senderSSRC: 0x"));
    T_DEBUG_PRINT(receiverFeedback.ssrc, HEX);
    T_DEBUG_PRINT(F(", sequence: "));
    T_DEBUG_PRINTLN(receiverFeedback.sequenceNr);
    
    // As we do not keep any recovery journals, no action here
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedEndSession(AppleMIDI_EndSession &endSession)
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
Participant<Settings> *AppleMidiSession<UdpClass, Settings>::getParticipant(const ssrc_t ssrc)
{
    for (auto i = 0; i < Settings::MaxNumberOfParticipants; i++)
        if (ssrc == participants[i].ssrc)
            return &participants[i];
    return NULL;
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::writeInvitation(UdpClass &port, AppleMIDI_Invitation_t invitation, const byte *command, ssrc_t ssrc)
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

        invitation.initiatorToken = htonl(invitation.initiatorToken);
    }
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::writeReceiverFeedback(UdpClass &port, AppleMIDI_ReceiverFeedback_t &receiverFeedback)
{
    T_DEBUG_PRINTLN(F("writeReceiverFeedback"));

    if (port.beginPacket(port.remoteIP(), port.remotePort()))
    {
        port.write((uint8_t *)amSignature, sizeof(amSignature));
        port.write((uint8_t *)amReceiverFeedback, sizeof(amReceiverFeedback));
        port.write(reinterpret_cast<uint8_t *>(&receiverFeedback), sizeof(AppleMIDI_ReceiverFeedback));
        port.endPacket();
        port.flush();
    }
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::writeRtpMidiBuffer(UdpClass &port, RtpBuffer_t &buffer, uint16_t sequenceNr, ssrc_t ssrc, uint32_t timestamp)
{
    T_DEBUG_PRINT(F("writeRtpMidiBuffer "));
        
    T_DEBUG_PRINT(" sequenceNr: ");
    T_DEBUG_PRINTLN(sequenceNr);

    if (!port.beginPacket(port.remoteIP(), port.remotePort()))
    {
        E_DEBUG_PRINT(F("Error port.beginPacket host: "));
        E_DEBUG_PRINT(port.remoteIP());
        E_DEBUG_PRINT(F(", port: "));
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
    //
    // https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html
    //
    // The timestamp is in the same units as described in Timestamp Synchronization
    // (units of 100 microseconds since an arbitrary time in the past). The lower 32 bits of this value
    // is encoded in the packet. The Apple driver may transmit packets with timestamps in the future.
    // Such messages should not be played until the scheduled time. (A future version of the driver may
    // have an option to not transmit messages with future timestamps, to accommodate hardware not
    // prepared to defer rendering the messages until the proper time.)
    //
    rtp.timestamp = htonl(0); // now
  //  rtp.timestamp = htonl(timestamp);
    rtp.sequenceNr = htons(sequenceNr);
    port.write((uint8_t *)&rtp, sizeof(rtp));

    // only now the length is known
    auto bufferLen = buffer.size();

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

    // MIDI Section
    while (!buffer.empty())
        port.write(buffer.pop_front());
    
    // *No* journal section
    
    port.endPacket();
    port.flush();
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::managePendingInvites()
{
    for (auto i = 0; i < Settings::MaxNumberOfParticipants; i++)
    {
        if (participants[i].ssrc == APPLEMIDI_PARTICIPANT_SLOT_FREE)
            continue;
        
        if (millis() - participants[i].lastInviteSentTime <  1000)
        {
            // timeout
            
            participants[i].lastInviteSentTime = millis();

           // this->SendInvitation()
            participants[i].connectionAttempts++;
        }
    }
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::manageTiming()
{
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::manageReceiverFeedback()
{
    for (auto i = 0; i < Settings::MaxNumberOfParticipants; i++)
    {
        if (participants[i].ssrc == APPLEMIDI_PARTICIPANT_SLOT_FREE)
            continue;
        
        if (participants[i].receiverFeedbackStartTime == 0)
            continue;
        
        if ((millis() - participants[i].receiverFeedbackStartTime) > 1000)
        {
            AppleMIDI_ReceiverFeedback_t rf;
            rf.ssrc = htonl(ssrc);
            rf.sequenceNr = htons(participants[i].sequenceNr);
            writeReceiverFeedback(controlPort, rf);

            // reset the clock. It is started when we receive MIDI
            participants[i].receiverFeedbackStartTime = 0;
        }
    }
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedRtp(const Rtp_t& rtp)
{
    auto participant = getParticipant(rtp.ssrc);
    if (NULL != participant)
    {
        if (participant->receiverFeedbackStartTime == 0)
            participant->receiverFeedbackStartTime = millis();
        participant->sequenceNr = rtp.sequenceNr;
    }
    else
    {
        W_DEBUG_PRINT(F("Received an RTP packet from an unknown ssrc: 0x"));
        W_DEBUG_PRINTLN(rtp.ssrc, HEX);
    }
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedMidi(byte value)
{
    if (NULL != _receivedMidiByteCallback)
        _receivedMidiByteCallback(0, value);

    inMidiBuffer.push_back(value);
}

END_APPLEMIDI_NAMESPACE

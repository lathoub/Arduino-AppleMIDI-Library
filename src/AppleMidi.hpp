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

        //  // both don't have data to determine protocol
        if (retVal1 == parserReturn::NotSureGiveMeMoreData
        &&  retVal2 == parserReturn::NotSureGiveMeMoreData)
			break;

        // one or the other don't have enough data to determine the protocol
        if (retVal1 == parserReturn::NotSureGiveMeMoreData
        ||  retVal2 == parserReturn::NotSureGiveMeMoreData)
            break; // one or the other buffer does not have enough data

        // TODO can we ever get here???
        
        if (NULL != _errorCallback)
            _errorCallback(ssrc, -3);

        T_DEBUG_PRINTLN(F("data buffer, parse error, popping 1 byte "));
		dataBuffer.pop_front();
    }

#ifdef APPLEMIDI_INITIATOR
    managePendingInvites();
    manageSynchronization();
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
#ifdef KEEP_SESSION_NAME
    T_DEBUG_PRINT(", sessionName: ");
    T_DEBUG_PRINT(invitation.sessionName);
#endif
    T_DEBUG_PRINTLN();

    // advertise our own session name
    strncpy(invitation.sessionName, localName, APPLEMIDI_SESSION_NAME_MAX_LEN);
    invitation.sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN] = '\0';

    if (participants.full())
    {
        if (NULL != _errorCallback)
            _errorCallback(ssrc, -3);

        T_DEBUG_PRINTLN(F("Not free slot found, rejecting"));
        writeInvitation(controlPort, controlPort.remoteIP(), controlPort.remotePort(), invitation, amInvitationRejected, ssrc);
        return;
    }
    
    Participant<Settings> participant;
    participant.ssrc = invitation.ssrc;
    participant.remoteIP   = controlPort.remoteIP();
    participant.remotePort = controlPort.remotePort();

#ifdef KEEP_SESSION_NAME
    strncpy(participant.sessionName, invitation.sessionName, APPLEMIDI_SESSION_NAME_MAX_LEN);
#endif
    participants.push_back(participant);
    
    writeInvitation(controlPort, participant.remoteIP, participant.remotePort, invitation, amInvitationAccepted, ssrc);
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedDataInvitation(AppleMIDI_Invitation &invitation)
{
    T_DEBUG_PRINTLN(F("Received Data Invitation"));
    T_DEBUG_PRINT("initiator: 0x");
    T_DEBUG_PRINT(invitation.initiatorToken, HEX);
    T_DEBUG_PRINT(", senderSSRC: 0x");
    T_DEBUG_PRINT(invitation.ssrc, HEX);
#ifdef KEEP_SESSION_NAME
    T_DEBUG_PRINT(", sessionName: ");
    T_DEBUG_PRINT(invitation.sessionName);
#endif
    T_DEBUG_PRINTLN();

    auto participant = getParticipant(invitation.ssrc);
    if (NULL == participant)
    {
        if (NULL != _errorCallback)
            _errorCallback(ssrc, -4);

        T_DEBUG_PRINTLN(F("Not free particiants slot, rejecting invitation"));
        writeInvitation(dataPort, dataPort.remoteIP(), dataPort.remotePort(), invitation, amInvitationRejected, ssrc);
        return;
    }

    // writeInvitation will alter the values of the invitation,
    // in order to safe memory and computing cycles its easier to make a copy
    // of the ssrc here.
    auto ssrc_ = invitation.ssrc;
    
    writeInvitation(dataPort, participant->remoteIP, participant->remotePort + 1, invitation, amInvitationAccepted, ssrc);

    participant->kind = Listener;
    
    // Inform that we have an established connection
    if (NULL != _connectedCallback)
        _connectedCallback(ssrc_, invitation.sessionName);
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedInvitationAccepted(AppleMIDI_Invitation &invitationAccepted, const amPortType &portType)
{
    if (portType == amPortType::Control)
        ReceivedControlInvitationAccepted(invitationAccepted);
    else
        ReceivedDataInvitationAccepted(invitationAccepted);
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedControlInvitationAccepted(AppleMIDI_Invitation &invitationAccepted)
{
    T_DEBUG_PRINTLN(F("Received Control InvitationAccepted"));
    T_DEBUG_PRINT("initiator: 0x");
    T_DEBUG_PRINT(invitationAccepted.initiatorToken, HEX);
    T_DEBUG_PRINT(", senderSSRC: 0x");
    T_DEBUG_PRINT(invitationAccepted.ssrc, HEX);
#ifdef KEEP_SESSION_NAME
    T_DEBUG_PRINT(", sessionName: ");
    T_DEBUG_PRINT(invitationAccepted.sessionName);
#endif
    T_DEBUG_PRINTLN();
    
    auto participant = this->getParticipantUsingToken(invitationAccepted.initiatorToken);
    if (NULL == participant)
    {
        T_DEBUG_PRINTLN("Participant not found using initiatorToken");
        // TODO: should not end up here
        return;
    }
    
    participant->ssrc               = invitationAccepted.ssrc;
    participant->lastInviteSentTime = millis() - 1000; // forces invite to be send
    participant->connectionAttempts = 0;
    participant->status             = ControlInvitationAccepted; // step it up
#ifdef KEEP_SESSION_NAME
    strncpy(participant->sessionName, invitationAccepted.sessionName, APPLEMIDI_SESSION_NAME_MAX_LEN);
    participant->sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN] = '\0';
#endif
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::ReceivedDataInvitationAccepted(AppleMIDI_Invitation &invitationAccepted)
{
    T_DEBUG_PRINTLN(F("Received Data Invitation"));
    T_DEBUG_PRINT("initiator: 0x");
    T_DEBUG_PRINT(invitationAccepted.initiatorToken, HEX);
    T_DEBUG_PRINT(", senderSSRC: 0x");
    T_DEBUG_PRINT(invitationAccepted.ssrc, HEX);
#ifdef KEEP_SESSION_NAME
    T_DEBUG_PRINT(", sessionName: ");
    T_DEBUG_PRINT(invitationAccepted.sessionName);
#endif
    T_DEBUG_PRINTLN();
    
    auto participant = this->getParticipantUsingToken(invitationAccepted.initiatorToken);
    if (NULL == participant)
    {
        T_DEBUG_PRINTLN("Participant not found using initiatorToken");
        // TODO: should not end up here
        return;
    }
    
    T_DEBUG_PRINTLN(F("Fully setup"));

    participant->status = DataInvitationAccepted;
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

    auto participant = getParticipant(synchronization.ssrc);
    if (NULL == participant)
    {
        V_DEBUG_PRINT(F("SYNC for unknown participant with ssrc 0x"));
        V_DEBUG_PRINT(synchronization.ssrc);

        return;
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
        synchronization.timestamps[1] = rtpMidiClock.Now();
        synchronization.count = SYNC_CK1;
        writeSynchronization(participant->remoteIP, participant->remotePort, synchronization);
        break;
    case SYNC_CK1: /* From session LISTENER */
        V_DEBUG_PRINTLN(F("SYNC_CK1"));
        synchronization.timestamps[2] = rtpMidiClock.Now();
        synchronization.count = SYNC_CK2;
        writeSynchronization(participant->remoteIP, participant->remotePort, synchronization);
        participant->doSynchronization = true;
        break;
    case SYNC_CK2: /* From session APPLEMIDI_INITIATOR */
        V_DEBUG_PRINTLN(F("SYNC_CK2"));
        // each party can estimate the offset between the two clocks using the following formula
        auto offset_estimate = ((synchronization.timestamps[2] + synchronization.timestamps[0]) / 2) - synchronization.timestamps[1];
        synchronization.count = SYNC_CK2;
        break;
    }

    switch (synchronization.count)
    {
    case SYNC_CK0:
    case SYNC_CK1:
    case SYNC_CK2:
        _lastSyncExchangeTime = millis();
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

    for (auto i = 0; i < participants.size(); i++)
    {
        if (endSession.ssrc == participants[i].ssrc)
        {
            participants.erase(i);
            break;
        }
    }

    if (NULL != _disconnectedCallback)
        _disconnectedCallback(endSession.ssrc);
}

template <class UdpClass, class Settings>
Participant<Settings>* AppleMidiSession<UdpClass, Settings>::getParticipant(const ssrc_t ssrc)
{
    for (auto i = 0; i < participants.size(); i++)
        if (ssrc == participants[i].ssrc)
            return &participants[i];
    return NULL;
}

template <class UdpClass, class Settings>
Participant<Settings>* AppleMidiSession<UdpClass, Settings>::getParticipantUsingToken(const uint32_t initiatorToken)
{
    for (auto i = 0; i < participants.size(); i++)
        if (initiatorToken == participants[i].initiatorToken)
            return &participants[i];
    return NULL;
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::writeInvitation(UdpClass &port, IPAddress remoteIP, uint16_t remotePort, AppleMIDI_Invitation_t & invitation, const byte *command, ssrc_t ssrc)
{
    T_DEBUG_PRINTLN(F("writeInvitation"));

    if (port.beginPacket(remoteIP, remotePort))
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
    else
        T_DEBUG_PRINT(F("beginPacket error"));
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::writeReceiverFeedback(const IPAddress& remoteIP, const uint16_t & remotePort, AppleMIDI_ReceiverFeedback_t & receiverFeedback)
{
    T_DEBUG_PRINTLN(F("writeReceiverFeedback"));

    if (controlPort.beginPacket(remoteIP, remotePort))
    {
        controlPort.write((uint8_t *)amSignature, sizeof(amSignature));
        
            controlPort.write((uint8_t *)amReceiverFeedback, sizeof(amReceiverFeedback));
            controlPort.write(reinterpret_cast<uint8_t *>(&receiverFeedback), sizeof(AppleMIDI_ReceiverFeedback));
        
        controlPort.endPacket();
        controlPort.flush();
    }
    else
        T_DEBUG_PRINT(F("beginPacket error"));
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::writeSynchronization(const IPAddress& remoteIP, const uint16_t & remotePort, AppleMIDI_Synchronization &synchronization)
{
    if (dataPort.beginPacket(remoteIP, remotePort))
    {
        dataPort.write((uint8_t *)amSignature, sizeof(amSignature));
        dataPort.write((uint8_t *)amSynchronization, sizeof(amSynchronization));
        synchronization.ssrc = htonl(this->ssrc);

        synchronization.timestamps[0] = htonll(synchronization.timestamps[0]);
        synchronization.timestamps[1] = htonll(synchronization.timestamps[1]);
        synchronization.timestamps[2] = htonll(synchronization.timestamps[2]);
        dataPort.write(reinterpret_cast<uint8_t *>(&synchronization), sizeof(synchronization));
        
        dataPort.endPacket();
        dataPort.flush();
    }
    else
        T_DEBUG_PRINT(F("beginPacket error"));
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::writeRtpMidiToAllParticipants()
{
    for (auto i = 0; i < participants.size(); i++)
    {
        auto participant = &participants[i];
        writeRtpMidiBuffer(participant->remoteIP, participant->remotePort);
    }
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::writeRtpMidiBuffer(const IPAddress& remoteIP, const uint16_t& remotePort)
{
    T_DEBUG_PRINT(F("writeRtpMidiBuffer "));
    
    if (!dataPort.beginPacket(remoteIP, remotePort))
    {
        E_DEBUG_PRINT(F("Error dataPort.beginPacket "));
        return;
    }

    sequenceNr++; // (modulo 2^16) modulo is automatically done for us ()
    
    T_DEBUG_PRINT(" sequenceNr: ");
    T_DEBUG_PRINTLN(sequenceNr);

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
  //  rtp.timestamp = htonl(rtpMidiClock.Now());
    rtp.sequenceNr = htons(sequenceNr);
    dataPort.write((uint8_t *)&rtp, sizeof(rtp));

    // only now the length is known
    auto bufferLen = outMidiBuffer.size();

    RtpMIDI_t rtpMidi;

    if (bufferLen <= 0x0F)
    { // Short header
        rtpMidi.flags = (uint8_t)bufferLen;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_B; // TODO: set or clear these flags (no journal)
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_J;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_Z;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_P;
        dataPort.write(rtpMidi.flags);
    }
    else
    { // Long header
        rtpMidi.flags = (uint8_t)(bufferLen >> 8);
        rtpMidi.flags |= RTP_MIDI_CS_FLAG_B; // set B-FLAG for long header
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_J;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_Z;
        // rtpMidi.flags &= RTP_MIDI_CS_FLAG_P;
        dataPort.write(rtpMidi.flags);
        dataPort.write((uint8_t)(bufferLen));
    }

    // MIDI Section
    while (!outMidiBuffer.empty())
        dataPort.write(outMidiBuffer.pop_front());
    
    // *No* journal section (Not supported)
    
    dataPort.endPacket();
    dataPort.flush();
}

//
// The initiator of the session polls if remote station is still alive.
//
template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::manageSynchronization()
{
   for (auto i = 0; i < participants.size(); i++)
   {
       auto participant = &participants[i];

       if (participant->kind == Listener)
           continue;

       if (participant->status != Connected)
           continue;

       if (!participant->doSynchronization)
           continue;
     
       bool doSyncronize = false;
       if (participant->syncronizationCount < 2)
       {
           if (millis() - participant->lastInviteSentTime >  500) // 2 x every 0.5 seconds
           {
               participant->syncronizationCount++;
               doSyncronize = true;
           }
       }
       else if (participant->syncronizationCount < 7)
       {
           if (millis() - participant->lastInviteSentTime >  1500) // 5 x every 1.5 seconds
           {
               participant->syncronizationCount++;
               doSyncronize = true;
           }
       }
       else if (millis() - participant->lastInviteSentTime >  10000)
       {
           doSyncronize = true;
       }

       if (!doSyncronize)
           continue;
     
       participant->lastInviteSentTime = millis();
       participant->doSynchronization = false;
       
       AppleMIDI_Synchronization synchronization;
       synchronization.timestamps[0] = rtpMidiClock.Now();
       synchronization.timestamps[1] = 0;
       synchronization.timestamps[2] = 0;
       synchronization.count = 0;

       writeSynchronization(participant->remoteIP, participant->remotePort, synchronization);
   }
}

template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::managePendingInvites()
{
    for (auto i = 0; i < participants.size(); i++)
    {
        auto participant = &participants[i];

        if (participant->kind == Listener)
            continue;

        if (participant->status == DataInvitationAccepted)
        {
            participant->status = Connected;
            
            // Inform that we have an established connection
            if (NULL != _connectedCallback)
                _connectedCallback(participant->ssrc, participant->sessionName);

            participant->doSynchronization = true;
        }

        if (participant->status == Connected)
            continue;

        // Only for Connected Initiators
        \
        if (millis() - participant->lastInviteSentTime >  1000)
        {
            if (participant->connectionAttempts >= 10)
            {
                 // too many attempts, give up - indicate this participant slot is free
                participants.erase(i);
                continue;
            }
            
            participant->lastInviteSentTime = millis();
            participant->connectionAttempts++;

            AppleMIDI_Invitation invitation;
            invitation.ssrc = this->ssrc;
            invitation.initiatorToken = participant->initiatorToken;
            strncpy(invitation.sessionName, this->localName, APPLEMIDI_SESSION_NAME_MAX_LEN);
            invitation.sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN] = '\0';

            if (participant->status == Initiating)
            {
                writeInvitation(controlPort, participant->remoteIP, participant->remotePort, invitation, amInvitation, ssrc);
                participant->status = AwaitingControlInvitationAccepted;
            }
            else if (participant->status == ControlInvitationAccepted)
            {
                writeInvitation(dataPort, participant->remoteIP, participant->remotePort + 1, invitation, amInvitation, ssrc);
                participant->status = AwaitingDataInvitationAccepted;
            }
        }
    }
}

// https://en.wikipedia.org/wiki/RTP-MIDI#Synchronization_sequence
// A partner not answering multiple CK0 messages shall consider
// that the remote partner is disconnected.
template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::manageSyncExchange()
{
    // This routine is only for listeners
/*
    if (millis() - _lastSyncExchangeTime > 60000)
    {
        // sender Quit
    //    controlPort.stop();
    //    dataPort.stop();
    }
 */
}

// The recovery journal mechanism requires that the receiver
// periodically inform the sender of the sequence number of the most
// recently received packet. This allows the sender to reduce the size
// of the recovery journal, to encapsulate only those changes to the
// MIDI stream state occurring after the specified packet number.
//
// This message is sent on the control port.
template <class UdpClass, class Settings>
void AppleMidiSession<UdpClass, Settings>::manageReceiverFeedback()
{
    for (auto i = 0; i < participants.size(); i++)
    {
        auto participant = &participants[i];
        
        if (participant->receiverFeedbackStartTime == 0)
            continue;

        if ((millis() - participant->receiverFeedbackStartTime) > 1000)
        {
            AppleMIDI_ReceiverFeedback_t rf;
            rf.ssrc = htonl(ssrc);
            rf.sequenceNr = htons(participant->sequenceNr);
            writeReceiverFeedback(participant->remoteIP, participant->remotePort, rf);

            // reset the clock. It is started when we receive MIDI
            participant->receiverFeedbackStartTime = 0;
        }
    }
}

template <class UdpClass, class Settings>
bool AppleMidiSession<UdpClass, Settings>::sendInvite(IPAddress ip, uint16_t port)
{
    W_DEBUG_PRINTLN(F("sendInvite"));

    if (participants.full())
        return false;
    
    Participant<Settings> participant;
    participant.kind = Initiator;
    participant.remoteIP = ip;
    participant.remotePort = port;
    participant.lastInviteSentTime = millis() - 1000; // forces invite to be send immediately
    participant.connectionAttempts = 0;
    participant.status = Initiating;
    participant.syncronizationCount = 0;
    participant.initiatorToken = random(1, INT32_MAX) * 2; // 0xb7062030;
    
    participants.push_back(participant);

    return true;
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
        // TODO??? re-connect?
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

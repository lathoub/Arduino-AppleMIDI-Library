#pragma once

#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::readControlPackets()
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
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::parseControlPackets()
{
    while (controlBuffer.size() > 0)
    {
        auto retVal = _appleMIDIParser.parse(controlBuffer, amPortType::Control);
        if (retVal == parserReturn::UnexpectedData)
        {
            if (NULL != _exceptionCallback)
                _exceptionCallback(ssrc, ParseException);
            
            controlBuffer.pop_front();
        }
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::readDataPackets()
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
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::parseDataPackets()
{
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
        
        if (NULL != _exceptionCallback)
            _exceptionCallback(ssrc, UnexpectedParseException);

         dataBuffer.pop_front();
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedInvitation(AppleMIDI_Invitation_t &invitation, const amPortType &portType)
{
    if (portType == amPortType::Control)
        ReceivedControlInvitation(invitation);
    else
        ReceivedDataInvitation(invitation);
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedControlInvitation(AppleMIDI_Invitation_t &invitation)
{
    // advertise our own session name
#ifdef KEEP_SESSION_NAME
    strncpy(invitation.sessionName, localName, DefaultSettings::MaxSessionNameLen);
    invitation.sessionName[DefaultSettings::MaxSessionNameLen] = '\0';
#endif
    
    if (participants.full())
    {
        writeInvitation(controlPort, controlPort.remoteIP(), controlPort.remotePort(), invitation, amInvitationRejected);
        
        if (NULL != _exceptionCallback)
            _exceptionCallback(ssrc, TooManyParticipantsException);

        return;
    }
    
    Participant<Settings> participant;
    participant.kind = Listener;
    participant.ssrc = invitation.ssrc;
    participant.remoteIP   = controlPort.remoteIP();
    participant.remotePort = controlPort.remotePort();
    participant.lastSyncExchangeTime = now;
    participant.sequenceNr = random(1, UINT16_MAX); // // http://www.rfc-editor.org/rfc/rfc6295.txt , 2.1.  RTP Header
#ifdef KEEP_SESSION_NAME
    strncpy(participant.sessionName, invitation.sessionName, DefaultSettings::MaxSessionNameLen);
#endif
    
    participants.push_back(participant);

    writeInvitation(controlPort, participant.remoteIP, participant.remotePort, invitation, amInvitationAccepted);
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedDataInvitation(AppleMIDI_Invitation &invitation)
{
    auto participant = getParticipantBySSRC(invitation.ssrc);
    if (NULL == participant)
    {
        writeInvitation(dataPort, dataPort.remoteIP(), dataPort.remotePort(), invitation, amInvitationRejected);

        if (NULL != _exceptionCallback)
            _exceptionCallback(ssrc, ParticipantNotFoundException);
        
        return;
    }

    // writeInvitation will alter the values of the invitation,
    // in order to safe memory and computing cycles its easier to make a copy
    // of the ssrc here.
    auto ssrc_ = invitation.ssrc;
    
    writeInvitation(dataPort, participant->remoteIP, participant->remotePort + 1, invitation, amInvitationAccepted);

    participant->kind = Listener;
    
    // Inform that we have an established connection
    if (NULL != _connectedCallback)
#ifdef KEEP_SESSION_NAME
        _connectedCallback(ssrc_, invitation.sessionName);
#else
        _connectedCallback(ssrc_, NULL);
#endif
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedBitrateReceiveLimit(AppleMIDI_BitrateReceiveLimit &)
{
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedInvitationAccepted(AppleMIDI_InvitationAccepted_t &invitationAccepted, const amPortType &portType)
{
    if (portType == amPortType::Control)
        ReceivedControlInvitationAccepted(invitationAccepted);
    else
        ReceivedDataInvitationAccepted(invitationAccepted);
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedControlInvitationAccepted(AppleMIDI_InvitationAccepted_t &invitationAccepted)
{
    auto participant = this->getParticipantByInitiatorToken(invitationAccepted.initiatorToken);
    if (NULL == participant)
    {
        return;
    }
    
    participant->ssrc               = invitationAccepted.ssrc;
    participant->lastInviteSentTime = now - 1000; // forces invite to be send
    participant->connectionAttempts = 0; // reset back to 0
    participant->invitationStatus   = ControlInvitationAccepted; // step it up
#ifdef KEEP_SESSION_NAME
    strncpy(participant->sessionName, invitationAccepted.sessionName, DefaultSettings::MaxSessionNameLen);
    participant->sessionName[DefaultSettings::MaxSessionNameLen] = '\0';
#endif
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedDataInvitationAccepted(AppleMIDI_InvitationAccepted_t &invitationAccepted)
{
    auto participant = this->getParticipantByInitiatorToken(invitationAccepted.initiatorToken);
    if (NULL == participant)
    {
        return;
    }
    
    participant->invitationStatus = DataInvitationAccepted;
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedInvitationRejected(AppleMIDI_InvitationRejected_t & invitationRejected)
{
    for (auto i = 0; i < participants.size(); i++)
    {
        if (invitationRejected.ssrc == participants[i].ssrc)
        {
            participants.erase(i);
            
            return;
        }
    }
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
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedSynchronization(AppleMIDI_Synchronization_t &synchronization)
{
    auto participant = getParticipantBySSRC(synchronization.ssrc);
    if (NULL == participant)
    {
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
        synchronization.timestamps[SYNC_CK1] = rtpMidiClock.Now();
        synchronization.count = SYNC_CK1;
        writeSynchronization(participant->remoteIP, participant->remotePort + 1, synchronization);
        break;
    case SYNC_CK1: /* From session LISTENER */
#ifdef APPLEMIDI_INITIATOR
        synchronization.timestamps[SYNC_CK2] = rtpMidiClock.Now();
        synchronization.count = SYNC_CK2;
        writeSynchronization(participant->remoteIP, participant->remotePort + 1, synchronization);
        participant->synchronizing = false;
#endif
        break;
    case SYNC_CK2: /* From session APPLEMIDI_INITIATOR */
            
#ifdef LATENCY_CALCULATION
        // each party can estimate the offset between the two clocks using the following formula
        participant->offsetEstimate = (uint32_t)(((synchronization.timestamps[2] + synchronization.timestamps[0]) / 2) - synchronization.timestamps[1]);
/*
        uint64_t remoteAverage   = ((synchronization.timestamps[2] + synchronization.timestamps[0]) / 2);
        uint64_t localAverage    =   synchronization.timestamps[1];

        static uint64_t oldRemoteAverage = 0;
        static uint64_t oldLocalAverage  = 0;

        uint64_t r = (remoteAverage - oldRemoteAverage);
        uint64_t l = (localAverage  - oldLocalAverage);

        oldRemoteAverage = remoteAverage;
        oldLocalAverage  = localAverage;
*/
#endif
        break;
    }

    // All particpants need to check in regularly,
    // failing to do so will result in a lost connection.
    participant->lastSyncExchangeTime = now;
}

// The recovery journal mechanism requires that the receiver periodically
// inform the sender of the sequence number of the most recently received packet.
// This allows the sender to reduce the size of the recovery journal, to
// encapsulate only those changes to the MIDI stream state occurring after
// the specified packet number.
//
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedReceiverFeedback(AppleMIDI_ReceiverFeedback_t &receiverFeedback)
{
    // As we do not keep any recovery journals, no command history, nothing!
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedEndSession(AppleMIDI_EndSession_t &endSession)
{
    for (size_t i = 0; i < participants.size(); i++)
    {
        if (endSession.ssrc == participants[i].ssrc)
        {
            participants.erase(i);
            
            if (NULL != _disconnectedCallback)
                _disconnectedCallback(participants[i].ssrc);

            return;
        }
    }
}

template <class UdpClass, class Settings, class Platform>
Participant<Settings>* AppleMIDISession<UdpClass, Settings, Platform>::getParticipantBySSRC(const ssrc_t ssrc)
{
    for (size_t i = 0; i < participants.size(); i++)
        if (ssrc == participants[i].ssrc)
            return &participants[i];
    return NULL;
}

template <class UdpClass, class Settings, class Platform>
Participant<Settings>* AppleMIDISession<UdpClass, Settings, Platform>::getParticipantByInitiatorToken(const uint32_t initiatorToken)
{
    for (auto i = 0; i < participants.size(); i++)
        if (initiatorToken == participants[i].initiatorToken)
            return &participants[i];
    return NULL;
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeInvitation(UdpClass &port, IPAddress remoteIP, uint16_t remotePort, AppleMIDI_Invitation_t & invitation, const byte *command)
{
    if (port.beginPacket(remoteIP, remotePort))
    {
        port.write((uint8_t *)amSignature, sizeof(amSignature));
        
            port.write((uint8_t *)command, sizeof(amInvitation));
            port.write((uint8_t *)amProtocolVersion, sizeof(amProtocolVersion));
            invitation.initiatorToken = htonl(invitation.initiatorToken);
            invitation.ssrc = htonl(ssrc);
            port.write(reinterpret_cast<uint8_t *>(&invitation), invitation.getLength());
        
        port.endPacket();
        port.flush();
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeReceiverFeedback(const IPAddress& remoteIP, const uint16_t & remotePort, AppleMIDI_ReceiverFeedback_t & receiverFeedback)
{
    if (controlPort.beginPacket(remoteIP, remotePort))
    {
        controlPort.write((uint8_t *)amSignature, sizeof(amSignature));
        
            controlPort.write((uint8_t *)amReceiverFeedback, sizeof(amReceiverFeedback));
        
            receiverFeedback.ssrc       = htonl(receiverFeedback.ssrc);
            receiverFeedback.sequenceNr = htons(receiverFeedback.sequenceNr);
        
            controlPort.write(reinterpret_cast<uint8_t *>(&receiverFeedback), sizeof(AppleMIDI_ReceiverFeedback));
        
        controlPort.endPacket();
        controlPort.flush();
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeSynchronization(const IPAddress& remoteIP, const uint16_t & remotePort, AppleMIDI_Synchronization_t &synchronization)
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
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeEndSession(const IPAddress& remoteIP, const uint16_t & remotePort, AppleMIDI_EndSession_t &endSession)
{
    if (controlPort.beginPacket(remoteIP, remotePort))
    {
        controlPort.write((uint8_t *)amSignature, sizeof(amSignature));
        controlPort.write((uint8_t *)amEndSession, sizeof(amEndSession));
        controlPort.write((uint8_t *)amProtocolVersion, sizeof(amProtocolVersion));

        endSession.initiatorToken = htonl(endSession.initiatorToken);
        endSession.ssrc           = htonl(endSession.ssrc);

        controlPort.write(reinterpret_cast<uint8_t *>(&endSession), sizeof(endSession));
        
        controlPort.endPacket();
        controlPort.flush();
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeRtpMidiToAllParticipants()
{
    for (size_t i = 0; i < participants.size(); i++)
    {
        auto participant = &participants[i];
        writeRtpMidiBuffer(participant);
    }
    outMidiBuffer.clear();
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeRtpMidiBuffer(Participant<Settings> * participant)
{
    const IPAddress remoteIP   = participant->remoteIP;
    const uint16_t  remotePort = participant->remotePort + 1;
    
    if (!dataPort.beginPacket(remoteIP, remotePort))
        return;

    participant->sequenceNr++; // (modulo 2^16) modulo is automatically done for us ()
    
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
    rtp.timestamp = (Settings::TimestampRtpPackets) ? htonl(rtpMidiClock.Now()) : 0;
    
    rtp.sequenceNr = htons(participant->sequenceNr);
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
    
    // write out the MIDI Section
    for (auto i = 0; i < bufferLen; i++)
        dataPort.write(outMidiBuffer[i]);
    
    // *No* journal section (Not supported)
    
    dataPort.endPacket();
    dataPort.flush();
}

//
//
//
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSynchronization()
{
    for (size_t i = 0; i < participants.size(); i++)
    {
#ifdef APPLEMIDI_INITIATOR
        auto participant = &participants[i];

        if (participant->invitationStatus != Connected)
            continue;
        
        // Only for Initiators that are Connected
        if (participant->kind == Listener)
        {
#endif
            manageSynchronizationListener(i);
#ifdef APPLEMIDI_INITIATOR
        }
        else
        {
            (participant->synchronizing) ? manageSynchronizationInitiatorInvites(i)
                                         : manageSynchronizationInitiatorHeartBeat(i);
        }
#endif
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSynchronizationListener(size_t i)
{
    auto participant = &participants[i];

    // The initiator must check in with the listener at least once every 60 seconds;
    // otherwise the responder may assume that the initiator has died and terminate the session.
    if (now - participant->lastSyncExchangeTime > Settings::CK_MaxTimeOut)
    {
        if (NULL != _exceptionCallback)
            _exceptionCallback(ssrc, ListenerTimeOutException);

        sendEndSession(participant);
        
        participants.erase(i);
        
        return;
    }
}

//
// The initiator of the session polls if remote station is still alive.
// (Initiators only)
//
// The initiator must initiate a new sync exchange at least once every 60 seconds;
// otherwise the responder may assume that the initiator has died and terminate the session.
//
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSynchronizationInitiatorHeartBeat(size_t i)
{
    auto participant = &participants[i];
    
    // Note: During startup, the initiator should send synchronization exchanges more frequently;
    // empirical testing has determined that sending a few exchanges improves clock
    // synchronization accuracy.
    // (Here: twice every 0.5 seconds, then 6 times every 1.5 seconds, then every 10 seconds.)
    bool doSyncronize = false;
    if (participant->synchronizationHeartBeats < 2)
    {
       if (now - participant->lastInviteSentTime >  500) // 2 x every 0.5 seconds
       {
           participant->synchronizationHeartBeats++;
           doSyncronize = true;
       }
    }
    else if (participant->synchronizationHeartBeats < 7)
    {
       if (now - participant->lastInviteSentTime >  1500) // 5 x every 1.5 seconds
       {
           participant->synchronizationHeartBeats++;
           doSyncronize = true;
       }
    }
    else if (now - participant->lastInviteSentTime >  DefaultSettings::SynchronizationHeartBeat)
    {
       doSyncronize = true;
    }

    if (!doSyncronize)
       return;

    participant->synchronizationCount = 0;
    sendSynchronization(participant);
}

// checks for
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSynchronizationInitiatorInvites(size_t i)
{
    auto participant = &participants[i];

    if (now - participant->lastInviteSentTime >  10000)
    {
        if (participant->synchronizationCount > DefaultSettings::MaxSynchronizationCK0Attempts)
        {
            if (NULL != _exceptionCallback)
                _exceptionCallback(ssrc, MaxAttemptsException);

            // After too many attempts, stop.
            sendEndSession(participant);

            participants.erase(i);
            return;
        }
        sendSynchronization(participant);
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::sendSynchronization(Participant<Settings>* participant)
{
    AppleMIDI_Synchronization_t synchronization;
    synchronization.timestamps[SYNC_CK0] = rtpMidiClock.Now();
    synchronization.timestamps[SYNC_CK1] = 0;
    synchronization.timestamps[SYNC_CK2] = 0;
    synchronization.count = 0;

    writeSynchronization(participant->remoteIP, participant->remotePort + 1, synchronization);
    participant->synchronizing = true;
    participant->synchronizationCount++;
    participant->lastInviteSentTime = now;
}

// (Initiators only)
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSessionInvites()
{
    for (auto i = 0; i < participants.size(); i++)
    {
        auto participant = &participants[i];

        if (participant->kind == Listener)
            continue;

        if (participant->invitationStatus == DataInvitationAccepted)
        {
            // Inform that we have an established connection
            if (NULL != _connectedCallback)
#ifdef KEEP_SESSION_NAME
                _connectedCallback(participant->ssrc, participant->sessionName);
#else
                _connectedCallback(participant->ssrc, NULL);
#endif
            participant->invitationStatus = Connected;
        }

        if (participant->invitationStatus == Connected)
            continue; // We are done here

        // try to connect every 1 second (1000 ms)
        if (now - participant->lastInviteSentTime >  1000)
        {
            if (participant->connectionAttempts >= DefaultSettings::MaxSessionInvitesAttempts)
            {
                if (NULL != _exceptionCallback)
                    _exceptionCallback(ssrc, NoResponseFromConnectionRequestException);

                // After too many attempts, stop.
                sendEndSession(participant);
                
                participants.erase(i);

                continue;
            }

            participant->lastInviteSentTime = now;
            participant->connectionAttempts++;

            AppleMIDI_Invitation invitation;
            invitation.ssrc = this->ssrc;
            invitation.initiatorToken = participant->initiatorToken;
#ifdef KEEP_SESSION_NAME
            strncpy(invitation.sessionName, this->localName, DefaultSettings::MaxSessionNameLen);
            invitation.sessionName[DefaultSettings::MaxSessionNameLen] = '\0';
#endif
            if (participant->invitationStatus == Initiating
            ||  participant->invitationStatus == AwaitingControlInvitationAccepted)
            {
                writeInvitation(controlPort, participant->remoteIP, participant->remotePort, invitation, amInvitation);
                participant->invitationStatus = AwaitingControlInvitationAccepted;
            }
            else
            if (participant->invitationStatus == ControlInvitationAccepted
            ||  participant->invitationStatus == AwaitingDataInvitationAccepted)
            {
                writeInvitation(dataPort, participant->remoteIP, participant->remotePort + 1, invitation, amInvitation);
                participant->invitationStatus = AwaitingDataInvitationAccepted;
            }
        }
    }
}

// The recovery journal mechanism requires that the receiver
// periodically inform the sender of the sequence number of the most
// recently received packet. This allows the sender to reduce the size
// of the recovery journal, to encapsulate only those changes to the
// MIDI stream state occurring after the specified packet number.
//
// This message is sent on the control port.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageReceiverFeedback()
{
    for (size_t i = 0; i < participants.size(); i++)
    {
        auto participant = &participants[i];
        
        if (participant->doReceiverFeedback == false)
            continue;

        if ((now - participant->receiverFeedbackStartTime) > Settings::ReceiversFeedbackThreshold)
        {
            AppleMIDI_ReceiverFeedback_t rf;
            rf.ssrc       = ssrc;
            rf.sequenceNr = participant->sequenceNr;
            writeReceiverFeedback(participant->remoteIP, participant->remotePort, rf);

            // reset the clock. It is started when we receive MIDI
            participant->doReceiverFeedback = false;
        }
    }
}

#ifdef APPLEMIDI_INITIATOR

template <class UdpClass, class Settings, class Platform>
bool AppleMIDISession<UdpClass, Settings, Platform>::sendInvite(IPAddress ip, uint16_t port)
{
    if (participants.full())
    {
        return false;
    }
    
    Participant<Settings> participant;
    participant.kind = Initiator;
    participant.remoteIP = ip;
    participant.remotePort = port;
    participant.lastInviteSentTime = now - 1000; // forces invite to be send immediately
    participant.lastSyncExchangeTime = now;
    participant.initiatorToken = random(1, INT32_MAX) * 2; // 0xb7062030;
    participant.sequenceNr = random(1, UINT16_MAX); // // http://www.rfc-editor.org/rfc/rfc6295.txt , 2.1.  RTP Header

    participants.push_back(participant);

    return true;
}

#endif

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::sendEndSession()
{
    while (participants.size() > 0)
    {
        auto participant = &participants.front();
        sendEndSession(participant);

        participants.pop_front();
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::sendEndSession(Participant<Settings>* participant)
{
    AppleMIDI_EndSession_t endSession;
    endSession.initiatorToken = 0;
    endSession.ssrc = this->ssrc;
    writeEndSession(participant->remoteIP, participant->remotePort, endSession);
    
    if (NULL != _disconnectedCallback)
        _disconnectedCallback(participant->ssrc);
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedRtp(const Rtp_t& rtp)
{
    auto participant = getParticipantBySSRC(rtp.ssrc);
    
    if (NULL != participant)
    {
        if (participant->doReceiverFeedback == false)
            participant->receiverFeedbackStartTime = now;
        participant->doReceiverFeedback = true;

#ifdef LATENCY_CALCULATION
        auto offset = (rtp.timestamp - participant->offsetEstimate);
        auto latency = (int32_t)(rtpMidiClock.Now() - offset);
#else
        auto latency = 0;
#endif
        participant->sequenceNr = rtp.sequenceNr;
        
        if (NULL != _receivedRtpCallback)
            _receivedRtpCallback(0, rtp, latency);
    }
    else
    {
        // TODO??? re-connect?
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::StartReceivedMidi()
{
   if (NULL != _startReceivedMidiByteCallback)
        _startReceivedMidiByteCallback(0);
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedMidi(byte value)
{
    if (NULL != _receivedMidiByteCallback)
        _receivedMidiByteCallback(0, value);

    inMidiBuffer.push_back(value);
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::EndReceivedMidi()
{
    if (NULL != _endReceivedMidiByteCallback)
        _endReceivedMidiByteCallback(0);
}

END_APPLEMIDI_NAMESPACE

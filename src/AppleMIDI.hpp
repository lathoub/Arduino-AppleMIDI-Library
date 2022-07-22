#pragma once

#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <class UdpClass, class Settings, class Platform>
size_t AppleMIDISession<UdpClass, Settings, Platform>::readControlPackets()
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

    return controlBuffer.size();
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::parseControlPackets()
{
    while (controlBuffer.size() > 0)
    {
        auto retVal = _appleMIDIParser.parse(controlBuffer, amPortType::Control);
        if (retVal == parserReturn::Processed 
        ||  retVal == parserReturn::NotEnoughData
        ||  retVal == parserReturn::NotSureGiveMeMoreData)
        {
            break;
        }
        else if (retVal == parserReturn::UnexpectedData)
        {
#ifdef USE_EXT_CALLBACKS
            if (nullptr != _exceptionCallback)
                _exceptionCallback(ssrc, ParseException, 0);
#endif            
            controlBuffer.pop_front();
        }
        else if (retVal == parserReturn::SessionNameVeryLong)
        {
            // purge the rest of the data in controlPort
            while (controlPort.read() >= 0) {}
        }
    }
}

template <class UdpClass, class Settings, class Platform>
size_t AppleMIDISession<UdpClass, Settings, Platform>::readDataPackets()
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

    return dataBuffer.size();
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
        
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, UnexpectedParseException, 0);
#endif
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
    // ignore invitation of a participant already in the participant list
#ifndef ONE_PARTICIPANT
    if (nullptr != getParticipantBySSRC(invitation.ssrc))
#else
    if (participant.ssrc == invitation.ssrc)
#endif
        return;

#ifndef ONE_PARTICIPANT
    if (participants.full())
#else
    if (participant.ssrc != 0)
#endif
    {
        writeInvitation(controlPort, controlPort.remoteIP(), controlPort.remotePort(), invitation, amInvitationRejected);     
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, TooManyParticipantsException, 0);
#endif
        return;
    }

#ifndef ONE_PARTICIPANT
    Participant<Settings> participant;
#endif
    participant.kind = Listener;
    participant.ssrc = invitation.ssrc;
    participant.remoteIP   = controlPort.remoteIP();
    participant.remotePort = controlPort.remotePort();
    participant.lastSyncExchangeTime = now;
#ifdef KEEP_SESSION_NAME
    strncpy(participant.sessionName, invitation.sessionName, Settings::MaxSessionNameLen);
#endif

#ifdef KEEP_SESSION_NAME
    // Re-use the invitation for acceptance. Overwrite sessionName with ours
    strncpy(invitation.sessionName, localName, Settings::MaxSessionNameLen);
    invitation.sessionName[Settings::MaxSessionNameLen] = '\0';
#endif

#ifdef USE_DIRECTORY
    switch (whoCanConnectToMe) {
    case None:
        writeInvitation(controlPort, controlPort.remoteIP(), controlPort.remotePort(), invitation, amInvitationRejected);
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, NotAcceptingAnyone, 0);
#endif
        return;
    case OnlyComputersInMyDirectory:
        if (!IsComputerInDirectory(controlPort.remoteIP())) {
            writeInvitation(controlPort, controlPort.remoteIP(), controlPort.remotePort(), invitation, amInvitationRejected);
#ifdef USE_EXT_CALLBACKS
            if (nullptr != _exceptionCallback)
                _exceptionCallback(ssrc, ComputerNotInDirectory, 0);
#endif
            return;
        }
    case Anyone:
        break;
    }
#endif
           
#ifndef ONE_PARTICIPANT
    participants.push_back(participant);
#endif

    writeInvitation(controlPort, participant.remoteIP, participant.remotePort, invitation, amInvitationAccepted);
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedDataInvitation(AppleMIDI_Invitation &invitation)
{
#ifndef ONE_PARTICIPANT
    auto pParticipant = getParticipantBySSRC(invitation.ssrc);
#else
    auto pParticipant = (participant.ssrc == invitation.ssrc) ? &participant : nullptr;
#endif
    if (nullptr == pParticipant)
    {
        writeInvitation(dataPort, dataPort.remoteIP(), dataPort.remotePort(), invitation, amInvitationRejected);

#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, ParticipantNotFoundException, invitation.ssrc);
#endif  
        return;
    }

#ifdef KEEP_SESSION_NAME
    // Re-use the invitation for acceptance. Overwrite sessionName with ours
    strncpy(invitation.sessionName, localName, Settings::MaxSessionNameLen);
    invitation.sessionName[Settings::MaxSessionNameLen] = '\0';
#endif

    // writeInvitation will alter the values of the invitation,
    // in order to safe memory and computing cycles its easier to make a copy
    // of the ssrc here.
    auto ssrc_ = invitation.ssrc;
    
    writeInvitation(dataPort, pParticipant->remoteIP, pParticipant->remotePort + 1, invitation, amInvitationAccepted);

    pParticipant->kind = Listener;
    
    // Inform that we have an established connection
    if (nullptr != _connectedCallback)
#ifdef KEEP_SESSION_NAME
        _connectedCallback(ssrc_, pParticipant->sessionName);
#else
        _connectedCallback(ssrc_, nullptr);
#endif
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedBitrateReceiveLimit(AppleMIDI_BitrateReceiveLimit &)
{
}

#ifdef APPLEMIDI_INITIATOR
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
#ifndef ONE_PARTICIPANT
    auto pParticipant = this->getParticipantByInitiatorToken(invitationAccepted.initiatorToken);
#else
    auto pParticipant = (participant.initiatorToken == invitationAccepted.initiatorToken) ? &participant : nullptr;
#endif
    if (nullptr == pParticipant)
    {
        return;
    }
    
    pParticipant->ssrc               = invitationAccepted.ssrc;
    pParticipant->lastInviteSentTime = now - 1000; // forces invite to be send
    pParticipant->connectionAttempts = 0; // reset back to 0
    pParticipant->invitationStatus   = ControlInvitationAccepted; // step it up
#ifdef KEEP_SESSION_NAME
    strncpy(pParticipant->sessionName, invitationAccepted.sessionName, Settings::MaxSessionNameLen);
    pParticipant->sessionName[Settings::MaxSessionNameLen] = '\0';
#endif
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedDataInvitationAccepted(AppleMIDI_InvitationAccepted_t &invitationAccepted)
{
#ifndef ONE_PARTICIPANT
    auto pParticipant = this->getParticipantByInitiatorToken(invitationAccepted.initiatorToken);
#else
    auto pParticipant = (participant.initiatorToken == invitationAccepted.initiatorToken) ? &participant : nullptr;
#endif
    if (nullptr == pParticipant)
    {
        return;
    }
    
    pParticipant->invitationStatus = DataInvitationAccepted;
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedInvitationRejected(AppleMIDI_InvitationRejected_t & invitationRejected)
{
    for (auto i = 0; i < participants.size(); i++)
    {
        if (invitationRejected.ssrc == participants[i].ssrc)
        {
#ifndef ONE_PARTICIPANT
            participants.erase(i);
#else
            participant.ssrc = 0;
#endif
            return;
        }
    }
}
#endif

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
#ifndef ONE_PARTICIPANT
    auto pParticipant = getParticipantBySSRC(synchronization.ssrc);
#else
    auto pParticipant = (participant.ssrc == synchronization.ssrc) ? &participant : nullptr;
#endif
    if (nullptr == pParticipant)
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, ParticipantNotFoundException, synchronization.ssrc);
#endif  

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
        writeSynchronization(pParticipant->remoteIP, pParticipant->remotePort + 1, synchronization);
        break;
    case SYNC_CK1: /* From session LISTENER */
#ifdef APPLEMIDI_INITIATOR
        synchronization.timestamps[SYNC_CK2] = rtpMidiClock.Now();
        synchronization.count = SYNC_CK2;
        writeSynchronization(pParticipant->remoteIP, pParticipant->remotePort + 1, synchronization);
        pParticipant->synchronizing = false;
#endif
        break;
    case SYNC_CK2: /* From session APPLEMIDI_INITIATOR */
            
#ifdef USE_EXT_CALLBACKS
        // each party can estimate the offset between the two clocks using the following formula
        pParticipant->offsetEstimate = (uint32_t)(((synchronization.timestamps[2] + synchronization.timestamps[0]) / 2) - synchronization.timestamps[1]);
#endif
        break;
    }

    // All particpants need to check in regularly,
    // failing to do so will result in a lost connection.
    pParticipant->lastSyncExchangeTime = now;
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
    // We do not keep any recovery journals, no command history, nothing! 
    // Here is where you would correct if packets are dropped (send them again)
#ifndef ONE_PARTICIPANT
    auto pParticipant = getParticipantBySSRC(receiverFeedback.ssrc);
#else
    auto pParticipant = (participant.ssrc == receiverFeedback.ssrc) ? &participant : nullptr;
#endif
    if (nullptr == pParticipant) {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, ParticipantNotFoundException, receiverFeedback.ssrc);
#endif  
        return;
    }

    if (pParticipant->sendSequenceNr < receiverFeedback.sequenceNr)
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(pParticipant->ssrc, SendPacketsDropped, pParticipant->sendSequenceNr - receiverFeedback.sequenceNr);
#endif
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedEndSession(AppleMIDI_EndSession_t &endSession)
{
#ifndef ONE_PARTICIPANT
    for (size_t i = 0; i < participants.size(); i++)
    {
        auto participant = participants[i];
#else
    {
#endif
        if (endSession.ssrc == participant.ssrc)
        {
            auto ssrc = participant.ssrc;

#ifndef ONE_PARTICIPANT
            participants.erase(i);
#else
            participant.ssrc = 0;
#endif            
            if (nullptr != _disconnectedCallback)
                _disconnectedCallback(ssrc);

            return;
        }
    }
}

#ifdef USE_DIRECTORY
template <class UdpClass, class Settings, class Platform>
bool AppleMIDISession<UdpClass, Settings, Platform>::IsComputerInDirectory(IPAddress remoteIP) const
{
    for (size_t i = 0; i < directory.size(); i++)
        if (remoteIP == directory[i])
            return true;
    return false;
}
#endif

#ifndef ONE_PARTICIPANT
template <class UdpClass, class Settings, class Platform>
Participant<Settings>* AppleMIDISession<UdpClass, Settings, Platform>::getParticipantBySSRC(const ssrc_t& ssrc)
{
    for (size_t i = 0; i < participants.size(); i++)
        if (ssrc == participants[i].ssrc)
            return &participants[i];
    return nullptr;
}

template <class UdpClass, class Settings, class Platform>
Participant<Settings>* AppleMIDISession<UdpClass, Settings, Platform>::getParticipantByInitiatorToken(const uint32_t& initiatorToken)
{
    for (auto i = 0; i < participants.size(); i++)
        if (initiatorToken == participants[i].initiatorToken)
            return &participants[i];
    return nullptr;
}
#endif

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeInvitation(UdpClass &port, const IPAddress& remoteIP, const uint16_t& remotePort, AppleMIDI_Invitation_t & invitation, const byte *command)
{
    if (!port.beginPacket(remoteIP, remotePort))
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, UdpBeginPacketFailed, 1);
#endif
        return;
    }

    port.write((uint8_t *)amSignature, sizeof(amSignature));
    
        port.write((uint8_t *)command, sizeof(amInvitation));
        port.write((uint8_t *)amProtocolVersion, sizeof(amProtocolVersion));
        invitation.initiatorToken = __htonl(invitation.initiatorToken);
        invitation.ssrc = ssrc;
        invitation.ssrc = __htonl(invitation.ssrc);
        port.write(reinterpret_cast<uint8_t *>(&invitation), invitation.getLength());
    
    port.endPacket();
    port.flush();
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeReceiverFeedback(const IPAddress& remoteIP, const uint16_t & remotePort, AppleMIDI_ReceiverFeedback_t & receiverFeedback)
{
    if (!controlPort.beginPacket(remoteIP, remotePort))
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, UdpBeginPacketFailed, 2);
#endif
        return;
    }

    controlPort.write((uint8_t *)amSignature, sizeof(amSignature));
    
        controlPort.write((uint8_t *)amReceiverFeedback, sizeof(amReceiverFeedback));
    
        receiverFeedback.ssrc       = __htonl(receiverFeedback.ssrc);
        receiverFeedback.sequenceNr = __htons(receiverFeedback.sequenceNr);
    
        controlPort.write(reinterpret_cast<uint8_t *>(&receiverFeedback), sizeof(AppleMIDI_ReceiverFeedback));
    
    controlPort.endPacket();
    controlPort.flush();
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeSynchronization(const IPAddress& remoteIP, const uint16_t & remotePort, AppleMIDI_Synchronization_t &synchronization)
{
    if (!dataPort.beginPacket(remoteIP, remotePort))
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, UdpBeginPacketFailed, 3);
#endif
        return;
    }

    dataPort.write((uint8_t *)amSignature, sizeof(amSignature));
    dataPort.write((uint8_t *)amSynchronization, sizeof(amSynchronization));
    synchronization.ssrc = ssrc;
    synchronization.ssrc = __htonl(synchronization.ssrc);

    synchronization.timestamps[0] = __htonll(synchronization.timestamps[0]);
    synchronization.timestamps[1] = __htonll(synchronization.timestamps[1]);
    synchronization.timestamps[2] = __htonll(synchronization.timestamps[2]);
    dataPort.write(reinterpret_cast<uint8_t *>(&synchronization), sizeof(synchronization));
    
    dataPort.endPacket();
    dataPort.flush();
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeEndSession(const IPAddress& remoteIP, const uint16_t & remotePort, AppleMIDI_EndSession_t &endSession)
{
    if (!controlPort.beginPacket(remoteIP, remotePort))
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, UdpBeginPacketFailed, 4);
#endif
        return;
    }

    controlPort.write((uint8_t *)amSignature, sizeof(amSignature));
    controlPort.write((uint8_t *)amEndSession, sizeof(amEndSession));
    controlPort.write((uint8_t *)amProtocolVersion, sizeof(amProtocolVersion));

    endSession.initiatorToken = __htonl(endSession.initiatorToken);
    endSession.ssrc           = __htonl(endSession.ssrc);

    controlPort.write(reinterpret_cast<uint8_t *>(&endSession), sizeof(endSession));
    
    controlPort.endPacket();
    controlPort.flush();
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeRtpMidiToAllParticipants()
{
#ifndef ONE_PARTICIPANT
    for (size_t i = 0; i < participants.size(); i++)
    {
        auto pParticipant = &participants[i];
        
        writeRtpMidiBuffer(pParticipant);
    }
#else
    writeRtpMidiBuffer(&participant);
#endif
    outMidiBuffer.clear();
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeRtpMidiBuffer(Participant<Settings>* participant)
{ 
    const auto bufferLen = outMidiBuffer.size();

    Rtp rtp;

    // First octet
    rtp.vpxcc = ((RTP_VERSION_2) << 6); // RTP version 2
    rtp.vpxcc &= ~RTP_P_FIELD; // no padding
    rtp.vpxcc &= ~RTP_X_FIELD; // no extension
    // No CSRC

    // second octet
    rtp.mpayload = PAYLOADTYPE_RTPMIDI; 

/*
    // The behavior of the 1-bit M field depends on the media type of the
    // stream.  For native streams, the M bit MUST be set to 1 if the MIDI
    // command section has a non-zero LEN field and MUST be set to 0
    // otherwise.  For mpeg4-generic streams, the M bit MUST be set to 1 for
    // all packets (to conform to [RFC3640]).
    if (bufferLen != 0)
        rtp.mpayload |= RTP_M_FIELD;
    else
        rtp.mpayload &= ~RTP_M_FIELD;
*/
    // Both https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html
    // and https://tools.ietf.org/html/rfc6295#section-2.1 indicate that the M field needs to be set
    // if the len in the MIDI section is NON-ZERO.
    // However, doing so on, MacOS does not take the given MIDI commands
    // Clear the M field
    rtp.mpayload &= ~RTP_M_FIELD;

    rtp.ssrc = ssrc;
    
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
    rtp.timestamp = (Settings::TimestampRtpPackets) ? rtpMidiClock.Now() : 0;
 
    // increment the sequenceNr
    participant->sendSequenceNr++;

    rtp.sequenceNr = participant->sendSequenceNr;

#ifdef USE_EXT_CALLBACKS
    if (_sentRtpCallback)
        _sentRtpCallback(rtp);
#endif

    rtp.timestamp  = __htonl(rtp.timestamp);
    rtp.ssrc       = __htonl(rtp.ssrc);
    rtp.sequenceNr = __htons(rtp.sequenceNr);

    if (!dataPort.beginPacket(participant->remoteIP, participant->remotePort + 1))
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, UdpBeginPacketFailed, 5);
#endif
        return;
    }

    // write rtp header
    dataPort.write((uint8_t *)&rtp, sizeof(rtp));

    // Write rtpMIDI section
    RtpMIDI_t rtpMidi;

    //   0                   1                   2                   3
    //   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //  |B|J|Z|P|LEN... |  MIDI list ...                                |
    //  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    rtpMidi.flags = 0;
    rtpMidi.flags &= ~RTP_MIDI_CS_FLAG_J; // no journal, clear J-FLAG
    rtpMidi.flags &= ~RTP_MIDI_CS_FLAG_Z; // no Delta Time 0 field, clear Z flag
    rtpMidi.flags &= ~RTP_MIDI_CS_FLAG_P; // no phantom flag

    if (bufferLen <= 0x0F)
    { // Short header
        rtpMidi.flags |= (uint8_t)bufferLen;
        rtpMidi.flags &= ~RTP_MIDI_CS_FLAG_B; // short header, clear B-FLAG
        dataPort.write(rtpMidi.flags);
    }
    else
    { // Long header
        rtpMidi.flags |= (uint8_t)(bufferLen >> 8);
        rtpMidi.flags |=  RTP_MIDI_CS_FLAG_B; // set B-FLAG for long header
        dataPort.write(rtpMidi.flags);
        dataPort.write((uint8_t)(bufferLen));
    }

    // write out the MIDI Section
    for (size_t i = 0; i < bufferLen; i++)
        dataPort.write(outMidiBuffer[i]);

    // *No* journal section (Not supported)

    dataPort.endPacket();
    dataPort.flush();

#ifdef USE_EXT_CALLBACKS
    if (_sentRtpMidiCallback)
        _sentRtpMidiCallback(rtpMidi);
#endif
}

//
//
//
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSynchronization()
{
#ifndef ONE_PARTICIPANT
    for (size_t i = 0; i < participants.size(); i++)
#endif
    {
#ifndef ONE_PARTICIPANT
        auto pParticipant = &participants[i];
        if (pParticipant->ssrc == 0) continue;
#else
        auto pParticipant = &participant;
        if (pParticipant->ssrc == 0) return;
#endif
#ifdef APPLEMIDI_INITIATOR
        if (pParticipant->invitationStatus != Connected)
            continue;
        
        // Only for Initiators that are Connected
        if (pParticipant->kind == Listener)
        {
#endif
            // The initiator must check in with the listener at least once every 60 seconds;
            // otherwise the responder may assume that the initiator has died and terminate the session.
            if (now - pParticipant->lastSyncExchangeTime > Settings::CK_MaxTimeOut)
            {
#ifdef USE_EXT_CALLBACKS
                if (nullptr != _exceptionCallback)
                    _exceptionCallback(ssrc, ListenerTimeOutException, 0);
#endif
                sendEndSession(pParticipant);
#ifndef ONE_PARTICIPANT
                participants.erase(i);
#else
                participant.ssrc = 0;
#endif  
            }
#ifdef APPLEMIDI_INITIATOR
        }
        else
        {
            (pParticipant->synchronizing) ? manageSynchronizationInitiatorInvites(i)
                                          : manageSynchronizationInitiatorHeartBeat(pParticipant);
        }
#endif
    }
}

#ifdef APPLEMIDI_INITIATOR

//
// The initiator of the session polls if remote station is still alive.
// (Initiators only)
//
// The initiator must initiate a new sync exchange at least once every 60 seconds;
// otherwise the responder may assume that the initiator has died and terminate the session.
//
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSynchronizationInitiatorHeartBeat(Participant<Settings>* pParticipant)
{
    // Note: During startup, the initiator should send synchronization exchanges more frequently;
    // empirical testing has determined that sending a few exchanges improves clock
    // synchronization accuracy.
    // (Here: twice every 0.5 seconds, then 6 times every 1.5 seconds, then every 10 seconds.)
    bool doSyncronize = false;
    if (pParticipant->synchronizationHeartBeats < 2)
    {
       if (now - pParticipant->lastInviteSentTime >  500) // 2 x every 0.5 seconds
       {
           pParticipant->synchronizationHeartBeats++;
           doSyncronize = true;
       }
    }
    else if (pParticipant->synchronizationHeartBeats < 7)
    {
       if (now - pParticipant->lastInviteSentTime >  1500) // 5 x every 1.5 seconds
       {
           pParticipant->synchronizationHeartBeats++;
           doSyncronize = true;
       }
    }
    else if (now - pParticipant->lastInviteSentTime >  Settings::SynchronizationHeartBeat)
    {
       doSyncronize = true;
    }

    if (!doSyncronize)
       return;

    pParticipant->synchronizationCount = 0;
    sendSynchronization(pParticipant);
}

// checks for
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSynchronizationInitiatorInvites(size_t i)
{
    auto pParticipant = &participants[i];

    if (now - pParticipant->lastInviteSentTime >  10000)
    {
        if (pParticipant->synchronizationCount > Settings::MaxSynchronizationCK0Attempts)
        {
#ifdef USE_EXT_CALLBACKS
            if (nullptr != _exceptionCallback)
                _exceptionCallback(ssrc, MaxAttemptsException, 0);
#endif
            // After too many attempts, stop.
            sendEndSession(pParticipant);

#ifndef ONE_PARTICIPANT
            participants.erase(i);
#else
            participant.ssrc = 0;
#endif
            return;
        }
        sendSynchronization(pParticipant);
    }
}

#endif

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
#ifndef ONE_PARTICIPANT
    for (auto i = 0; i < participants.size(); i++)
#endif
    {
#ifndef ONE_PARTICIPANT
        auto pParticipant = &participants[i];
#else
        auto pParticipant = &participant;
#endif

        if (pParticipant->kind == Listener)
#ifndef ONE_PARTICIPANT
            continue;
#else
            return;
#endif
        if (pParticipant->invitationStatus == DataInvitationAccepted)
        {
            // Inform that we have an established connection
            if (nullptr != _connectedCallback)
#ifdef KEEP_SESSION_NAME
                _connectedCallback(pParticipant->ssrc, pParticipant->sessionName);
#else
                _connectedCallback(pParticipant->ssrc, nullptr);
#endif
            pParticipant->invitationStatus = Connected;
        }

        if (pParticipant->invitationStatus == Connected)
#ifndef ONE_PARTICIPANT
            continue;
#else
            return;
#endif

        // try to connect every 1 second (1000 ms)
        if (now - pParticipant->lastInviteSentTime >  1000)
        {
            if (pParticipant->connectionAttempts >= Settings::MaxSessionInvitesAttempts)
            {
#ifdef USE_EXT_CALLBACKS
                if (nullptr != _exceptionCallback)
                    _exceptionCallback(ssrc, NoResponseFromConnectionRequestException, 0);
#endif
                // After too many attempts, stop.
                sendEndSession(pParticipant);
                
#ifndef ONE_PARTICIPANT
                participants.erase(i);
#else
                participant.ssrc = 0;
#endif
#ifndef ONE_PARTICIPANT
                continue;
#else
                return;
#endif
            }

            pParticipant->lastInviteSentTime = now;
            pParticipant->connectionAttempts++;

            AppleMIDI_Invitation invitation;
            invitation.ssrc = this->ssrc;
            invitation.initiatorToken = pParticipant->initiatorToken;
#ifdef KEEP_SESSION_NAME
            strncpy(invitation.sessionName, this->localName, Settings::MaxSessionNameLen);
            invitation.sessionName[Settings::MaxSessionNameLen] = '\0';
#endif
            if (pParticipant->invitationStatus == Initiating
            ||  pParticipant->invitationStatus == AwaitingControlInvitationAccepted)
            {
                writeInvitation(controlPort, pParticipant->remoteIP, pParticipant->remotePort, invitation, amInvitation);
                pParticipant->invitationStatus = AwaitingControlInvitationAccepted;
            }
            else
            if (pParticipant->invitationStatus == ControlInvitationAccepted
            ||  pParticipant->invitationStatus == AwaitingDataInvitationAccepted)
            {
                writeInvitation(dataPort, pParticipant->remoteIP, pParticipant->remotePort + 1, invitation, amInvitation);
                pParticipant->invitationStatus = AwaitingDataInvitationAccepted;
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
#ifndef ONE_PARTICIPANT
    for (uint8_t i = 0; i < participants.size(); i++)
#endif
    {
#ifndef ONE_PARTICIPANT
        auto pParticipant = &participants[i];
        if (pParticipant->ssrc == 0) continue;
#else
        auto pParticipant = &participant;
        if (pParticipant->ssrc == 0) return;
#endif
       
        if (pParticipant->doReceiverFeedback == false)
#ifndef ONE_PARTICIPANT
            continue;
#else
            return;
#endif

        if ((now - pParticipant->receiverFeedbackStartTime) > Settings::ReceiversFeedbackThreshold)
        {
            AppleMIDI_ReceiverFeedback_t rf;
            rf.ssrc       = ssrc;
            rf.sequenceNr = pParticipant->receiveSequenceNr;
            writeReceiverFeedback(pParticipant->remoteIP, pParticipant->remotePort, rf);

            // reset the clock. It is started when we receive MIDI
            pParticipant->doReceiverFeedback = false;
        }
    }
}

#ifdef APPLEMIDI_INITIATOR

template <class UdpClass, class Settings, class Platform>
bool AppleMIDISession<UdpClass, Settings, Platform>::sendInvite(IPAddress ip, uint16_t port)
{
#ifndef ONE_PARTICIPANT
    if (participants.full())
#else
    if (participant.ssrc != 0)
#endif
    {
        return false;
    }

#ifndef ONE_PARTICIPANT
    Participant<Settings> participant;
#endif
    participant.kind = Initiator;
    participant.remoteIP = ip;
    participant.remotePort = port;
    participant.lastInviteSentTime = now - 1000; // forces invite to be send immediately
    participant.lastSyncExchangeTime = now;
    participant.initiatorToken = random(1, INT32_MAX) * 2;

#ifndef ONE_PARTICIPANT
    participants.push_back(participant);
#endif

    return true;
}

#endif

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::sendEndSession()
{
#ifndef ONE_PARTICIPANT
    while (participants.size() > 0)
    {
        auto participant = &participants.front();
        sendEndSession(participant);

        participants.pop_front();
    }
#else
    if (participant.src != 0)
    {
        sendEndSession(&participant);
        participant.ssrc = 0;
    }
#endif
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::sendEndSession(Participant<Settings>* participant)
{
    AppleMIDI_EndSession_t endSession;
    endSession.initiatorToken = 0;
    endSession.ssrc = this->ssrc;
    writeEndSession(participant->remoteIP, participant->remotePort, endSession);
    
    if (nullptr != _disconnectedCallback)
        _disconnectedCallback(participant->ssrc);
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedRtp(const Rtp_t& rtp)
{
#ifndef ONE_PARTICIPANT
    auto pParticipant = getParticipantBySSRC(rtp.ssrc);
#else
    auto pParticipant = (participant.ssrc == rtp.ssrc) ? &participant : nullptr;
#endif
    
    if (nullptr != pParticipant)
    {
        if (pParticipant->doReceiverFeedback == false)
            pParticipant->receiverFeedbackStartTime = now;
        pParticipant->doReceiverFeedback = true;

#ifdef USE_EXT_CALLBACKS
        auto offset = (rtp.timestamp - pParticipant->offsetEstimate);
        auto latency = (int32_t)(rtpMidiClock.Now() - offset);

        if (pParticipant->firstMessageReceived == true)
            // avoids first message to generate sequence exception
            // as we do not know the last sequenceNr received.
            pParticipant->firstMessageReceived = false;
        else if (rtp.sequenceNr - pParticipant->receiveSequenceNr - 1 != 0) {
            if (nullptr != _exceptionCallback)
                _exceptionCallback(ssrc, ReceivedPacketsDropped, rtp.sequenceNr - pParticipant->receiveSequenceNr - 1);
        }

        if (nullptr != _receivedRtpCallback)
            _receivedRtpCallback(pParticipant->ssrc, rtp, latency);
#endif

        pParticipant->receiveSequenceNr = rtp.sequenceNr;
    }
    else
    {
        // TODO??? re-connect?
    }
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::StartReceivedMidi()
{
#ifdef USE_EXT_CALLBACKS
   if (nullptr != _startReceivedMidiByteCallback)
        _startReceivedMidiByteCallback(ssrc);
#endif
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedMidi(byte value)
{
#ifdef USE_EXT_CALLBACKS
    if (nullptr != _receivedMidiByteCallback)
        _receivedMidiByteCallback(ssrc, value);
#endif

    inMidiBuffer.push_back(value);
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::EndReceivedMidi()
{
#ifdef USE_EXT_CALLBACKS
    if (nullptr != _endReceivedMidiByteCallback)
        _endReceivedMidiByteCallback(ssrc);
#endif
}

END_APPLEMIDI_NAMESPACE

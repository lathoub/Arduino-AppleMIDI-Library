#pragma once

#include "AppleMIDI_Namespace.h"
#include <string.h>

BEGIN_APPLEMIDI_NAMESPACE

// Read one UDP datagram into buffer without concatenating the next packet.
template <class UdpClass, class Settings, class Platform>
size_t AppleMIDISession<UdpClass, Settings, Platform>::readUdpDatagram(UdpClass &port, RtpBuffer_t &buffer)
{
    size_t remaining = port.available(); // unread tail of the current datagram

    if (remaining == 0)
    {
        // Current datagram is fully pulled from the socket.
        // Do not start the next one while leftover bytes are still being parsed.
        if (!buffer.empty())
            return buffer.size();

        remaining = port.parsePacket();
    }

    while (remaining > 0 && !buffer.full())
    {
        auto bytesToRead = min(min(remaining, buffer.free()), sizeof(packetBuffer));
        auto bytesRead = port.read(packetBuffer, bytesToRead);
        remaining -= bytesRead;
        buffer.push_back(packetBuffer, bytesRead);
    }

    return buffer.size();
}

template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::drainUdpRemainder(UdpClass &port)
{
    while (port.available() > 0 && port.read() >= 0) {}
}

// Read pending control UDP packets into the control buffer.
template <class UdpClass, class Settings, class Platform>
size_t AppleMIDISession<UdpClass, Settings, Platform>::readControlPackets()
{
    return readUdpDatagram(controlPort, controlBuffer);
}

// Parse buffered control packets and handle errors.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::parseControlPackets()
{
    while (controlBuffer.size() > 0)
    {
        auto retVal = _appleMIDIParser.parse(controlBuffer, amPortType::Control);
        if (retVal == parserReturn::Processed)
        {
            if (controlBuffer.empty())
                drainUdpRemainder(controlPort);
            break;
        }
        if (retVal == parserReturn::NotEnoughData
         || retVal == parserReturn::NotSureGiveMeMoreData)
        {
            if (controlPort.available() == 0)
                controlBuffer.clear(); // truncated/incomplete datagram
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
            drainUdpRemainder(controlPort);
        }
    }
}

// Read pending data UDP packets into the data buffer.
template <class UdpClass, class Settings, class Platform>
size_t AppleMIDISession<UdpClass, Settings, Platform>::readDataPackets()
{
    return readUdpDatagram(dataPort, dataBuffer);
}

// Parse buffered data packets using RTP-MIDI and AppleMIDI parsers.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::parseDataPackets()
{
    while (dataBuffer.size() > 0)
    {
        auto retVal1 = _rtpMIDIParser.parse(dataBuffer);
        if (retVal1 == parserReturn::Processed)
        {
            if (dataBuffer.empty())
                drainUdpRemainder(dataPort);
            break;
        }
        if (retVal1 == parserReturn::NotEnoughData)
        {
            if (dataPort.available() == 0 && !_rtpMIDIParser.isMidMessage())
                dataBuffer.clear();
            break;
        }

        auto retVal2 = _appleMIDIParser.parse(dataBuffer, amPortType::Data);
        if (retVal2 == parserReturn::Processed)
        {
            if (dataBuffer.empty())
                drainUdpRemainder(dataPort);
            break;
        }
        if (retVal2 == parserReturn::NotEnoughData)
        {
            if (dataPort.available() == 0)
                dataBuffer.clear();
            break;
        }

        if (retVal1 == parserReturn::NotSureGiveMeMoreData
         || retVal2 == parserReturn::NotSureGiveMeMoreData)
        {
            if (dataPort.available() == 0 && !_rtpMIDIParser.isMidMessage())
                dataBuffer.clear();
            break;
        }

#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, UnexpectedParseException, 0);
#endif
         dataBuffer.pop_front();
    }
}

// Route an invitation based on the incoming port type.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedInvitation(AppleMIDI_Invitation_t &invitation, const amPortType &portType)
{
   if (portType == amPortType::Control)
        ReceivedControlInvitation(invitation);
    else
        ReceivedDataInvitation(invitation);
}

// Handle an incoming control invitation from a remote participant.
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
    participant.sendSequenceNr = random(1, UINT16_MAX); // http://www.rfc-editor.org/rfc/rfc6295.txt , 2.1.  RTP Header
    participant.remoteIP   = controlPort.remoteIP();
    participant.remotePort = controlPort.remotePort();
    participant.remoteDataPort = (uint16_t)(controlPort.remotePort() + 1);
    participant.lastSyncExchangeTime = now;
#ifdef KEEP_SESSION_NAME
    strncpy(participant.sessionName, invitation.sessionName, Settings::MaxSessionNameLen);
    participant.sessionName[Settings::MaxSessionNameLen] = '\0';
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

// Handle an incoming data invitation for an existing participant.
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
    
    pParticipant->remoteDataPort = dataPort.remotePort();

    writeInvitation(dataPort, dataPort.remoteIP(), pParticipant->remoteDataPort, invitation, amInvitationAccepted);

    pParticipant->kind = Listener;
    
    // Inform that we have an established connection
    if (nullptr != _connectedCallback)
    {
#ifdef KEEP_SESSION_NAME
        _connectedCallback(ssrc_, pParticipant->sessionName);
#else
        _connectedCallback(ssrc_, nullptr);
#endif
    }
}

// Placeholder for bitrate receive limit messages (not used).
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedBitrateReceiveLimit(AppleMIDI_BitrateReceiveLimit &)
{
}

#ifdef APPLEMIDI_INITIATOR
// Route accepted invitations based on the incoming port type.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedInvitationAccepted(AppleMIDI_InvitationAccepted_t &invitationAccepted, const amPortType &portType)
{
    if (portType == amPortType::Control)
        ReceivedControlInvitationAccepted(invitationAccepted);
    else
        ReceivedDataInvitationAccepted(invitationAccepted);
}

// Update participant state after control invitation acceptance.
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

// Update participant state after data invitation acceptance.
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
    pParticipant->remoteDataPort = dataPort.remotePort();
}

// Remove participant on invitation rejection.
// Match by initiator token, same as OK: the peer SSRC is unknown until control OK.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedInvitationRejected(AppleMIDI_InvitationRejected_t & invitationRejected)
{
#ifndef ONE_PARTICIPANT
    for (size_t i = 0; i < participants.size(); i++)
    {
        if (invitationRejected.initiatorToken == participants[i].initiatorToken)
        {
            participants.erase(i);
            return;
        }
    }
#else
    if (invitationRejected.initiatorToken != participant.initiatorToken)
        return;
    participant.ssrc = 0;
    participant.kind = Listener;
    participant.initiatorToken = 0;
#endif
}
#endif

// Handle an incoming synchronization exchange packet.
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
        writeSynchronization(pParticipant->remoteIP, pParticipant->remoteDataPort, synchronization);
        break;
    case SYNC_CK1: /* From session LISTENER */
#ifdef APPLEMIDI_INITIATOR
        synchronization.timestamps[SYNC_CK2] = rtpMidiClock.Now();
        synchronization.count = SYNC_CK2;
        writeSynchronization(pParticipant->remoteIP, pParticipant->remoteDataPort, synchronization);
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
// Process receiver feedback about last received sequence numbers.
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

    int16_t ahead = (int16_t)(receiverFeedback.sequenceNr - pParticipant->sendSequenceNr);
    if (ahead > 0)
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(pParticipant->ssrc, SendPacketsDropped, (int32_t)ahead);
#endif
    }
}

// Handle end-session requests and notify callbacks.
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
// Check whether a remote IP is in the allowed directory list.
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
// Find a participant by SSRC.
template <class UdpClass, class Settings, class Platform>
Participant<Settings>* AppleMIDISession<UdpClass, Settings, Platform>::getParticipantBySSRC(const ssrc_t& ssrc)
{
    for (size_t i = 0; i < participants.size(); i++)
        if (ssrc == participants[i].ssrc)
            return &participants[i];
    return nullptr;
}

// Find a participant by initiator token.
template <class UdpClass, class Settings, class Platform>
Participant<Settings>* AppleMIDISession<UdpClass, Settings, Platform>::getParticipantByInitiatorToken(const uint32_t& initiatorToken)
{
    for (auto i = 0; i < participants.size(); i++)
        if (initiatorToken == participants[i].initiatorToken)
            return &participants[i];
    return nullptr;
}
#endif

// Serialize and send an invitation packet on the given port.
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

#ifndef APPLEMIDI_NO_RECEIVER_FEEDBACK
// Send receiver feedback on the control port.
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
#endif

// Send a synchronization packet on the data port.
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

// Send an end-session packet on the control port.
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

// Flush the outgoing MIDI buffer to all participants.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::writeRtpMidiToAllParticipants()
{
#ifndef ONE_PARTICIPANT
    for (size_t i = 0; i < participants.size(); i++)
    {
        auto pParticipant = &participants[i];
        if (pParticipant->ssrc == 0)
            continue;
        writeRtpMidiBuffer(pParticipant);
    }
#else
    if (participant.ssrc != 0)
        writeRtpMidiBuffer(&participant);
#endif
    outMidiBuffer.clear();
}

// Build and send an RTP-MIDI packet for a participant.
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

    if (!dataPort.beginPacket(participant->remoteIP, participant->remoteDataPort))
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, UdpBeginPacketFailed, 5);
#endif
        return;
    }

    // Write RTP + rtpMIDI without a MaxBufferSize stack copy.
    dataPort.write(reinterpret_cast<uint8_t *>(&rtp), sizeof(rtp));

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
        dataPort.write(&rtpMidi.flags, 1);
    }
    else
    { // Long header
        rtpMidi.flags |= (uint8_t)(bufferLen >> 8);
        rtpMidi.flags |=  RTP_MIDI_CS_FLAG_B; // set B-FLAG for long header
        uint8_t hdr[2] = { rtpMidi.flags, (uint8_t)bufferLen };
        dataPort.write(hdr, 2);
    }

    size_t remaining = bufferLen;
    size_t idx = 0;
    while (remaining > 0)
    {
        size_t n = remaining;
        if (n > sizeof(packetBuffer))
            n = sizeof(packetBuffer);
        for (size_t j = 0; j < n; j++)
            packetBuffer[j] = outMidiBuffer[idx++];
        dataPort.write(packetBuffer, n);
        remaining -= n;
    }

    dataPort.endPacket();
    dataPort.flush();

#ifdef USE_EXT_CALLBACKS
    if (_sentRtpMidiCallback)
        _sentRtpMidiCallback(rtpMidi);
#endif
}

// Manage synchronization state for all active participants.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSynchronization()
{
#ifndef ONE_PARTICIPANT
    for (size_t i = 0; i < participants.size();)
#endif
    {
#ifndef ONE_PARTICIPANT
        auto pParticipant = &participants[i];
        if (pParticipant->ssrc == 0)
        {
            i++;
            continue;
        }
#else
        auto pParticipant = &participant;
        if (pParticipant->ssrc == 0) return;
#endif
#ifdef APPLEMIDI_INITIATOR
        if (pParticipant->invitationStatus != Connected)
        {
#ifndef ONE_PARTICIPANT
            i++;
#endif
            continue;
        }
        
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
                continue;
#else
                participant.ssrc = 0;
#endif  
            }
#ifdef APPLEMIDI_INITIATOR
        }
        else
        {
            if (pParticipant->synchronizing)
            {
                if (manageSynchronizationInitiatorInvites(pParticipant))
#ifndef ONE_PARTICIPANT
                    continue;
#else
                    return;
#endif
            }
            else
            {
                manageSynchronizationInitiatorHeartBeat(pParticipant);
            }
        }
#endif
#ifndef ONE_PARTICIPANT
        i++;
#endif
    }
}

#ifdef APPLEMIDI_INITIATOR

// Initiator heartbeat: schedule periodic synchronization exchanges.
//
// The initiator must initiate a new sync exchange at least once every 60 seconds;
// otherwise the responder may assume that the initiator has died and terminate the session.
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

// Retry sync invitations while establishing synchronization.
template <class UdpClass, class Settings, class Platform>
bool AppleMIDISession<UdpClass, Settings, Platform>::manageSynchronizationInitiatorInvites(Participant<Settings>* pParticipant)
{
    if (now - pParticipant->lastInviteSentTime >  10000)
    {
        if (pParticipant->synchronizationCount > Settings::MaxSynchronizationCK0Attempts)
        {
#ifdef USE_EXT_CALLBACKS
            if (nullptr != _exceptionCallback)
                _exceptionCallback(ssrc, MaxAttemptsException, 0);
#endif
            sendEndSession(pParticipant);

#ifndef ONE_PARTICIPANT
            for (size_t j = 0; j < participants.size(); j++)
            {
                if (&participants[j] == pParticipant)
                {
                    participants.erase(j);
                    break;
                }
            }
#else
            participant.ssrc = 0;
#endif
            return true;
        }
        sendSynchronization(pParticipant);
    }
    return false;
}

#endif

// Send a CK0 synchronization message to a participant.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::sendSynchronization(Participant<Settings>* participant)
{
    AppleMIDI_Synchronization_t synchronization;
    synchronization.timestamps[SYNC_CK0] = rtpMidiClock.Now();
    synchronization.timestamps[SYNC_CK1] = 0;
    synchronization.timestamps[SYNC_CK2] = 0;
    synchronization.count = 0;

    writeSynchronization(participant->remoteIP, participant->remoteDataPort, synchronization);
    participant->synchronizing = true;
    participant->synchronizationCount++;
    participant->lastInviteSentTime = now;
}

// Manage invitation retries for session establishment (initiators only).
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::manageSessionInvites()
{
#ifndef ONE_PARTICIPANT
    for (auto i = 0; i < participants.size();)
#endif
    {
#ifndef ONE_PARTICIPANT
        auto pParticipant = &participants[i];
#else
        auto pParticipant = &participant;
#endif

        if (pParticipant->kind == Listener)
#ifndef ONE_PARTICIPANT
        {
            i++;
            continue;
        }
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
        {
            i++;
            continue;
        }
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
                continue;
#else
                participant.ssrc = 0;
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
                writeInvitation(dataPort, pParticipant->remoteIP, pParticipant->remoteDataPort, invitation, amInvitation);
                pParticipant->invitationStatus = AwaitingDataInvitationAccepted;
            }
        }
#ifndef ONE_PARTICIPANT
        i++;
#endif
    }
}

#ifndef APPLEMIDI_NO_RECEIVER_FEEDBACK
// Periodically emit receiver feedback for active participants.
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
#endif

#ifdef APPLEMIDI_INITIATOR

// Queue a new outgoing invitation for a remote endpoint.
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
    participant.sendSequenceNr = random(1, UINT16_MAX); // http://www.rfc-editor.org/rfc/rfc6295.txt , 2.1.  RTP Header
    participant.remoteIP = ip;
    participant.remotePort = port;
    participant.remoteDataPort = (uint16_t)(port + 1);
    participant.lastInviteSentTime = now - 1000; // forces invite to be send immediately
    participant.lastSyncExchangeTime = now;
    participant.initiatorToken = random(1, INT32_MAX) * 2;

#ifndef ONE_PARTICIPANT
    participants.push_back(participant);
#endif

    return true;
}

#endif

// Send end-session to all participants and clear them.
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
    if (participant.ssrc != 0)
    {
        sendEndSession(&participant);
        participant.ssrc = 0;
    }
#endif
}

// Send end-session to a single participant and notify callbacks.
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

// Handle an incoming RTP header and track latency/sequence.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedRtp(const Rtp_t& rtp)
{
#ifndef ONE_PARTICIPANT
    auto pParticipant = getParticipantBySSRC(rtp.ssrc);
#else
    auto pParticipant = (participant.ssrc == rtp.ssrc) ? &participant : nullptr;
#endif
    
    _acceptIncomingMidi = (pParticipant != nullptr);
    if (!_acceptIncomingMidi)
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, ParticipantNotFoundException, rtp.ssrc);
#endif
        return;
    }

#ifndef APPLEMIDI_NO_RECEIVER_FEEDBACK
    if (pParticipant->doReceiverFeedback == false)
        pParticipant->receiverFeedbackStartTime = now;
    pParticipant->doReceiverFeedback = true;
#endif

#ifdef USE_EXT_CALLBACKS
    auto offset = (rtp.timestamp - pParticipant->offsetEstimate);
    auto latency = (int32_t)(rtpMidiClock.Now() - offset);

    if (pParticipant->firstMessageReceived == true)
        // avoids first message to generate sequence exception
        // as we do not know the last sequenceNr received.
        pParticipant->firstMessageReceived = false;
    else
    {
        // RFC 3550 serial arithmetic: uint16 wrap 65535 -> 0 is delta 1, not loss.
        int16_t delta = (int16_t)(rtp.sequenceNr - pParticipant->receiveSequenceNr);
        if (delta > 1) {
            if (nullptr != _exceptionCallback)
                _exceptionCallback(ssrc, ReceivedPacketsDropped, (int32_t)(delta - 1));
        }
    }

    if (nullptr != _receivedRtpCallback)
        _receivedRtpCallback(pParticipant->ssrc, rtp, latency);
#endif

    pParticipant->receiveSequenceNr = rtp.sequenceNr;
}

// Notify that a MIDI byte stream has started.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::StartReceivedMidi()
{
    if (!_acceptIncomingMidi)
        return;

#ifdef USE_EXT_CALLBACKS
   if (nullptr != _startReceivedMidiByteCallback)
        _startReceivedMidiByteCallback(ssrc);
#endif
}

// Handle a received MIDI byte and buffer it.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::ReceivedMidi(byte value)
{
    if (!_acceptIncomingMidi)
        return;

#ifdef USE_EXT_CALLBACKS
    if (nullptr != _receivedMidiByteCallback)
        _receivedMidiByteCallback(ssrc, value);
#endif

    if (inMidiBuffer.full())
    {
#ifdef USE_EXT_CALLBACKS
        if (nullptr != _exceptionCallback)
            _exceptionCallback(ssrc, BufferFullException, 1);
#endif
        return;
    }

    inMidiBuffer.push_back(value);
}

// Notify that a MIDI byte stream has ended.
template <class UdpClass, class Settings, class Platform>
void AppleMIDISession<UdpClass, Settings, Platform>::EndReceivedMidi()
{
    if (!_acceptIncomingMidi)
        return;

#ifdef USE_EXT_CALLBACKS
    if (nullptr != _endReceivedMidiByteCallback)
        _endReceivedMidiByteCallback(ssrc);
#endif
}

END_APPLEMIDI_NAMESPACE

#pragma once

#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

struct DefaultSettings
{
    static const size_t UdpTxPacketMaxSize = 24;
    
    static const size_t MaxBufferSize = 64;
    
    static const size_t MaxSessionNameLen = 24;

    static const uint8_t MaxNumberOfParticipants = 2;
    
    // The recovery journal mechanism requires that the receiver periodically
    // inform the sender of the sequence number of the most recently received packet.
    // This allows the sender to reduce the size of the recovery journal, to encapsulate
    // only those changes to the MIDI stream state occurring after the specified packet number.
    //
    // Each partner then sends cyclically to the other partner the RS message, indicating
    // the last sequence number received correctly, in other words, without any gap between
    // two sequence numbers. The sender can then free the memory containing old journalling data if necessary.
    static const unsigned long ReceiversFeedbackThreshold = 1000;
    
    // Should not be lower than 11000 (15s)
    // MacOS sends CK messages every 10 seconds
    // rtpMIDI on Windows sends CK messages every x seconds
    static const unsigned long CK_MaxTimeOut = 45000;

    // when set to true, the lower 32-bits of the rtpClock ae send
    // when set to false, 0 will be set for immediate playout.
    static const bool TimestampRtpPackets = true;
    
    static const uint8_t MaxSessionInvitesAttempts = 13;
    
    static const uint8_t MaxSynchronizationCK0Attempts = 5;
    
    static const unsigned long SynchronizationHeartBeat = 10000;
};

enum parserReturn: uint8_t
{
    Processed,
    NotSureGiveMeMoreData,
    NotEnoughData,
    UnexpectedData,
};

END_APPLEMIDI_NAMESPACE

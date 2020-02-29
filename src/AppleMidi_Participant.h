#pragma once

#include "AppleMidi_Defs.h"

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <class Settings>
struct Participant
{
    ParticipantKind kind;
    ssrc_t          ssrc;
    IPAddress       remoteIP;
    uint16_t        remotePort;
    
    unsigned long   receiverFeedbackStartTime;
    bool            doReceiverFeedback = false;
    uint16_t        sequenceNr = random(1, UINT16_MAX); // http://www.rfc-editor.org/rfc/rfc6295.txt , 2.1.  RTP Header
    unsigned long   lastSyncExchangeTime;

#ifdef APPLEMIDI_INITIATOR
    uint8_t         connectionAttempts = 0;
    uint32_t        initiatorToken = 0;
    unsigned long   lastInviteSentTime;
    InviteStatus    invitationStatus = Initiating;
    
    uint8_t         synchronizationHeartBeats = 0;
    uint8_t         synchronizationCount = 0;
    bool            synchronizing = false;
#endif
    
#ifdef KEEP_SESSION_NAME
    char            sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN + 1];
#endif
} ;

END_APPLEMIDI_NAMESPACE

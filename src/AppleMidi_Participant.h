#pragma once

#include "AppleMidi_Defs.h"
#include "AppleMidi_Settings.h"

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <class Settings>
struct Participant
{
    ParticipantKind kind;
    ssrc_t          ssrc;
    unsigned long   receiverFeedbackStartTime;
    uint16_t        sequenceNr;

    IPAddress       remoteIP;
    uint16_t        remotePort;

    uint8_t         connectionAttempts;
    unsigned long   lastInviteSentTime;
    InviteStatus    status;
    uint32_t        initiatorToken;
    uint8_t         syncronizationCount;
    bool            doSynchronization;

#ifdef KEEP_SESSION_NAME
    char            sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN + 1];
#endif
} ;

END_APPLEMIDI_NAMESPACE

#pragma once

#include "AppleMidi_Defs.h"
#include "AppleMidi_Settings.h"

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <class Settings>
class Participant
{
public:
    ssrc_t ssrc;
    char sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN + 1];
    unsigned long receiverFeedbackStartTime;
    uint16_t sequenceNr;
};

END_APPLEMIDI_NAMESPACE

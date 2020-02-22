#pragma once

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

struct DefaultSettings
{
    static const size_t MaxBufferSize = 64;
    
    static const size_t MaxSessionNameLen = 24; // todo: not used for the moment - because of use in _Defs

    static const uint8_t MaxNumberOfParticipants = 1;
};

enum parserReturn: uint8_t
{
    Processed,
    NotSureGiveMeMoreData,
    NotEnoughData,
    UnexpectedData,
};

END_APPLEMIDI_NAMESPACE

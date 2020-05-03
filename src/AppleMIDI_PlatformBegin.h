#pragma once

#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

#ifdef WIN32
#pragma pack(push, 1)
#define PACKED
#elif ARDUINO
#define PACKED __attribute__ ((packed))
#elif __APPLE__
#define PACKED
#else
#define PACKED
#endif

struct DefaultPlatform
{
};

END_APPLEMIDI_NAMESPACE

#pragma once

#include "AppleMIDI_Namespace.h"

#include "utility/endian.h"

BEGIN_APPLEMIDI_NAMESPACE

#ifdef _MSC_VER
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

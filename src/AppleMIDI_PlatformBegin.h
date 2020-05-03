#pragma once

#include "AppleMIDI_Namespace.h"

#include "utility/endian.h"

BEGIN_APPLEMIDI_NAMESPACE

#ifdef _MSC_VER
#pragma pack(push, 1)
#define PACKED
#else
#define PACKED __attribute__((__packed__))
#endif

struct DefaultPlatform
{
};

END_APPLEMIDI_NAMESPACE

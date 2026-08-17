#include "AppleMIDI.h"

BEGIN_APPLEMIDI_NAMESPACE

static uint32_t millisLow32 = 0;
static uint32_t millisHigh32 = 0;
uint64_t now = 0;

void refreshNow()
{
    uint32_t low = millis();
    if (low < millisLow32)
        millisHigh32++;
    millisLow32 = low;
    now = ((uint64_t)millisHigh32 << 32) | (uint64_t)low;
}

END_APPLEMIDI_NAMESPACE

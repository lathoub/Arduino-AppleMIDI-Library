/**
 * @file ump_Convert.cpp
 * @brief MIDI 1.0 <-> UMP conversion (standalone, no AppleMIDI dependency)
 */

#include "NetworkMIDI2_Config.h"

#if defined(NETWORK_MIDI_2_SUPPORT)

#include "ump_Convert.h"

#define MIDI1_NOTE_OFF         0x80
#define MIDI1_NOTE_ON          0x90
#define MIDI1_POLY_PRESSURE    0xA0
#define MIDI1_CONTROL_CHANGE   0xB0
#define MIDI1_PROGRAM_CHANGE   0xC0
#define MIDI1_CHANNEL_PRESSURE 0xD0
#define MIDI1_PITCH_BEND       0xE0
#define MIDI1_SYSEX_START      0xF0
#define MIDI1_SYSEX_END        0xF7
#define MIDI1_REALTIME_MIN     0xF8

#define STATE_IDLE         0
#define STATE_HAVE_STATUS  1
#define STATE_HAVE_DATA1   2

namespace appleMidi {
namespace ump {

static uint8_t midi1_status_nibble(uint8_t status) {
    return (status >> 4) & 0x0F;
}
static uint8_t midi1_channel(uint8_t status) {
    return status & 0x0F;
}
static bool is_channel_voice(uint8_t status) {
    uint8_t n = midi1_status_nibble(status);
    return (n >= 0x8 && n <= 0xE);
}
static uint8_t channel_voice_data_bytes(uint8_t status_nibble) {
    switch (status_nibble) {
        case UMP_MIDI1_NOTE_OFF:
        case UMP_MIDI1_NOTE_ON:
        case UMP_MIDI1_POLY_PRESSURE:
        case UMP_MIDI1_CONTROL_CHANGE:
        case UMP_MIDI1_PITCH_BEND:
            return 2;
        case UMP_MIDI1_PROGRAM_CHANGE:
        case UMP_MIDI1_CHANNEL_PRESSURE:
            return 1;
        default:
            return 0;
    }
}

void ump_parser_init(ump_bytestream_parser_t *p) {
    if (p) {
        p->state = STATE_IDLE;
        p->running_status = 0;
        p->data_count = 0;
    }
}

ump_convert_result_t ump_byte_to_ump(ump_bytestream_parser_t *p, uint8_t byte,
                                    uint32_t *ump, uint8_t group) {
    if (!p || !ump) return UMP_CONVERT_ERROR;

    if (byte >= MIDI1_REALTIME_MIN) {
        if (byte == 0xFE) return UMP_CONVERT_SKIP;
        *ump = (UMP_MT_SYSTEM_REALTIME << UMP_MT_SHIFT) | ((uint32_t)byte << 16);
        return UMP_CONVERT_OK;
    }

    if (byte >= MIDI1_SYSEX_START) {
        p->running_status = 0;
        p->state = STATE_IDLE;
        return UMP_CONVERT_SKIP;
    }

    if (byte & 0x80) {
        if (!is_channel_voice(byte)) {
            p->running_status = 0;
            p->state = STATE_IDLE;
            return UMP_CONVERT_SKIP;
        }
        p->running_status = byte;
        p->data_count = 0;
        p->state = STATE_HAVE_STATUS;
        return UMP_CONVERT_NEED_MORE;
    }

    if (p->state == STATE_IDLE && p->running_status) {
        p->state = STATE_HAVE_STATUS;
        p->data_count = 0;
    }

    if (p->state != STATE_HAVE_STATUS && p->state != STATE_HAVE_DATA1) {
        p->state = STATE_IDLE;
        return UMP_CONVERT_ERROR;
    }

    uint8_t status_nibble = midi1_status_nibble(p->running_status);
    uint8_t n_data = channel_voice_data_bytes(status_nibble);

    if (n_data == 0) {
        p->state = STATE_IDLE;
        return UMP_CONVERT_ERROR;
    }

    p->data[p->data_count++] = byte;

    if (p->data_count < n_data) {
        p->state = STATE_HAVE_DATA1;
        return UMP_CONVERT_NEED_MORE;
    }

    *ump = (UMP_MT_MIDI1_CV << UMP_MT_SHIFT)
         | ((uint32_t)(group & 0x0F) << UMP_GROUP_SHIFT)
         | ((uint32_t)(status_nibble & 0x0F) << UMP_MT2_STATUS_SHIFT)
         | ((uint32_t)(midi1_channel(p->running_status) & 0x0F) << UMP_MT2_CHANNEL_SHIFT)
         | ((uint32_t)p->data[0] << UMP_MT2_DATA1_SHIFT);
    if (n_data == 2) *ump |= (p->data[1] & 0xFF);

    p->state = STATE_IDLE;
    return UMP_CONVERT_OK;
}

bool ump_has_second_word(const ump_bytestream_parser_t *p) {
    (void)p;
    return false;
}

uint32_t ump_build_midi1_channel_voice(uint8_t status, uint8_t data1, uint8_t data2,
                                       uint8_t group) {
    if (!is_channel_voice(status)) return 0;
    uint8_t status_nibble = midi1_status_nibble(status);
    uint8_t channel = midi1_channel(status);
    uint8_t n_data = channel_voice_data_bytes(status_nibble);

    uint32_t ump = (UMP_MT_MIDI1_CV << UMP_MT_SHIFT)
                 | ((uint32_t)(group & 0x0F) << UMP_GROUP_SHIFT)
                 | ((uint32_t)(status_nibble & 0x0F) << UMP_MT2_STATUS_SHIFT)
                 | ((uint32_t)(channel & 0x0F) << UMP_MT2_CHANNEL_SHIFT)
                 | ((uint32_t)data1 << UMP_MT2_DATA1_SHIFT);
    if (n_data >= 2) ump |= (data2 & 0xFF);
    return ump;
}

int ump_to_bytes(uint32_t ump, uint8_t *bytes, size_t maxlen) {
    if (!bytes || maxlen < 1) return -1;

    uint8_t mt = (ump >> UMP_MT_SHIFT) & 0x0F;

    if (mt == UMP_MT_SYSTEM_REALTIME) {
        bytes[0] = (ump >> 16) & 0xFF;
        return 1;
    }

    if (mt == UMP_MT_MIDI1_CV) {
        uint8_t status_nibble = (ump >> UMP_MT2_STATUS_SHIFT) & 0x0F;
        uint8_t channel = (ump >> UMP_MT2_CHANNEL_SHIFT) & 0x0F;
        uint8_t data1 = (ump >> UMP_MT2_DATA1_SHIFT) & 0xFF;
        uint8_t data2 = ump & 0xFF;

        uint8_t status = (status_nibble << 4) | channel;
        uint8_t n_data = channel_voice_data_bytes(status_nibble);

        if (maxlen < (size_t)(1 + n_data)) return -1;

        bytes[0] = status;
        bytes[1] = data1;
        if (n_data == 2) {
            bytes[2] = data2;
            return 3;
        }
        return 2;
    }

    return -1;
}

bool ump_is_multi_word_start(uint32_t ump) {
    uint8_t mt = (ump >> UMP_MT_SHIFT) & 0x0F;
    return (mt == UMP_MT_DATA || mt == UMP_MT_SYSEX7 || mt == UMP_MT_SYSEX8_MDS);
}

} /* namespace ump */
} /* namespace appleMidi */

#endif /* NETWORK_MIDI_2_SUPPORT */

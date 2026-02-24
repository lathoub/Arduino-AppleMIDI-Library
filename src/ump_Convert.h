#pragma once

#if defined(NETWORK_MIDI_2_SUPPORT)

/**
 * @file ump_Convert.h
 * @brief MIDI 1.0 byte stream <-> Universal MIDI Packet (UMP) conversion
 *
 * Standalone - no dependency on AppleMIDI. For Network MIDI 2.0 transport.
 *
 * Supports:
 *   - MT 2: MIDI 1.0 Channel Voice (Note On/Off, CC, PC, etc.)
 *   - MT 1: System Real-Time (Timing Clock, Start, Stop, etc.)
 */

#include "ump_Defs.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
namespace appleMidi {
namespace ump {
#endif

/* ============================================================================
 * Byte stream -> UMP conversion (stateful parser)
 * ============================================================================ */

typedef enum {
    UMP_CONVERT_NEED_MORE,
    UMP_CONVERT_OK,
    UMP_CONVERT_SKIP,
    UMP_CONVERT_ERROR
} ump_convert_result_t;

typedef struct {
    uint8_t  state;
    uint8_t  running_status;
    uint8_t  data_count;
    uint8_t  reserved;
    uint8_t  data[2];
} ump_bytestream_parser_t;

void ump_parser_init(ump_bytestream_parser_t *p);

ump_convert_result_t ump_byte_to_ump(ump_bytestream_parser_t *p, uint8_t byte,
                                    uint32_t *ump, uint8_t group);

bool ump_has_second_word(const ump_bytestream_parser_t *p);

/**
 * Build UMP from complete MIDI 1.0 channel voice message.
 * @param status MIDI status byte (0x80-0xEF)
 * @param data1  First data byte
 * @param data2  Second data byte (0 for PC, Channel Pressure)
 * @param group  UMP Group (0-15)
 */
uint32_t ump_build_midi1_channel_voice(uint8_t status, uint8_t data1, uint8_t data2,
                                       uint8_t group);

/* ============================================================================
 * UMP -> Byte stream conversion
 * ============================================================================ */

int ump_to_bytes(uint32_t ump, uint8_t *bytes, size_t maxlen);

bool ump_is_multi_word_start(uint32_t ump);

#ifdef __cplusplus
} /* namespace ump */
} /* namespace appleMidi */
#endif

#endif /* NETWORK_MIDI_2_SUPPORT */

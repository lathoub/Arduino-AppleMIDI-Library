#pragma once

#if defined(NETWORK_MIDI_2_SUPPORT)

/**
 * @file ump_Defs.h
 * @brief Universal MIDI Packet (UMP) definitions for Network MIDI 2.0
 *
 * UMP format as per M2-104-UM (UMP and MIDI 2.0 Protocol Specification).
 * Standalone - no dependency on AppleMIDI/RTP-MIDI implementation.
 */

#include <stdint.h>

#ifdef __cplusplus
namespace appleMidi {
namespace ump {
#endif

/* ============================================================================
 * UMP Message Types (MT) - bits 31-28 of first UMP word
 * ============================================================================ */
#define UMP_MT_MASK           0xF0000000u
#define UMP_MT_SHIFT          28

#define UMP_MT_UTILITY         0x0u  /* Utility (no Group) */
#define UMP_MT_SYSTEM_REALTIME 0x1u  /* System Real-Time (no Group) */
#define UMP_MT_MIDI1_CV        0x2u  /* MIDI 1.0 Channel Voice */
#define UMP_MT_DATA            0x3u  /* SysEx (64/96/128-bit) */
#define UMP_MT_MIDI2_CV        0x4u  /* MIDI 2.0 Channel Voice */
#define UMP_MT_MIDI2_CVM       0x5u  /* MIDI 2.0 Channel Voice Mixed */
#define UMP_MT_SYSEX7          0x6u  /* SysEx 7-bit (64/96/128-bit) */
#define UMP_MT_SYSEX8_MDS      0x7u  /* SysEx 8-bit + MIDI-CI */
#define UMP_MT_FLEX            0xFu  /* Flexible (Stream, Endpoint, etc.) */

/* ============================================================================
 * UMP Group - bits 27-24 (0-15)
 * ============================================================================ */
#define UMP_GROUP_MASK        0x0F000000u
#define UMP_GROUP_SHIFT       24

/* ============================================================================
 * UMP Message Type 2: MIDI 1.0 Channel Voice (32-bit)
 * Format: MT(4) | Group(4) | Status(4) | Channel(4) | Data1(8) | Data2(8)
 * ============================================================================ */
#define UMP_MT2_STATUS_MASK   0x00F00000u
#define UMP_MT2_STATUS_SHIFT  20
#define UMP_MT2_CHANNEL_MASK  0x000F0000u
#define UMP_MT2_CHANNEL_SHIFT 16
#define UMP_MT2_DATA1_MASK    0x0000FF00u
#define UMP_MT2_DATA1_SHIFT   8
#define UMP_MT2_DATA2_MASK    0x000000FFu

/* MIDI 1.0 status nibbles (high nibble of status byte) */
#define UMP_MIDI1_NOTE_OFF          0x8u
#define UMP_MIDI1_NOTE_ON           0x9u
#define UMP_MIDI1_POLY_PRESSURE     0xAu
#define UMP_MIDI1_CONTROL_CHANGE    0xBu
#define UMP_MIDI1_PROGRAM_CHANGE    0xCu
#define UMP_MIDI1_CHANNEL_PRESSURE  0xDu
#define UMP_MIDI1_PITCH_BEND        0xEu

/* ============================================================================
 * UMP Message Type 1: System Real-Time (32-bit)
 * ============================================================================ */
#define UMP_MIDI1_SYS_REALTIME_TIMING_CLOCK  0xF8u
#define UMP_MIDI1_SYS_REALTIME_START         0xFAu
#define UMP_MIDI1_SYS_REALTIME_CONTINUE      0xFBu
#define UMP_MIDI1_SYS_REALTIME_STOP          0xFCu
#define UMP_MIDI1_SYS_REALTIME_ACTIVE_SENSE  0xFEu
#define UMP_MIDI1_SYS_REALTIME_SYS_RESET     0xFFu

/* Default UMP Group (Group 0 = first group) */
#define UMP_DEFAULT_GROUP  0u

#ifdef __cplusplus
} /* namespace ump */
} /* namespace appleMidi */
#endif

#endif /* NETWORK_MIDI_2_SUPPORT */

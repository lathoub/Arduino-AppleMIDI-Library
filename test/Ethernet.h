#pragma once
#include <stdio.h>

#include "Arduino.h"


class EthernetUDP
{
    Deque<byte, 4096> _buffer;
    uint16_t _port;

public:

    EthernetUDP()
    {
        _port = 0;
    }

    void begin(uint16_t port)
    {
        _port = port;

        if (port == 5004 && true)
        {
            // AppleMIDI messages
        }
        
        if (port == 5005 && true)
        {
            // rtp-MIDI and AppleMIDI messages
            
        byte aa[] = {
                 0x80, 0x61, 0xbf, 0xa2, 0x12, 0xb, 0x5a, 0xf7, 0xaa, 0x34, 0x96, 0x4a,
                 0xc0, 0x2b,
                 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x0, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x0,
                 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x00, 0xf8, 0x0, 0xf8, 0x00, 0xf8, 0xc0, 0xbf, 0x89, 0x90, 0x05, 0xd0, 0x7a, 0xd5 };

            byte bb[] = { 0x80, 0x61, 0xD5, 0xE2, 0x18, 0xCC, 0xAD, 0x1D, 0xC5, 0xB1, 0x54, 0x0, 0x41, 0xF8, 0x20, 0xD5, 0x8B, 0x0, 0x9, 0x18, 0x80, 0x40, 0x81, 0xF1, 0x49, 0x40 };
            
            byte lowHighJournalWrong[] = {
                0x80, 0x61, 0xcc, 0x73, 0x19, 0xe,
                0x4e, 0xd4, 0xc5, 0xb1, 0x54, 0x00, 0x42, 0xd0, 0x30, 0x20, 0xcc, 0x4a, 0x00, 0x0a, 0x18, 0x8,
                0x40, 0x81, 0xf1, 0x90, 0x40, 0x2d
            };
            
            byte sysexJournalMalformed[] = {
                0x80, 0x61, 0x99, 0xc6, 0x1e, 0x90, 0x97, 0xc4, 0xc8, 0x86, 0x76, 0xf9,
                0xc0, 0xc2,
                0xf0,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19,
                    0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20,
                    0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19,
                    0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20,
                    0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19,
                    0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20,
                    0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19,
                    0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20,
                    0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x66,
                0xf7,
                0xc0, 0x99, 0x96, 0x90, 0x05, 0xd0, 0x00, 0x7b };

                
            byte sysexTimingActiveSensingJournal[] = {
                0x80, 0x61, 0xae, 0xae, 0x20, 0x7f, 0xd6, 0xe7, 0xc8, 0x86, 0x76, 0xf9,
                0xc0, 0xc6,
                0xf0,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19,
                    0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20,
                    0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19,
                    0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20,
                    0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19,
                    0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20,
                    0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19,
                    0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20,
                    0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x21,
                    0x19, 0x20, 0x21, 0x19, 0x20, 0x21, 0x19, 0x20, 0x66,
                0xf7,
                0x00, // time
                0xf8, // Timing Clock
                0x00, // Time
                0xfe, // Active Sensing
                0x40, 0xae, 0xa0, 0x10, 0x05, 0x50, 0x00, 0x8f }; // Journal
                
           byte sysexJournal[] = {
                0x80, 0x61, 0x85, 0xce, 0x1a, 0x5f, 0x1c, 0xa3, 0xc8, 0x86, 0x76, 0xf9,
                0xc1, 0x9a,
                0xf0,
                    0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x66, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x66, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21, 0x19, 0x19, 0x20, 0x21,
                    0x66,
                0xf7,
                0x40, 0x85, 0x8b, 0x10, 0x05, 0x50, 0x00, 0x8c };

            byte sysexMalformedTimingClock[] = {
                0x80, 0x61, 0x85, 0xd9, 0x1a, 0x5f, 0x26, 0xb0, 0xc8, 0x86, 0x76, 0xf9, 0x41, 0xf8, 0xc0, 0x85, 0x8b, 0x90, 0x05, 0xd0, 0x00, 0x95 };

            
            // sysex (command length is xx (or 0x71) in 2 bytes - B-FLAG)
            byte sysexSME[] = {
                0x80, 0x61, 0x9A, 0xF, 0x0, 0x2A, 0x7D, 0x3D, 0x29, 0xDC, 0x48, 0x99,
                0x80, 0x70,
                0xF0,
                  0x41,
                      0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
                      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
                      0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
                      0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
                      0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
                      0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
                      0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
                      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
                      0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
                      0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
                      0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
                0xF7 };

            byte sysexSE[] = {
                0x80, 0x61, 0x9A, 0xF, 0x0, 0x2A, 0x7D, 0x3D, 0x29, 0xDC, 0x48, 0x99,
                0x80, 0x3f,
                0xF0,
                  0x41,
                      0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
                      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
                      0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
                      0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
                      0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
                      0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
                0xF7 };

            byte sysexF[] = {
                0x80, 0x61, 0x7c, 0xbc, 0x0a, 0xff, 0x56, 0xba, 0x0a, 0x1a, 0x2f, 0x43,
                0x05,
                0xf0,
                    0x41, 0x19, 0x20,
                0xf7 };
            
            // 36 bytes
            byte noteOnOff[] = {
                0x80, 0x61, 0x27, 0x9e, 0x00, 0x1d, 0xb5, 0x36, 0x36, 0x09, 0x2f, 0x2a, // rtp
                // MIDI section
                0x46, // flag
                0x80, 0x3f, 0x00, // note off 63 on channel 1,
                0x00, // delta time
                0x3d, 0x00, // note 61
                // Journal Section (17 bytes)
                0x20, // journal flag
                0x27, 0x34, // sequence nr
                0x00, 0x0e, 0x08, // channel 1 channel flag
                0x02, 0x59, // note on off
                0xbd, 0x40, 0xbf, 0x40, // Log list
                0x15, 0xad, 0x5a, 0xdf, 0xa8, // offbit octets
            };
            
            byte noteOnOff2[] = {
                0x80, 0x61, 0x27, 0x9e, 0x00, 0x1d, 0xb5, 0x36, 0x36, 0x09, 0x2f, 0x2a, // rtp
                // MIDI section
                0x46, // flag
                0x80, 0x3f, 0x00, // note off 63 on channel 1,
                0x00, // delta time
                0x3d, 0x00, // note 61
                // Journal Section (17 bytes)
                0x20, // journal flag
                0x27, 0x34, // sequence nr
                0x00, 0x0e, 0x08, // channel 1 channel flag
                0x02, 0x59, // note on off
                0xbd, 0x40, 0xbf, 0x40, // Log list
                0x15, 0xad, 0x5a, 0xdf, 0xa8, // offbit octets

                0x80, 0x61, 0x27, 0x9e, 0x00, 0x1d, 0xb5, 0x36, 0x36, 0x09, 0x2f, 0x2a, // rtp
                // MIDI section
                0x46, // flag
                0x80, 0x3f, 0x00, // note off 63 on channel 1,
                0x00, // delta time
                0x3d, 0x00, // note off note 61 on channel 1 (note the running status)
                // Journal Section (17 bytes)
                0x20, // journal flag
                0x27, 0x34, // sequence nr
                0x00, 0x0e, 0x08, // channel 1 channel flag
                0x02, 0x59, // note on off
                0xbd, 0x40, 0xbf, 0x40, // Log list
                0x15, 0xad, 0x5a, 0xdf, 0xa8, // offbit octets
            };
            
            
            byte controlChange[] = {
  0x80, 0x61, 0x20, 0xa5, 0x7f, 0xc,
  0x73, 0x2d, 0xc5, 0xb1, 0x54, 0x00, 0x80, 0xbf, 0xb0, 0x7b, 0x00, 0x00, 0xb1, 0x7b, 0x00, 0x0,
  0xb2, 0x7b, 0x00, 0x00, 0xb3, 0x7b, 0x00, 0x00, 0xb4, 0x7b, 0x00, 0x00, 0xb5, 0x7b, 0x00, 0x0,
  0xb6, 0x7b, 0x00, 0x00, 0xb7, 0x7b, 0x00, 0x00, 0xb8, 0x7b, 0x00, 0x00, 0xb9, 0x7b, 0x00, 0x0,
  0xba, 0x7b, 0x00, 0x00, 0xbb, 0x7b, 0x00, 0x00, 0xbc, 0x7b, 0x00, 0x00, 0xbd, 0x7b, 0x00, 0x0,
  0xbe, 0x7b, 0x00, 0x00, 0xbf, 0x7b, 0x00, 0x00, 0xe0, 0x00, 0x40, 0x00, 0xe1, 0x00, 0x40, 0x0,
  0xe2, 0x00, 0x40, 0x00, 0xe3, 0x00, 0x40, 0x00, 0xe4, 0x00, 0x40, 0x00, 0xe5, 0x00, 0x40, 0x0,
  0xe6, 0x00, 0x40, 0x00, 0xe7, 0x00, 0x40, 0x00, 0xe8, 0x00, 0x40, 0x00, 0xe9, 0x00, 0x40, 0x0,
  0xea, 0x00, 0x40, 0x00, 0xeb, 0x00, 0x40, 0x00, 0xec, 0x00, 0x40, 0x00, 0xed, 0x00, 0x40, 0x0,
  0xee, 0x00, 0x40, 0x00, 0xef, 0x00, 0x40, 0x00, 0xb0, 0x40, 0x00, 0x00, 0xb1, 0x40, 0x00, 0x0,
  0xb2, 0x40, 0x00, 0x00, 0xb3, 0x40, 0x00, 0x00, 0xb4, 0x40, 0x00, 0x00, 0xb5, 0x40, 0x00, 0x0,
  0xb6, 0x40, 0x00, 0x00, 0xb7, 0x40, 0x00, 0x00, 0xb8, 0x40, 0x00, 0x00, 0xb9, 0x40, 0x00, 0x0,
  0xba, 0x40, 0x00, 0x00, 0xbb, 0x40, 0x00, 0x00, 0xbc, 0x40, 0x00, 0x00, 0xbd, 0x40, 0x00, 0x0,
                0xbe, 0x40, 0x00, 0x00, 0xbf, 0x40, 0x00  };
            
            byte RTStart[] = {
                 0x80, 0x61, 0x20, 0xa6, 0x7f, 0xc,  0x73, 0x66, 0xc5, 0xb1, 0x54, 0x00, 0x43,
                        0xfa, 0x00, 0xf8,
                        0x2f, 0x20, 0xa5,
                                0x00, 0x0a, 0x5, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x08, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x0, 0x40,
                                0x10, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x18, 0x0a, 0x50, 0x01, 0x4, 0x00, 0x7b,  0x00, 0x00, 0x40,
                                0x20, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x28, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x30, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7,  0x00, 0x00, 0x40,
                                0x38, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x40, 0x0a, 0x5,  0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x48, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x0,  0x40,
                                0x50, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x58, 0x0a, 0x50, 0x01, 0x4,  0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x60, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x68, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40,
                                0x70, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7,  0x00, 0x00, 0x40,
                                0x78, 0x0a, 0x50, 0x01, 0x40, 0x00, 0x7b, 0x00, 0x00, 0x40 };
            
            byte TCNote[] = {
                 0x80, 0x61, 0x4e, 0x24, 0x82, 0x9f, 0xdc, 0x22, 0xc5, 0xb1, 0x54, 0x00,
                       0xc0, 0x20,
                             0xf8, 0x00, 0x90, 0x2b, 0x7f, 0x00, 0x34, 0x7f, 0x00, 0x35, 0x7f, 0x00,
                                   0x36, 0x7f, 0x00, 0x37, 0x7f, 0x00, 0x38, 0x7f, 0x00, 0x39, 0x7f, 0x00,
                                   0x3a, 0x7f, 0x00, 0x3b, 0x7f, 0x00, 0x3c, 0x7f,
                             0x6f, 0x45, 0x85, 0x10, 0x05, 0x50, 0x00, 0x0f,
                                   0x80, 0x0f, 0x58, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0x80, 0x57, 0x10, 0x0f, 0xf8, 0x88,
                                   0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0x90, 0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb,
                                   0x00, 0x80, 0x40, 0x98, 0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0xa0, 0x0a, 0x50,
                                   0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0xa8, 0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80,
                                   0x40, 0xb0, 0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0xb8, 0x0a, 0x50, 0x81, 0xc0,
                                   0x00, 0xfb, 0x00, 0x80, 0x40, 0xc0, 0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0xc8,
                                   0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0xd0, 0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb,
                                   0x00, 0x80, 0x40, 0xd8, 0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0xe0, 0x0a, 0x50,
                                   0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0xe8, 0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80,
                                   0x40, 0xf0, 0x0a, 0x50, 0x81, 0xc0, 0x00, 0xfb, 0x00, 0x80, 0x40, 0xf8, 0x0a, 0x50, 0x81, 0xc0,
                                   0x00, 0xfb, 0x00, 0x80, 0x40 };
            
            byte aaa[] = {
                0x80, 0x61, 0xa5, 0x05, 0x01, 0x08, 0x58, 0x2a, 0x34, 0xc7, 0xab, 0xfd, 0x4e, 0x80, 0x53, 0x00, 0x11, 0x35, 0x00, 0x8f, 0xff, 0xff,
                0xff, 0x00, 0x90, 0x4f, 0x40, 0x20, 0xa4, 0xdb, 0x00, 0x13, 0x08, 0x03, 0x3a, 0xb5, 0x7f, 0xcd,
                0x40, 0xd3, 0x40, 0x02, 0x10, 0x10, 0x10, 0x08, 0x00, 0xa9, 0x48,
            };
            
            
            byte slecht[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
            
     //       write(noteOnOff, sizeof(noteOnOff));
        }

        
        
        if (port == 5005 && true)
        {
            // rtp-MIDI and AppleMIDI messages
        }

    };

    bool beginPacket(uint32_t, uint16_t)
    {
        return true;
    }

    bool beginPacket(IPAddress, uint16_t)
    {
        return true;
    }

    size_t parsePacket()
    {
        return _buffer.size();
    };

    size_t available()
    {
        return _buffer.size();
    };

    size_t read(byte* buffer, size_t size)
    {
        size = min(size, _buffer.size());
            
        for (size_t i = 0; i < size; i++)
            buffer[i] = _buffer.pop_front();

        return size;
    };

    void write(uint8_t buffer)
    {
        _buffer.push_back(buffer);
    };

    void write(uint8_t* buffer, size_t size)
    {
        for (size_t i = 0; i < size; i++)
            _buffer.push_back(buffer[i]);
    };

    void endPacket() { };
    void flush()
    {
        if (_port == 5004)
        {
            if (_buffer[0] == 0xff && _buffer[1] == 0xff && _buffer[2] == 'I' &&_buffer[3] == 'N')
            {
                _buffer.clear();

                
                byte u[] = {
                    0xff, 0xff,
                    0x4f, 0x4b,
                    0x00, 0x00, 0x00, 0x02,
                    0xb7, 0x06, 0x20, 0x30,
                    0xda, 0x8d, 0xc5, 0x8a,
                    0x4d, 0x61, 0x63, 0x62, 0x6f, 0x6f, 0x6b, 0x20, 0x50, 0x72, 0x6f, 0x20, 0x66, 0x72, 0x6f, 0x6d, 0x20, 0x53, 0x61, 0x6e, 0x64, 0x72, 0x61, 0x20, 0x56, 0x65, 0x72, 0x62, 0x65, 0x6b, 0x65, 0x6e, 0x20, 0x28, 0x32, 0x29, 0x00 };
                
                
                
                
                
                byte r[] = { 0xff, 0xff,
                                      0x4f, 0x4b,
                                      0x00, 0x0, 0x00, 0x02,
                                      0xb7, 0x06, 0x20, 0x30,
                                      0xda, 0x8d, 0xc5, 0x8a,
                                      0x53, 0x65, 0x73, 0x73, 0x69, 0x6, 0x6e, 0x31, 0x2d, 0x42, 0x00 };
                write(u, sizeof(u));
            }
        }
        if (_port == 5005)
        {
            if (_buffer[0] == 0xff && _buffer[1] == 0xff && _buffer[2] == 'I' &&_buffer[3] == 'N')
            {
                _buffer.clear();
                byte r[] = { 0xff, 0xff,
                                      0x4f, 0x4b,
                                      0x00, 0x0, 0x00, 0x02,
                                      0xb7, 0x06, 0x20, 0x30,
                                      0xda, 0x8d, 0xc5, 0x8a,
                                      0x53, 0x65, 0x73, 0x73, 0x69, 0x6, 0x6e, 0x31, 0x2d, 0x42, 0x00 };
                write(r, sizeof(r));
            }
            else if (_buffer[0] == 0xff && _buffer[1] == 0xff && _buffer[2] == 'C' &&_buffer[3] == 'K')
            {
                if (_buffer[8] == 0x00)
                {
                    _buffer.clear();
                    byte r[] = { 0xff, 0xff,
                                 0x43, 0x4b,
                                 0xda, 0x8d, 0xc5, 0x8a,
                                 0x01,
                                 0x65, 0x73, 0x73,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x34,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x6c, 0x83,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
                    write(r, sizeof(r));
                }
                else
                    _buffer.clear();
            }
        }
    };

    void stop() { _buffer.clear(); };

    uint32_t remoteIP() { return 1; }
    uint16_t remotePort() { return _port; }
};

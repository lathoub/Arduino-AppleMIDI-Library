# Basic RTPMIDI Cheat Sheet

From https://tools.ietf.org/html/rfc6295

## RTP Header

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    | V |P|X|  CC   |M|     PT      |        Sequence number        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                           Timestamp                           |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                             SSRC                              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                     MIDI command section ...                  |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                       Journal section ...                     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

| Bit             |  pos  | description                                                                                                              |
| --------------- | :---: | ------------------------------------------------------------------------------------------------------------------------ |
| V               |  0-1  | Version. 2.                                                                                                              |
| P               |   2   | Has paddding. RTP needs it for encryption. 0.                                                                            |
| X               |   3   | Header extension. 0.                                                                                                     |
| CC              |  4-7  | CSRC count. 0.                                                                                                           |
| M               |   8   | Has MIDI data.                                                                                                           |
| PT              | 9-15  | Payload type. Always 0x61. MIDI.                                                                                         |
| Sequence number | 16-31 | Starts random, increase one on each packet. (%2^16). There is an extended one with 32 bits and rollovers.                |
| Timestamp       | 32-63 | Time this packet was generated. On Apple midi the unit is 0.1 ms. (1^-4 seconds). Real RTPMIDI is at session connection. |
| SSRC            | 64-96 | Random unique SSRC for this sender. Same for all the session.                                                            |

Timestamp can be buffered to reduce jitter on the receiving end, creating a
continuous lag of a specific length.

## MIDI Command section

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |B|J|Z|P|LEN... |  MIDI list ...                                |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

| Bit    | pos | description                                                                                                      |
| ------ | :-: | ---------------------------------------------------------------------------------------------------------------- |
| B      |  0  | Length is 12 bits. If true length at 4-7 is MSB, and one more byte.                                              |
| J      |  1  | There is a journal                                                                                               |
| Z      |  2  | First midi command as is in MIDI section. No timestamp for first command.                                        |
| P      |  3  | Phantom MIDI command. The first command is a running command from previous stream.                               |
| length | 4-7 | How many bytes. May be extended with the B bit.                                                                  |
| MIDI   | ... | MIDI data, then timestamp, MIDI data, timestamp and so on.. or timestamp, midi data and so on. Depends on Z bit. |

Timestamps in running Length encoding. https://en.wikipedia.org/wiki/Run-length_encoding

# Journal

## Journal Bits

     0                   1                   2
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |S|Y|A|H|TOTCHAN|   Checkpoint Packet Seqnum    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

| Bit                      | pos  | description                                                              |
| ------------------------ | :--: | ------------------------------------------------------------------------ |
| S                        |  0   | Single packet loss. To indicate only one packet is described in journal. |
| Y                        |  1   | Has system journal                                                       |
| A                        |  2   | Has channel journals. Needs totchan.                                     |
| H                        |  3   | Enhanced Chapter C encoding.                                             |
| TOTCHAN                  | 4-7  | Nr channels -1 (has totchan + 1 channels)                                |
| Checkpoint packet seqnum | 8-23 | Seq nr for this journal Normally the one before the current packet.      |

## Channel Journal

One for each (TOTCHAN + 1)

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |S| CHAN  |H|      LENGTH       |P|C|M|W|N|E|T|A|  Chapters ... |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

| Bit    |  pos   | description                                                              |
| ------ | :----: | ------------------------------------------------------------------------ |
| S      |   0    | Single packet loss. To indicate only one packet is described in journal. |
| CHAN   |  1-4   | Channel number                                                           |
| H      |   5    | Whether controllers are Enhanced Chapter C.                               |
| LENGTH |  6-15  | Lenght of the journal                                                    |
| P      | 16 / 0 | Chapter P. Program Change.                                               |
| C      | 17 / 1 | Chapter C. Control Change.                                               |
| M      | 18 / 2 | Chapter M. Parameter System.                                             |
| W      | 19 / 3 | Chapter W. Pitch Wheel.                                                  |
| N      | 20 / 4 | Chapter N. Note On/Off                                                   |
| E      | 21 / 5 | Chapter E. Note Command Extras                                           |
| T      | 22 / 6 | Chapter T. After Touch.                                                  |
| A      | 23 / 7 | Chapter A. Poly Aftertouch.                                              |

## Chapter P

     0                   1                   2
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |S|   PROGRAM   |B|   BANK-MSB  |X|  BANK-LSB   |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

## Chapter C

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 8 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |S|     LEN     |S|   NUMBER    |A|  VALUE/ALT  |S|   NUMBER    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |A|  VALUE/ALT  |  ....                                         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

## Chapter M

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 8 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |S|     LEN     |S|   NUMBER    |A|  VALUE/ALT  |S|   NUMBER    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |A|  VALUE/ALT  |  ....                                         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

# Chapter W

     0                   1
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |S|     FIRST   |R|    SECOND   |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

## Chapter N

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 8 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |B|     LEN     |  LOW  | HIGH  |S|   NOTENUM   |Y|  VELOCITY   |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |S|   NOTENUM   |Y|  VELOCITY   |             ....              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |    OFFBITS    |    OFFBITS    |     ....      |    OFFBITS    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

| Bit |   pos   | description                          |
| --- | :-----: | ------------------------------------ |
| B   |    0    | S-Style functionality. By default 1. |
| S   |   16n   | If B is 0, all S are 0.              |
| Y   | 16n + 8 | Recomendation to play.               |

## Chapter T

    0
    0 1 2 3 4 5 6 7
    +-+-+-+-+-+-+-+-+
    |S|   PRESSURE  |
    +-+-+-+-+-+-+-+-+

## Chapter A

     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 8 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |S|    LEN      |S|   NOTENUM   |X|  PRESSURE   |S|   NOTENUM   |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |X|  PRESSURE   |  ....                                         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

(original by https://github.com/davidmoreno/rtpmidid/blob/master/docs/RFC6295_notes.md)

#pragma once

#if !defined(BYTE_ORDER) 

    #ifndef BIG_ENDIANs
    #define BIG_ENDIAN 4321
    #endif
    #ifndef LITTLE_ENDIAN
    #define LITTLE_ENDIAN 1234
    #endif

    #define TEST_LITTLE_ENDIAN (((union { unsigned x; unsigned char c; }){1}).c)

    #ifdef TEST_LITTLE_ENDIAN
    #define BYTE_ORDER LITTLE_ENDIAN
    #else
    #define BYTE_ORDER BIG_ENDIAN
    #endif

    #undef TEST_LITTLE_ENDIAN
#endif

#include <stdint.h>

#ifndef __bswap16
    #define __bswap16(x) ((uint16_t)((((uint16_t)(x)&0xff00) >> 8) | (((uint16_t)(x)&0x00ff) << 8)))
#endif

#ifndef __bswap32
    #define __bswap32(x)                                                                     \
        ((uint32_t)((((uint32_t)(x)&0xff000000) >> 24) | (((uint32_t)(x)&0x00ff0000) >> 8) | \
                    (((uint32_t)(x)&0x0000ff00) << 8) | (((uint32_t)(x)&0x000000ff) << 24)))
#endif

#ifndef __bswap64
    #define __bswap64(x)                                            \
        ((uint64_t)((((uint64_t)(x)&0xff00000000000000ULL) >> 56) | \
                    (((uint64_t)(x)&0x00ff000000000000ULL) >> 40) | \
                    (((uint64_t)(x)&0x0000ff0000000000ULL) >> 24) | \
                    (((uint64_t)(x)&0x000000ff00000000ULL) >> 8) |  \
                    (((uint64_t)(x)&0x00000000ff000000ULL) << 8) |  \
                    (((uint64_t)(x)&0x0000000000ff0000ULL) << 24) | \
                    (((uint64_t)(x)&0x000000000000ff00ULL) << 40) | \
                    (((uint64_t)(x)&0x00000000000000ffULL) << 56)))
#endif

#if BYTE_ORDER == LITTLE_ENDIAN

#if !defined(ntohs)
    #define ntohs(x) __bswap16(x)
#endif
#if !defined(htons)
    #define htons(x) __bswap16(x)
#endif
#if !defined(ntohl)
    #define ntohl(x) __bswap32(x)
#endif
#if !defined(htonl)
    #define htonl(x) __bswap32(x)
#endif
#if !defined(ntohll)
    #define ntohll(x) __bswap64(x)
#endif
#if !defined(htonll)
    #define htonll(x) __bswap64(x)
#endif

#else // BIG_ENDIAN

#if !defined(ntohs)
    #define ntohl(x) ((uint32_t)(x))
#endif
#if !defined(ntohs)
    #define ntohs(x) ((uint16_t)(x))
#endif
#if !defined(htonl)
    #define htonl(x) ((uint32_t)(x))
#endif
#if !defined(htons)
    #define htons(x) ((uint16_t)(x))
#endif
#if !defined(ntohll)
    #define ntohll(x) ((uint64_t)(x))
#endif
#if !defined(htonll)
    #define htonll(x) ((uint64_t)(x))
#endif

#endif

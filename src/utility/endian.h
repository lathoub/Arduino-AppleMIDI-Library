#pragma once

#if !defined(_BYTE_ORDER) 

    #define _BIG_ENDIAN 4321
    #define _LITTLE_ENDIAN 1234

    #define TEST_LITTLE_ENDIAN (((union { unsigned x; unsigned char c; }){1}).c)

    #ifdef TEST_LITTLE_ENDIAN
    #define _BYTE_ORDER _LITTLE_ENDIAN
    #else
    #define _BYTE_ORDER _BIG_ENDIAN
    #endif

    #undef TEST_LITTLE_ENDIAN

    #include <stdint.h>

    #ifdef __GNUC__
    #define	__bswap16(_x)	__builtin_bswap16(_x)
    #define	__bswap32(_x)	__builtin_bswap32(_x)
    #define	__bswap64(_x)	__builtin_bswap64(_x)
    #else /* __GNUC__ */

    static __inline __uint16_t
    __bswap16(__uint16_t _x)
    {
        return ((__uint16_t)((_x >> 8) | ((_x << 8) & 0xff00)));
    }

    static __inline __uint32_t
    __bswap32(__uint32_t _x)
    {
        return ((__uint32_t)((_x >> 24) | ((_x >> 8) & 0xff00) |
            ((_x << 8) & 0xff0000) | ((_x << 24) & 0xff000000)));
    }

    static __inline __uint64_t
    __bswap64(__uint64_t _x)
    {
        return ((__uint64_t)((_x >> 56) | ((_x >> 40) & 0xff00) |
            ((_x >> 24) & 0xff0000) | ((_x >> 8) & 0xff000000) |
            ((_x << 8) & ((__uint64_t)0xff << 32)) |
            ((_x << 24) & ((__uint64_t)0xff << 40)) |
            ((_x << 40) & ((__uint64_t)0xff << 48)) | ((_x << 56))));
    }
    #endif /* !__GNUC__ */

    #ifndef __machine_host_to_from_network_defined
    #if _BYTE_ORDER == _LITTLE_ENDIAN
    #define __ntohs(x) __bswap16(x)
    #define __htons(x) __bswap16(x)
    #define __ntohl(x) __bswap32(x)
    #define __htonl(x) __bswap32(x)
    #define __ntohll(x) __bswap64(x)
    #define __htonll(x) __bswap64(x)
    #else // BIG_ENDIAN
    #define __ntohl(x) ((uint32_t)(x))
    #define __ntohs(x) ((uint16_t)(x))
    #define __htonl(x) ((uint32_t)(x))
    #define __htons(x) ((uint16_t)(x))
    #define __ntohll(x) ((uint64_t)(x))
    #define __htonll(x) ((uint64_t)(x))
    #endif
    #endif /* __machine_host_to_from_network_defined */

#endif /* _BYTE_ORDER */

#ifndef __machine_host_to_from_network_defined
#if _BYTE_ORDER == _LITTLE_ENDIAN
#define __ntohll(x) __bswap64(x)
#define __htonll(x) __bswap64(x)
#else // BIG_ENDIAN
#define __ntohll(x) ((uint64_t)(x))
#define __htonll(x) ((uint64_t)(x))
#endif
#endif /* __machine_host_to_from_network_defined */

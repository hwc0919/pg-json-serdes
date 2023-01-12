//
// Created by wanchen.he on 2023/1/12.
//
#pragma once

#pragma once

#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#else // some Unix-like OS
#include <arpa/inet.h>
#endif

#if defined __linux__ || defined __FreeBSD__ || defined __OpenBSD__ || defined __MINGW32__ || defined __HAIKU__

#ifdef __linux__
#include <endian.h> // __BYTE_ORDER __LITTLE_ENDIAN
#elif defined __FreeBSD__ || defined __OpenBSD__
#include <sys/endian.h> // _BYTE_ORDER _LITTLE_ENDIAN
#define __BYTE_ORDER    _BYTE_ORDER
#define __LITTLE_ENDIAN _LITTLE_ENDIAN
#elif defined __MINGW32__
#include <sys/param.h> // BYTE_ORDER LITTLE_ENDIAN
#define __BYTE_ORDER    BYTE_ORDER
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#endif

#include <algorithm>

#else

#define USE_BUILTIN_NTOHLL

#endif

namespace pg_json
{

class ByteOrder
{
public:
    template <typename T>
    static T hton(T val)
    {
        return swap(val);
    }

    template <typename T>
    static T ntoh(T val)
    {
        return swap(val);
    }

private:
    static unsigned long long swap(unsigned long long val)
    {
#ifdef USE_BUILTIN_NTOHLL
        return ::htonll(val);

#else

#if __BYTE_ORDER == __LITTLE_ENDIAN
        char * ptr = reinterpret_cast<char *>(&val);
        std::reverse(ptr, ptr + sizeof(val));
#endif
        return val;
#endif
    }

    static long long swap(long long val)
    {
        return swap(static_cast<unsigned long long>(val));
    }

    static unsigned long swap(unsigned long val)
    {
        if (sizeof(long) == sizeof(int))
        {
            return swap(static_cast<uint32_t>(val));
        }
        return swap(static_cast<unsigned long long>(val));
    }

    static long swap(long val)
    {
        if (sizeof(long) == sizeof(int))
        {
            return swap(static_cast<int32_t>(val));
        }
        return swap(static_cast<unsigned long long>(val));
    }

    static uint32_t swap(uint32_t val)
    {
        return ::htonl(val);
    }

    static int32_t swap(int32_t val)
    {
        return ::htonl(val);
    }

    static uint16_t swap(uint16_t val)
    {
        return ::htons(val);
    }

    static int16_t swap(int16_t val)
    {
        return ::htons(val);
    }

    static uint8_t swap(uint8_t val)
    {
        return val;
    }

    static int8_t swap(int8_t val)
    {
        return val;
    }
};

} // namespace pg_json

#ifdef USE_BUILTIN_NTOHLL
#undef USE_BUILTIN_NTOHLL
#endif

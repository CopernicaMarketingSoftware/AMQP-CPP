/**
 *  Endian.h
 * 
 *  On Apple systems, there are no macro's to convert between little
 *  and big endian byte orders. This header file adds the missing macros
 * 
 *  @author madmongo1 <https://github.com/madmongo1>
 */
 
/**
 *  Include guard
 */
#pragma once

/**
 *  The contents of the file are only relevant for Apple
 */
#if defined(__APPLE__)

// dependencies
#include <machine/endian.h>
#include <libkern/OSByteOrder.h>

// define 16 bit macros
#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

// define 32 bit macros
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

// define 64 but macros
#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

// not on apple
#else

// non-apple systems have their own endian header file
#include <endian.h>

// end of "#if defined(__APPLE__)"
#endif


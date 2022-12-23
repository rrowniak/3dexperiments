#pragma once

#include <SDL.h>


#include <cstdint>
#include <cmath>
#include <cstring>
#include <chrono>
#include <stdexcept>
#include <limits>

using PALETTEENTRY = SDL_Color;
typedef PALETTEENTRY* LPPALETTEENTRY;

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;
typedef unsigned int   QUAD;
typedef unsigned int   UINT;
typedef int64_t INT64;

typedef void* LPDIRECTDRAWSURFACE7;
typedef void* LPDIRECTDRAWCLIPPER;
typedef void* COLORREF;

typedef uint64_t LONG;
typedef uint32_t DWORD;   // DWORD = unsigned 32 bit value
typedef uint16_t WORD;    // WORD = unsigned 16 bit value
typedef uint8_t BYTE;     // BYTE = unsigned 8 bit value

typedef LONG HRESULT;

#define FAILED(hr) (((HRESULT)(hr)) < 0)

#define INT_MAX (std::numeric_limits<int>::max())
#define INT_MIN (std::numeric_limits<int>::min())


typedef  enum
 {
   PC_RESERVED = 0x01,
   PC_EXPLICIT = 0x02,
   PC_NOCOLLAPSE = 0x04
 } PaletteEntryFlag;

 #define _RGB32Bit(r, g, b) ((r << 16) + (g << 8) + b)
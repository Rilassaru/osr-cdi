/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#ifndef __CUSTOMIZED_TYPE_DEFS_H_
#define __CUSTOMIZED_TYPE_DEFS_H_

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/** -----------------------------------------------------------------
 * Compiler & Macro definitions
------------------------------------------------------------------ */
#define ROM     const
#define rom

#if !defined(__PACKED)
    #define __PACKED
#endif

#ifndef Nop
#define Nop()       {asm("NOP");}
#endif
#ifndef ClrWdt
#define ClrWdt()    {asm("CLRWDT");}
#endif
#ifndef Reset
#define Reset()     {asm("RESET");}
#endif
#ifndef Sleep
#define Sleep()     {asm("SLEEP");}
#endif

#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

/** -----------------------------------------------------------------
 * Legacy Type Aliases (Used by Microchip USB Framework)
------------------------------------------------------------------ */
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

/** -----------------------------------------------------------------
 * Byte / Word Unions
------------------------------------------------------------------ */
typedef union
{
    uint8_t Val;
    struct
    {
        uint8_t b0:1;
        uint8_t b1:1;
        uint8_t b2:1;
        uint8_t b3:1;
        uint8_t b4:1;
        uint8_t b5:1;
        uint8_t b6:1;
        uint8_t b7:1;
    } bits;
} UINT8_VAL, UINT8_BITS, BYTE_VAL, BYTE_BITS;

typedef union
{
    uint16_t Val;
    uint8_t v[2] __PACKED;
    struct __PACKED
    {
        uint8_t LB;
        uint8_t HB;
    } byte;
    struct __PACKED
    {
        uint8_t b0:1;
        uint8_t b1:1;
        uint8_t b2:1;
        uint8_t b3:1;
        uint8_t b4:1;
        uint8_t b5:1;
        uint8_t b6:1;
        uint8_t b7:1;
        uint8_t b8:1;
        uint8_t b9:1;
        uint8_t b10:1;
        uint8_t b11:1;
        uint8_t b12:1;
        uint8_t b13:1;
        uint8_t b14:1;
        uint8_t b15:1;
    } bits;
} UINT16_VAL, UINT16_BITS, WORD_VAL, WORD_BITS;

typedef union
{
    uint32_t Val;
    uint16_t w[2] __PACKED;
    uint8_t  v[4] __PACKED;
    struct __PACKED
    {
        uint16_t LW;
        uint16_t HW;
    } word;
    struct __PACKED
    {
        uint8_t LB;
        uint8_t HB;
        uint8_t UB;
        uint8_t MB;
    } byte;
} UINT32_VAL, DWORD_VAL;

#endif /* __CUSTOMIZED_TYPE_DEFS_H_ */
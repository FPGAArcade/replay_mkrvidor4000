/*--------------------------------------------------------------------
 *                       Replay Firmware
 *                      www.fpgaarcade.com
 *                     All rights reserved.
 *
 *                     admin@fpgaarcade.com
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *--------------------------------------------------------------------
 *
 * Copyright (c) 2020, The FPGAArcade community (see AUTHORS.txt)
 *
 */

#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include "SPI.h"
#include "replay.h"
// Uncomment after patching (see wifinina.cpp)
// #include <ArduinoBLE.h>

// hardware/irq.h
extern "C" unsigned disableIRQ(void);
extern "C" unsigned enableIRQ(void);

// hardware_vidor/usbblaster.h
extern "C" void USBBlaster_Disable();

void setup() {

  Insert_SRAM_Sentinels();

  char buf[256];
  sprintf(buf, "FPGAArcade Replay VIDOR %s %s", __DATE__, __TIME__);

  Serial1.begin(115200);
  Serial1.println("\033[2J");
  Serial1.println("=========================================================");
  Serial1.println(buf);
  Serial1.println("=========================================================");
  Serial1.flush();

  USBBlaster_Disable();
}

void loop() {
  replay_main();
}

static __attribute__ ((noinline)) void Insert_SRAM_Sentinels()
{
  disableIRQ();

  const void* heap_end = sbrk(0);
  const void* stack_end = __builtin_frame_address(0);
  const uint32_t sentinel = 0xFA57F00D;

  uint32_t* start = (uint32_t*)((((intptr_t)heap_end)+3) & ~ 0x3);
  uint32_t*   end = (uint32_t*)((((intptr_t)stack_end) ) & ~ 0x3);

  while(start < end)
  {
    *start++ = sentinel;
  }

  enableIRQ();
}

__attribute__((naked)) void HardFault_Handler(void)
{
  /*
   * Get the appropriate stack pointer, depending on our mode,
   * and use it as the parameter to the C handler. This function
   * will never return
   *
   * Based on https://electronics.stackexchange.com/questions/293772/finding-the-source-of-a-hard-fault-using-extended-hardfault-handler
   */

 __asm(  ".syntax unified\n"
         "movs   r0, #4  \n"
         "mov    r1, lr  \n"
         "tst    r0, r1  \n"
         "beq    _msp    \n"
         "mrs    r0, psp \n"
         "b      PrintExceptionInfo \n"
         "_msp:  \n"
         "mrs    r0, msp \n"
         "b      PrintExceptionInfo \n"
         ".syntax divided\n") ;
}

extern "C" void PrintExceptionInfo(unsigned long* stack)
{
  static const char* regs[] = { "R0", "R1", "R2", "R3", "R12", "LR", "PC", "PSR", "SP" };

  kprintf("\n\r\n\r ** Hard Fault **\n\r\n\r");

  kprintf("***********************************************************************\n\r");
  kprintf("* %3s = %08x  %3s = %08x  %3s = %08x  %3s = %08x      *\n\r", regs[0], stack[0], regs[1], stack[1], regs[2], stack[2], regs[3], stack[3]);
  kprintf("* %3s = %08x  %3s = %08x  %3s = %08x  %3s = %08x      *\n\r", regs[4], stack[4], regs[5], stack[5], regs[6], stack[6], regs[7], stack[7]);
  kprintf("* %3s = %08x                                                      *\n\r", regs[8], &stack[0]);
  kprintf("***********************************************************************\n\r");

  for (int i = 0; i < 7; ++i)
  {
    const unsigned long rom_start = 0x00002000, rom_end = 0x00040000;
    const unsigned long ram_start = 0x20000000, ram_end = 0x20008000;
    unsigned long r = stack[i];

    if (((rom_start <= r) && (r < rom_end)) ||
        ((ram_start <= r) && (r < ram_end)))
    {
      kprintf("* @ %3s :                                                             *\n\r", regs[i]);
      kprintmem((const uint8_t*)(r & ~0x3) - 32, 64);
      kprintf("*                                                                     *\n\r");
    }
  }

  kprintf("* @ %3s:                                                              *\n\r", regs[8]);
  kprintmem((const uint8_t*)&stack[8], 512);
  kprintf("*                                                                     *\n\r");

  kprintf("***********************************************************************\n\r");

  pinMode(LED_BUILTIN, OUTPUT);
  while(true)
  {
   for (int i = 0; i < 100000; ++i)
     digitalWrite(LED_BUILTIN, HIGH);
   for (int i = 0; i < 100000; ++i)
     digitalWrite(LED_BUILTIN, LOW);
  }
}

static char kprintbuf[256];
static void kprintstr(const char* str)
{
  if (!str)
    return;
  while(*str)
  {
    sercom5.writeDataUART(*str++);
  }
}

extern "C" int kprintf(const char * fmt, ...)
{
  disableIRQ();

  va_list args;
  va_start (args, fmt);
  int n = vsnprintf (kprintbuf, sizeof(kprintbuf)-1, fmt, args);
  if (0 < n && n < (int)sizeof(kprintbuf))
  {
    kprintbuf[n] = 0;
    kprintstr(kprintbuf);
  }
  va_end (args);

  enableIRQ();

  return n;
}

extern "C" void kprintmem(const uint8_t* memory, uint32_t size)
{
  uint32_t i, j, len;
  char format[150];
  char alphas[27];
  strcpy(format, "* 0x%08X: %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X ");

  const unsigned long rom_start = 0x00000000, rom_end = 0x00040000;
  const unsigned long ram_start = 0x20000000, ram_end = 0x20008000;

  // Make sure we don't access invalid addresses, outside of ROM/RAM by cutting 
  // size short 16 bytes from the end (because we will dereference 16 bytes later)
  uintptr_t start = (uintptr_t)memory;
  if ((signed)rom_start <= (signed)start && start+16 < rom_end)     // this is a ROM address
    if ((start + size) > rom_end)
      size = ((rom_end - start + 15) & ~15) - 16;
  if (ram_start <= start && start+16 < ram_end)     // this is a RAM address
    if ((start + size) > ram_end)
      size = ((ram_end - start + 15) & ~15) - 16;

  for (i = 0; i < size; i += 16) {
    len = size - i;

        // last line is less than 16 bytes? rewrite the format string
    if (len < 16) {
      strcpy(format, "* 0x%08X: ");

      for (j = 0; j < 16; ++j) {
        if (j < len) {
          strcat(format, "%02X");

        } else {
          strcat(format, "__");
        }

        if ((j & 0x3) == 0x3) {
          strcat(format, " ");
        }
      }

    } else {
      len = 16;
    }

        // create the ascii representation
    for (j = 0; j < len; ++j) {
      alphas[j] = (isalnum(memory[i + j]) ? memory[i + j] : '.');
    }

    for (; j < 16; ++j) {
      alphas[j] = '_';
    }

    alphas[j] = 0;

    j = strlen(format);
    sprintf(format + j, "'%s'  *\n\r", alphas);

    kprintf(format, i + (intptr_t)memory,
      memory[i + 0], memory[i + 1], memory[i + 2], memory[i + 3], memory[i + 4], memory[i + 5], memory[i + 6], memory[i + 7],
      memory[i + 8], memory[i + 9], memory[i + 10], memory[i + 11], memory[i + 12], memory[i + 13], memory[i + 14], memory[i + 15]);

    format[j] = '\0';
  }
}

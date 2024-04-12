/*

  u8g_dev_ssd1309_128x64.c

  Universal 8bit Graphics Library

  Copyright (c) 2012, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list
    of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice, this
    list of conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "u8g.h"

#include <string.h>
#include <stdbool.h>

#define WIDTH 128
#define HEIGHT 64
#define PAGE_HEIGHT 8

// ssd1309 ini sequence
static const uint8_t u8g_dev_ssd1309_128x64_init_seq[] PROGMEM = {
  U8G_ESC_CS(0),      // disable chip
  U8G_ESC_ADR(0),     // instruction mode
  U8G_ESC_RST(1),     // do reset low pulse with (1*16)+2 milliseconds
  U8G_ESC_CS(1),      // enable chip

  0xFD, 0x12,         // Command Lock
  0xAE,               // Set Display Off
  0xD5, 0xA0,         // set Display Clock Divide Ratio/Oscillator Frequency
  0xA8, 0x3F,         // Set Multiplex Ratio
  0x3D, 0x00,         // Set Display Offset
  0x40,               // Set Display Start Line
  0xA1,               // Set Segment Re-Map
  0xC8,               // Set COM Output Scan Direction
  0xDA, 0x12,         // Set COM Pins Hardware Configuration
  0x81, 0xDF,         // Set Current Control
  0xD9, 0x82,         // Set Pre-Charge Period
  0xDB, 0x34,         // Set VCOMH Deselect Level
  0xA4,               // Set Entire Display On/Off
  0xA6,               // Set Normal/Inverse Display
  U8G_ESC_VCC(1),     // Power up VCC & Stabilized
  U8G_ESC_DLY(50),
  0xAF,               // Set Display On
  U8G_ESC_DLY(50),
  U8G_ESC_CS(0),      // disable chip
  U8G_ESC_END         // end of sequence
};

// select one init sequence here
#define u8g_dev_ssd1309_128x64_init_seq u8g_dev_ssd1309_128x64_init_seq

static const uint8_t u8g_dev_ssd1309_128x64_data_start[] PROGMEM = {
  U8G_ESC_ADR(0),     // instruction mode
  U8G_ESC_CS(1),      // enable chip
  0x10,               // set upper 4 bit of the col adr to 0
  0x00,               // set lower 4 bit of the col adr to 4
  U8G_ESC_END         // end of sequence
};

static const uint8_t u8g_dev_ssd13xx_sleep_on[] PROGMEM = {
  U8G_ESC_ADR(0),     // instruction mode
  U8G_ESC_CS(1),      // enable chip
  0xAE,               // display off
  U8G_ESC_CS(0),      // disable chip
  U8G_ESC_END         // end of sequence
};

static const uint8_t u8g_dev_ssd13xx_sleep_off[] PROGMEM = {
  U8G_ESC_ADR(0),     // instruction mode
  U8G_ESC_CS(1),      // enable chip
  0xAF,               // display on
  U8G_ESC_DLY(50),    // delay 50 ms
  U8G_ESC_CS(0),      // disable chip
  U8G_ESC_END         // end of sequence
};

uint8_t u8g_dev_ssd1309_128x64_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg) {
  switch (msg) {
    case U8G_DEV_MSG_INIT:
      u8g_InitCom(u8g, dev, U8G_SPI_CLK_CYCLE_300NS);
      u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd1309_128x64_init_seq);
      break;
    case U8G_DEV_MSG_STOP:
      break;
    case U8G_DEV_MSG_PAGE_NEXT: {
      u8g_pb_t *pb = (u8g_pb_t *)(dev->dev_mem);
      u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd1309_128x64_data_start);
      u8g_WriteByte(u8g, dev, 0xB0 | pb->p.page);   // select current page (SSD1306)
      u8g_SetAddress(u8g, dev, 1);                  // data mode
      if (u8g_pb_WriteBuffer(pb, u8g, dev) == 0) return 0;
      u8g_SetChipSelect(u8g, dev, 0);
    }
    break;
    case U8G_DEV_MSG_CONTRAST:
      u8g_SetChipSelect(u8g, dev, 1);
      u8g_SetAddress(u8g, dev, 0);                  // instruction mode
      u8g_WriteByte(u8g, dev, 0x81);
      u8g_WriteByte(u8g, dev, (*(uint8_t *)arg));   // 11 Jul 2015: fixed contrast calculation
      u8g_SetChipSelect(u8g, dev, 0);
      return 1;
    case U8G_DEV_MSG_SLEEP_ON:
      u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd13xx_sleep_on);
      return 1;
    case U8G_DEV_MSG_SLEEP_OFF:
      u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd13xx_sleep_off);
      return 1;
  }
  return u8g_dev_pb8v1_base_fn(u8g, dev, msg, arg);
}

#define PAGE_COUNT (HEIGHT / PAGE_HEIGHT)

/* The _f_ variant keeps an the contents of the full screen in an extra buffer,
 * and sends it all at once after all pages were drawn.
 * It also skips unchanged pages, and within those it skips unchanged columns at the beggining and end.
 * All of this improves fps by reducing both total transferred data and screen rerenders.
 * It requires an extra 1041 bytes of RAM.
 */
uint8_t u8g_dev_ssd1309_128x64_f_fn(u8g_t *u8g, u8g_dev_t *dev, uint8_t msg, void *arg) {
  if (msg == U8G_DEV_MSG_PAGE_NEXT) {
    u8g_pb_t *pb = (u8g_pb_t *)(dev->dev_mem);
    uint8_t *buf = (uint8_t *)pb->buf;
    uint8_t page = pb->p.page;
    static uint8_t full_buffer[PAGE_COUNT][WIDTH] U8G_NOCOMMON;
    static uint8_t column_start[PAGE_COUNT] = {},
                   column_count[PAGE_COUNT] = {};

    uint8_t start = 0;
    while (start < WIDTH && full_buffer[page][start] == buf[start]) start++;
    column_start[page] = start;

    const bool page_changed = start < WIDTH;
    if (page_changed) {
      uint8_t end = WIDTH - 1;
      while (end > start && full_buffer[page][end] == buf[end]) end--;
      const uint8_t count = column_count[page] = end - start + 1;
      memcpy(full_buffer[page] + start, buf + start, count);
    }

    // Only send buffer after the last page has been cached
    const bool is_last_page = page == PAGE_COUNT - 1;
    if (is_last_page) {
      // Full render to clear noise in LCD RAM on start
      static bool has_rendered = false;
      for (uint8_t page = 0; page < PAGE_COUNT; page++) {
        const uint8_t start = has_rendered ? column_start[page] : 0,
                      count = has_rendered ? column_count[page] : WIDTH;
        const bool page_changed = start < WIDTH;
        if (page_changed) {
          u8g_WriteEscSeqP(u8g, dev, u8g_dev_ssd1309_128x64_data_start);
          u8g_WriteByte(u8g, dev, 0xB0 | page);               // Select current page (SSD1306)
          if (start > 0) {
            u8g_WriteByte(u8g, dev, 0x00 | (start & 0b1111)); // Start column low nybble
            u8g_WriteByte(u8g, dev, 0x10 | (start >> 4));     // Start column high nybble
          }
          u8g_SetAddress(u8g, dev, 1);                        // Data mode
          u8g_WriteSequence(u8g, dev, count, full_buffer[page] + start);
        }
      }
      u8g_SetChipSelect(u8g, dev, 0);
      has_rendered = true;
    }
    return u8g_dev_pb8v1_base_fn(u8g, dev, msg, arg);
  }
  return u8g_dev_ssd1309_128x64_fn(u8g, dev, msg, arg);
}

U8G_PB_DEV(u8g_dev_ssd1309_128x64_hw_spi, WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_ssd1309_128x64_fn, U8G_COM_HW_SPI);
U8G_PB_DEV(u8g_dev_ssd1309_128x64_sw_spi, WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_ssd1309_128x64_fn, U8G_COM_SW_SPI);
U8G_PB_DEV(u8g_dev_ssd1309_128x64_i2c, WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_ssd1309_128x64_fn, U8G_COM_SSD_I2C);

U8G_PB_DEV(u8g_dev_ssd1309_128x64_f_hw_spi, WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_ssd1309_128x64_f_fn, U8G_COM_HW_SPI);
U8G_PB_DEV(u8g_dev_ssd1309_128x64_f_sw_spi, WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_ssd1309_128x64_f_fn, U8G_COM_SW_SPI);
U8G_PB_DEV(u8g_dev_ssd1309_128x64_f_i2c, WIDTH, HEIGHT, PAGE_HEIGHT, u8g_dev_ssd1309_128x64_f_fn, U8G_COM_SSD_I2C);

/*
  main.cpp = SH1106
  FontUsage.ino
  GraphicsTest.ino
  PrintUTF8.ino

  How to overwrite previous text with a new text.
  How to avoid the buffer clear command.

  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Use the (Arduino compatible) u8g2 function "print"  to draw a text.

  =======================================================================================
  Copyright (c) 2016, olikraus@gmail.com
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

  Usually, a sequence like this is used:
  u8g2->clearBuffer();                  // clear the internal memory
  u8g2->setFont(u8g2_font_ncenB08_tr);  // choose a suitable font
  u8g2->setCursor(0,20)                 // set write position
  u8g2->print("Hello World!");          // write something to the internal memory
  u8g2->sendBuffer();                   // transfer internal memory to the display

  In order to speed up the display content rendering for any modified text,
  we could drop the clearBuffer command and just overwrite the previous string:

  u8g2->setFont(u8g2_font_ncenB08_tr);  // choose a suitable font
  u8g2->setCursor(0,20)                 // set write position
  u8g2->print("hello there");           // write something to the internal memory
  u8g2->sendBuffer();                   // transfer internal memory to the display

  This will only work if:
  - Background is drawn
  - All gylphs have the same height
  Additionally some extra spaces are required of the new text is shorter than the
  previous one.

  To draw the background: use font mode 0
  To ensure that all glyphs have the same height, use h or m fonts.

  u8g2->setFontMode(0);                 // write solid glyphs
  u8g2->setFont(u8g2_font_ncenB08_hr);  // choose a suitable h font
  u8g2->setCursor(0,20)                 // set write position
  u8g2->print("hello there  ");         // use extra spaces here
  u8g2->sendBuffer();                   // transfer internal memory to the display
*/

#include <Arduino.h>
#include "LoRaBoards.h"

#define INFO_SCREEN_DELAY 5000

/*
  Linear Congruential Generator (LCG)
  z = (a*z + c) % m;
  m = 256 (8 Bit)

  for period:
  a-1: dividable by 2
  a-1: multiple of 4
  c: not dividable by 2

  c = 17
  a-1 = 64 --> a = 65
*/
uint8_t z = 127;    // start value
uint32_t lcg_rnd(void)
{
    z = (uint8_t)((uint16_t)65 * (uint16_t)z + (uint16_t)17);
    return (uint32_t)z;
}

void u8g2_prepare(void)
{
    u8g2->setFont(u8g2_font_6x10_tf);
    u8g2->setFontRefHeightExtendedText();
    u8g2->setDrawColor(1);
    u8g2->setFontPosTop();
    u8g2->setFontDirection(0);
}

void u8g2_box_frame(uint8_t a)
{
    u8g2->drawStr( 0, 0, "drawBox");
    u8g2->drawBox(5, 10, 20, 10);
    u8g2->drawBox(10 + a, 15, 30, 7);
    u8g2->drawStr( 0, 30, "drawFrame");
    u8g2->drawFrame(5, 10 + 30, 20, 10);
    u8g2->drawFrame(10 + a, 15 + 30, 30, 7);
}

void u8g2_disc_circle(uint8_t a)
{
    u8g2->drawStr( 0, 0, "drawDisc");
    u8g2->drawDisc(10, 18, 9);
    u8g2->drawDisc(24 + a, 16, 7);
    u8g2->drawStr( 0, 30, "drawCircle");
    u8g2->drawCircle(10, 18 + 30, 9);
    u8g2->drawCircle(24 + a, 16 + 30, 7);
}

void u8g2_r_frame(uint8_t a)
{
    u8g2->drawStr( 0, 0, "drawRFrame/Box");
    u8g2->drawRFrame(5, 10, 40, 30, a + 1);
    u8g2->drawRBox(50, 10, 25, 40, a + 1);
}

void u8g2_string(uint8_t a)
{
    u8g2->setFontDirection(0);
    u8g2->drawStr(30 + a, 31, " 0");
    u8g2->setFontDirection(1);
    u8g2->drawStr(30, 31 + a, " 90");
    u8g2->setFontDirection(2);
    u8g2->drawStr(30 - a, 31, " 180");
    u8g2->setFontDirection(3);
    u8g2->drawStr(30, 31 - a, " 270");
}

void u8g2_line(uint8_t a)
{
    u8g2->drawStr( 0, 0, "drawLine");
    u8g2->drawLine(7 + a, 10, 40, 55);
    u8g2->drawLine(7 + a * 2, 10, 60, 55);
    u8g2->drawLine(7 + a * 3, 10, 80, 55);
    u8g2->drawLine(7 + a * 4, 10, 100, 55);
}

void u8g2_triangle(uint8_t a)
{
    uint16_t offset = a;
    u8g2->drawStr( 0, 0, "drawTriangle");
    u8g2->drawTriangle(14, 7, 45, 30, 10, 40);
    u8g2->drawTriangle(14 + offset, 7 - offset, 45 + offset, 30 - offset, 57 + offset, 10 - offset);
    u8g2->drawTriangle(57 + offset * 2, 10, 45 + offset * 2, 30, 86 + offset * 2, 53);
    u8g2->drawTriangle(10 + offset, 40 + offset, 45 + offset, 30 + offset, 86 + offset, 53 + offset);
}

void u8g2_ascii_1()
{
    char s[2] = " ";
    uint8_t x, y;
    u8g2->drawStr( 0, 0, "ASCII page 1");
    for ( y = 0; y < 6; y++ ) {
        for ( x = 0; x < 16; x++ ) {
            s[0] = y * 16 + x + 32;
            u8g2->drawStr(x * 7, y * 10 + 10, s);
        }
    }
}

void u8g2_ascii_2()
{
    char s[2] = " ";
    uint8_t x, y;
    u8g2->drawStr( 0, 0, "ASCII page 2");
    for ( y = 0; y < 6; y++ ) {
        for ( x = 0; x < 16; x++ ) {
            s[0] = y * 16 + x + 160;
            u8g2->drawStr(x * 7, y * 10 + 10, s);
        }
    }
}

void u8g2_extra_page(uint8_t a)
{
    u8g2->drawStr( 0, 0, "Unicode");
    u8g2->setFont(u8g2_font_unifont_t_symbols);
    u8g2->setFontPosTop();
    u8g2->drawUTF8(0, 24, "☀ ☁");
    switch (a) {
    case 0:
    case 1:
    case 2:
    case 3:
        u8g2->drawUTF8(a * 3, 36, "☂");
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        u8g2->drawUTF8(a * 3, 36, "☔");
        break;
    }
}

#define cross_width 24
#define cross_height 24
static const unsigned char cross_bits[] U8X8_PROGMEM  = {
    0x00, 0x18, 0x00, 0x00, 0x24, 0x00, 0x00, 0x24, 0x00, 0x00, 0x42, 0x00,
    0x00, 0x42, 0x00, 0x00, 0x42, 0x00, 0x00, 0x81, 0x00, 0x00, 0x81, 0x00,
    0xC0, 0x00, 0x03, 0x38, 0x3C, 0x1C, 0x06, 0x42, 0x60, 0x01, 0x42, 0x80,
    0x01, 0x42, 0x80, 0x06, 0x42, 0x60, 0x38, 0x3C, 0x1C, 0xC0, 0x00, 0x03,
    0x00, 0x81, 0x00, 0x00, 0x81, 0x00, 0x00, 0x42, 0x00, 0x00, 0x42, 0x00,
    0x00, 0x42, 0x00, 0x00, 0x24, 0x00, 0x00, 0x24, 0x00, 0x00, 0x18, 0x00,
};

#define cross_fill_width 24
#define cross_fill_height 24
static const unsigned char cross_fill_bits[] U8X8_PROGMEM  = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x18, 0x64, 0x00, 0x26,
    0x84, 0x00, 0x21, 0x08, 0x81, 0x10, 0x08, 0x42, 0x10, 0x10, 0x3C, 0x08,
    0x20, 0x00, 0x04, 0x40, 0x00, 0x02, 0x80, 0x00, 0x01, 0x80, 0x18, 0x01,
    0x80, 0x18, 0x01, 0x80, 0x00, 0x01, 0x40, 0x00, 0x02, 0x20, 0x00, 0x04,
    0x10, 0x3C, 0x08, 0x08, 0x42, 0x10, 0x08, 0x81, 0x10, 0x84, 0x00, 0x21,
    0x64, 0x00, 0x26, 0x18, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#define cross_block_width 14
#define cross_block_height 14
static const unsigned char cross_block_bits[] U8X8_PROGMEM  = {
    0xFF, 0x3F, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20,
    0xC1, 0x20, 0xC1, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20, 0x01, 0x20,
    0x01, 0x20, 0xFF, 0x3F,
};

void u8g2_bitmap_overlay(uint8_t a)
{
    uint8_t frame_size = 28;

    u8g2->drawStr(0, 0, "Bitmap overlay");

    u8g2->drawStr(0, frame_size + 12, "Solid / transparent");
    u8g2->setBitmapMode(false /* solid */);
    u8g2->drawFrame(0, 10, frame_size, frame_size);
    u8g2->drawXBMP(2, 12, cross_width, cross_height, cross_bits);
    if (a & 4)
        u8g2->drawXBMP(7, 17, cross_block_width, cross_block_height, cross_block_bits);

    u8g2->setBitmapMode(true /* transparent*/);
    u8g2->drawFrame(frame_size + 5, 10, frame_size, frame_size);
    u8g2->drawXBMP(frame_size + 7, 12, cross_width, cross_height, cross_bits);
    if (a & 4)
        u8g2->drawXBMP(frame_size + 12, 17, cross_block_width, cross_block_height, cross_block_bits);
}

void u8g2_bitmap_modes(uint8_t transparent)
{
    const uint8_t frame_size = 24;

    u8g2->drawBox(0, frame_size * 0.5, frame_size * 5, frame_size);
    u8g2->drawStr(frame_size * 0.5, 50, "Black");
    u8g2->drawStr(frame_size * 2, 50, "White");
    u8g2->drawStr(frame_size * 3.5, 50, "XOR");

    if (!transparent) {
        u8g2->setBitmapMode(false /* solid */);
        u8g2->drawStr(0, 0, "Solid bitmap");
    } else {
        u8g2->setBitmapMode(true /* transparent*/);
        u8g2->drawStr(0, 0, "Transparent bitmap");
    }
    u8g2->setDrawColor(0);// Black
    u8g2->drawXBMP(frame_size * 0.5, 24, cross_width, cross_height, cross_bits);
    u8g2->setDrawColor(1); // White
    u8g2->drawXBMP(frame_size * 2, 24, cross_width, cross_height, cross_bits);
    u8g2->setDrawColor(2); // XOR
    u8g2->drawXBMP(frame_size * 3.5, 24, cross_width, cross_height, cross_bits);
}

void draw(int is_blank)
{
    int i, j;
    int n;
    char s[4];

    for ( j = 0; j < 20; j++ ) {
        #ifdef BOARD_LED
        flashLed();
        #endif
        // random number
        n = lcg_rnd();

        // random string
        for ( i = 0; i < 3; i++ ) {
            s[i] = lcg_rnd() >> 3;
            if ( s[i] < 16 )
                s[i] += 'a';
            else
                s[i] += 'A';
        }
        s[3] = '\0';

        // print number
        u8g2->setCursor(0, 15);  u8g2->print("Number: ");
        if ( is_blank )          u8g2->print("       ");
        u8g2->setCursor(70, 15); u8g2->print(n);

        // print string
        u8g2->setCursor(0, 30);  u8g2->print("Text: ");
        u8g2->setCursor(70, 30); u8g2->print(s);
        if ( is_blank )          u8g2->print("        ");

        u8g2->sendBuffer();      // make the result visible

        delay(200);              // delay, so that the user can see the result
    }
}

void draw_m1_t()
{
    u8g2->clearBuffer();
    u8g2->setFontMode(1);   u8g2->setFont(u8g2_font_cu12_tr);

    u8g2->setCursor(0, 15); u8g2->print(F("setFontMode(1);"));
    u8g2->setCursor(0, 30); u8g2->print(F("setFont(cu12_tr);"));
    u8g2->setCursor(0, 55); u8g2->print(F("sehr schlecht"));

    u8g2->sendBuffer();     delay(INFO_SCREEN_DELAY);

    u8g2->setFontMode(1);   u8g2->setFont(u8g2_font_cu12_tr);
    u8g2->clearBuffer();    // clear the internal memory once
    draw(0);
}

void draw_m0_t()
{
    u8g2->clearBuffer();
    u8g2->setFontMode(0);   u8g2->setFont(u8g2_font_cu12_tr);

    u8g2->setCursor(0, 15); u8g2->print(F("setFontMode(0);"));
    u8g2->setCursor(0, 30); u8g2->print(F("setFont(cu12_tr);"));
    u8g2->setCursor(0, 55); u8g2->print(F("falsch"));

    u8g2->sendBuffer();     delay(INFO_SCREEN_DELAY);

    u8g2->setFontMode(0);   u8g2->setFont(u8g2_font_cu12_tr);
    u8g2->clearBuffer();    // clear the internal memory once
    draw(0);
}

void draw_m1_h()
{
    u8g2->clearBuffer();
    u8g2->setFontMode(1);   u8g2->setFont(u8g2_font_cu12_hr);

    u8g2->setCursor(0, 15); u8g2->print(F("setFontMode(1);"));
    u8g2->setCursor(0, 30); u8g2->print(F("setFont(cu12_hr);"));
    u8g2->setCursor(0, 55); u8g2->print(F("immer noch schlecht"));

    u8g2->sendBuffer();     delay(INFO_SCREEN_DELAY);

    u8g2->setFontMode(1);   u8g2->setFont(u8g2_font_cu12_hr);
    u8g2->clearBuffer();    // clear the internal memory once
    draw(0);
}

void draw_m0_h()
{
    u8g2->clearBuffer();
    u8g2->setFontMode(0);   u8g2->setFont(u8g2_font_cu12_hr);

    u8g2->setCursor(0, 15); u8g2->print(F("setFontMode(0);"));
    u8g2->setCursor(0, 30); u8g2->print(F("setFont(cu12_hr);"));
    u8g2->setCursor(0, 55); u8g2->print(F("fast ok"));

    u8g2->sendBuffer();     delay(INFO_SCREEN_DELAY);

    u8g2->setFontMode(0);   u8g2->setFont(u8g2_font_cu12_hr);
    u8g2->clearBuffer();    // clear the internal memory once
    draw(0);
}

void draw_m0_h_with_extra_blank()
{
    u8g2->clearBuffer();
    u8g2->setFontMode(0);   u8g2->setFont(u8g2_font_cu12_hr);

    u8g2->setCursor(0, 15); u8g2->print(F("setFontMode(0);"));
    u8g2->setCursor(0, 30); u8g2->print(F("setFont(cu12_hr);"));
    u8g2->setCursor(0, 55); u8g2->print(F("Extra blank --> Ok"));

    u8g2->sendBuffer();     delay(INFO_SCREEN_DELAY);

    u8g2->setFontMode(0);   u8g2->setFont(u8g2_font_cu12_hr);
    u8g2->clearBuffer();    // clear the internal memory once
    draw(1);                // + extra blank
}

uint8_t draw_state = 0;

void draw_graphics(void)
{
    u8g2_prepare();
    switch (draw_state >> 3) {
    case 0: u8g2_box_frame(draw_state & 7); break;
    case 1: u8g2_disc_circle(draw_state & 7); break;
    case 2: u8g2_r_frame(draw_state & 7); break;
    case 3: u8g2_string(draw_state & 7); break;
    case 4: u8g2_line(draw_state & 7); break;
    case 5: u8g2_triangle(draw_state & 7); break;
    case 6: u8g2_ascii_1(); break;
    case 7: u8g2_ascii_2(); break;
    case 8: u8g2_extra_page(draw_state & 7); break;
    case 9: u8g2_bitmap_modes(0); break;
    case 10: u8g2_bitmap_modes(1); break;
    case 11: u8g2_bitmap_overlay(draw_state & 7); break;
    }
}


void setup(void)
{
    setupBoards();
    if (!u8g2) {
        Serial.println("No SH1106 display found! Please check whether the connection is normal");
        while (1);
    }
    #ifdef BOARD_LED
    digitalWrite(BOARD_LED, LED_OFF);
    #endif
}





void loop(void)
{
    // This problem applies only to full buffer mode
    u8g2->disableUTF8Print();
    u8g2->clearBuffer();
    u8g2->setFontMode(1);   u8g2->setFont(u8g2_font_cu12_tr);
    u8g2->setCursor(0, 15); u8g2->print(F("Problem mit"));
    u8g2->setCursor(0, 30); u8g2->print(F("full buffer mode"));
    u8g2->setCursor(0, 45); u8g2->print(F("und fehlendem clear"));
    u8g2->sendBuffer();     delay(INFO_SCREEN_DELAY);


    draw_m1_t();          // fontmode 1, t font --> wrong
    draw_m1_h();          // fontmode 1, h font --> wrong
    draw_m0_t();          // fontmode 0, t font --> wrong
    draw_m0_h();          // fontmode 1, h font --> fast ok
    draw_m0_h_with_extra_blank(); // fontmode 1, h font with extra blank --> correct
    delay(5000);


    for (int i=0; i <=(12*8); i++) {   // picture loop
        draw_state = i;
        u8g2->clearBuffer();
        draw_graphics();
        u8g2->sendBuffer();
        delay(400);  // delay between each page
    }
    delay(5000);

    u8g2->enableUTF8Print();       // enable UTF8 support for the Arduino print() function
    u8g2->setFont(u8g2_font_unifont_t_chinese2);  // use chinese2 for all the glyphs of "你好世界"
    u8g2->setFontDirection(0);
    u8g2->clearBuffer();
    u8g2->setCursor(0, 5);    u8g2->print("UTF8Print()");
    u8g2->setCursor(0, 20);   u8g2->print("Hello World!");
    u8g2->setCursor(0, 45);   u8g2->print("你好世界");       // Chinese "Hello World"
    u8g2->sendBuffer();

    delay(5000);

}

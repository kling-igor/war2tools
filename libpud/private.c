/*
 * private.c
 * libpud
 *
 * Copyright (c) 2014 Jean Guyomarc'h
 */

#include "pud_private.h"

const char *
long2bin(uint32_t x)
{
   static char buf[64];
   char b[16];
   int i, k = 0;
   const char *q;

   snprintf(b, sizeof(b), "%08x", x);
   for (i = 0; i < 8; i++)
     {
        switch (b[i])
          {
           case '0': q = "0000"; break;
           case '1': q = "0001"; break;
           case '2': q = "0010"; break;
           case '3': q = "0011"; break;
           case '4': q = "0100"; break;
           case '5': q = "0101"; break;
           case '6': q = "0110"; break;
           case '7': q = "0111"; break;
           case '8': q = "1000"; break;
           case '9': q = "1001"; break;
           case 'a': q = "1010"; break;
           case 'b': q = "1011"; break;
           case 'c': q = "1100"; break;
           case 'd': q = "1101"; break;
           case 'e': q = "1110"; break;
           case 'f': q = "1111"; break;
           default: return "<ERROR>";
          }
        memcpy(buf + k, q, 4);
        k += 4;
     }
   buf[k] = 0;

   return buf;
}

const char *
mode2str(Pud_Open_Mode mode)
{
   if (mode & PUD_OPEN_MODE_R)
     {
        if (mode & PUD_OPEN_MODE_W)
          return "rb+"; /* RW */
        else /* RO */
          return "rb";
     }
   else if (mode & PUD_OPEN_MODE_W)
     return "wb+";

   /* Should not happen... */
   return "rb";
}

Pud_Color
color_make(uint8_t r,
           uint8_t g,
           uint8_t b,
           uint8_t a)
{
   Pud_Color c = {
      .r = r,
      .g = g,
      .b = b,
      .a = a
   };
   return c;
}

const char *
dim2str(Pud_Dimensions dim)
{
   switch (dim)
     {
      case PUD_DIMENSIONS_32_32:   return "32 x 32";
      case PUD_DIMENSIONS_64_64:   return "64 x 64";
      case PUD_DIMENSIONS_96_96:   return "96 x 96";
      case PUD_DIMENSIONS_128_128: return "128 x 128";
      default: break;
     }
   return "<INVALID DIMENSIONS>";
}

Pud_Color
color_for_player(uint8_t player)
{
   switch (player)
     {
      case PUD_PLAYER_RED:    return color_make(0xc0, 0x00, 0x00, 0xff); // Red
      case PUD_PLAYER_BLUE:   return color_make(0x00, 0x00, 0xc0, 0xff); // Blue
      case PUD_PLAYER_GREEN:  return color_make(0x00, 0xff, 0x00, 0xff); // Green
      case PUD_PLAYER_VIOLET: return color_make(0x80, 0x00, 0xc0, 0xff); // Violet
      case PUD_PLAYER_ORANGE: return color_make(0xff, 0x80, 0x00, 0xff); // Orange
      case PUD_PLAYER_BLACK:  return color_make(0x00, 0x00, 0x00, 0xff); // Black
      case PUD_PLAYER_WHITE:  return color_make(0xff, 0xff, 0xff, 0xff); // White
      case PUD_PLAYER_YELLOW: return color_make(0xff, 0xd0, 0x00, 0xff); // Yellow
      default: ERR("Invalid player [%i]", player); break;
     }

   return color_make(0x7f, 0x7f, 0x7f, 0xff);
}

Pud_Bool
pud_mem_map_ok(Pud *pud)
{
   return (pud->ptr < pud->mem_map + pud->mem_map_size);
}


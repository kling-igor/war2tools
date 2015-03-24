/*
 * tileset.c
 * libwar2
 *
 * Copyright (c) 2014 Jean Guyomarc'h
 */

#include "war2_private.h"

static Pud_Bool
_ts_entries_parse(War2_Data                *w2,
                  War2_Tileset_Descriptor  *ts,
                  const unsigned int       *entries,
                  War2_Tileset_Decode_Func  func)
{
   /* Lookup table (flip table): 0=>7, 1=>6, 2=>5, ... 7=>0
    * Thanks wargus for the tip. */
   const int ft[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };

   unsigned char *ptr, *data, *map;
   size_t size, i, map_size;
   Pud_Bool flip_x, flip_y;
   int o, x, y, j, i_img;
   Pud_Color img[1024];
   uint8_t chunk[32];
   uint16_t w;
   int img_ctr = 0;
   unsigned char col;

   /* If no callback has been specified, do nothing */
   if (!func)
     {
        WAR2_VERBOSE(w2, 1, "Warning: No callback specified.");
        return PUD_TRUE;
     }

   /* Extract palette - 256x3 */
   ptr = war2_palette_extract(w2, entries[0]);
   if (!ptr) DIE_RETURN(PUD_FALSE, "Failed to get palette");
   war2_palette_convert(ptr, ts->palette);
   free(ptr);

   /* Get minitiles info */
   ptr = war2_entry_extract(w2, entries[1], &size);
   if (!ptr)
     DIE_RETURN(PUD_FALSE, "Failed to extract entry minitile info [%i]", entries[1]);
   data = war2_entry_extract(w2, entries[2], NULL);
   if (!data)
     {
        free(ptr);
        DIE_RETURN(PUD_FALSE, "Failed to extract entry minitile data [%i]", entries[2]);
     }
   map = war2_entry_extract(w2, entries[3], &map_size);
   if (!map)
     {
        free(ptr);
        free(data);
        DIE_RETURN(PUD_FALSE, "Failed to extract entry map [%i]", entries[3]);
     }
   ts->tiles = size / 32;

   /* This entry contains the offsets for a given tile 0x????
    * Each chunk is 42 bytes: 32 used and 10 unused */
   for (i = 0; i < map_size; i += 42)
     {
        /* Read the first 16 words (32 bytes) */
        memcpy(&(chunk[0]), &(map[i]), 32);
        for (j = 0; j < 32; j += 2)
          {
             memcpy(&w, &(chunk[j]), sizeof(uint16_t));
             //printf("Chunk [0x%04lx]: 0x%04x\n", i / 42, w);

             /* If one of the words is 0, the tile is unused */
             if (w == 0x0000)
               {
                  break;
               }
          }
     }

   // FIXME Fog of war (16 first tiles) */

   /* Jump by blocks of 16 words */
   for (i = 0, img_ctr = 1; i < size; i += 32, img_ctr++)
     {
        /* For each word in the block of 16 */
        for (j = 0, i_img = 0; j < 32; j += 2, i_img++)
          {
             /* Get offset and flips */
             memcpy(&o, &(ptr[i + j]), sizeof(uint16_t));
             flip_x = o & 2; // 0b10
             flip_y = o & 1; // 0b01
             o = (o & 0xfffc) * 16;

             /* Decode a minitile (8x8) */
             for (y = 0; y < 8; y++)
               {
                  for (x = 0; x < 8; x++)
                    {
                       /* If flip_x/flip_y are PUD_TRUE, the minitile must be flipped on
                        * its x/y axis. We use a flip table which avoids calculations
                        * to do so. */
                       col = data[o + ((flip_x ? ft[x] : x) + (flip_y ? ft[y] : y) * 8)];

                       /* Maths: we have 16 blocks of 8x8 to place in a 32x32
                        * image which has a linear memory layout */
                       const int xblock = x + ((i_img % 4) * 8);
                       const int yblock = y + ((i_img / 4) * 8);

                       /* Convert the byte to color thanks to the palette */
                       img[xblock + 32 * yblock] = ts->palette[col];
                    }
               }
          }

        func(img, 32, 32, ts, img_ctr);
     }

   free(ptr);
   free(data);
   free(map);

   return PUD_TRUE;
}

War2_Tileset_Descriptor *
war2_tileset_decode(War2_Data                *w2,
                    Pud_Era                   era,
                    War2_Tileset_Decode_Func  func)
{
   const unsigned int forest[] = { 2, 3, 4, 5, 6, 7, 8 };
   const unsigned int wasteland[] = { 10, 11, 12, 13, 14, 15, 16 };
   const unsigned int winter[] = { 18, 19, 20, 21, 22, 23, 24 };
   const unsigned int swamp[] = { 438, 439, 440, 441, 442, 443, 444 };
   const unsigned int *entries;
   War2_Tileset_Descriptor *ts;

   /* Alloc */
   ts = calloc(1, sizeof(War2_Tileset_Descriptor));
   if (!ts) DIE_RETURN(NULL, "Failed to allocate memory");
   ts->era = era;

   switch (era)
     {
      case PUD_ERA_FOREST:    entries = forest;    break;
      case PUD_ERA_WASTELAND: entries = wasteland; break;
      case PUD_ERA_WINTER:    entries = winter;    break;
      case PUD_ERA_SWAMP:     entries = swamp;     break;
     }

   _ts_entries_parse(w2, ts, entries, func);

   return ts;
}

void
war2_tileset_descriptor_free(War2_Tileset_Descriptor *ts)
{
   free(ts);
}


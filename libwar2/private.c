/*
 * Copyright (c) 2014-2016 Jean Guyomarc'h
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "war2_private.h"

PUDAPI_INTERNAL void
war2_palette_convert(const unsigned char *ptr,
                     Pud_Color            palette[256])
{
   const unsigned char *p;
   unsigned int i;

   /* I don't know why this is the bitshift needed (no doc so no explaination)
    * but this gives the right colorspace (thanks wargus) */
   for (i = 0; i < 256; ++i)
     {
        p = &(ptr[i * 3]);
        palette[i].r = p[0] << 2;
        palette[i].g = p[1] << 2;
        palette[i].b = p[2] << 2;
        palette[i].a = 0xff;
     }
}

/*
   Copyright (C) Prikol Software 1996-1997
   Copyright (C) Aleksey Volynskov 1996-1997
   Copyright (C) <ARembo@gmail.com> 2011

   This file is part of the Doom2D:Rembo project.

   Doom2D:Rembo is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 as
   published by the Free Software Foundation.

   Doom2D:Rembo is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/> or
   write to the Free Software Foundation, Inc.,
   51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
*/

#include "vga.h"
#include "error.h"
#include "glob.h"
#include "view.h"
#include <3ds.h>

// адрес экранного буфера
unsigned char *scra;

// виртуальный экран
unsigned char scrbuf[64000];

// virtual palette because i'm a lazy fuck
static vgapal curpal;

int SCRW = 400;
int SCRH = 240;

u8 *framebuffer = NULL;
u8 *screen = NULL;
u8 *vgascreen = NULL; // massive hack

int cx1, cx2, cy1, cy2;

char fullscreen = OFF;

short V_init(void) {
  u16 w, h;
  framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &w, &h);
  // 3ds screens are actually rotated 90 degrees
  SCRH = w;
  SCRW = h;
  logo("   got 3DS top screen FB: %dx%d\n", SCRW, SCRH);
  vgascreen = malloc(SCRW * SCRH);
  screen = malloc(SCRW * SCRH * 3);
  return 0;
}

// переключение в текстовый режим
void V_done(void) {
  free(vgascreen);
  free(screen);
}

// установить область вывода
void V_setrect(short x, short w, short y, short h) {
  cx1 = x;
  cx2 = x + w - 1;
  cy1 = y;
  cy2 = y + h - 1;
  if (cx1 < 0)
    cx1 = 0;
  if (cx2 >= SCRW)
    cx2 = SCRW - 1;
  if (cy1 < 0)
    cy1 = 0;
  if (cy2 >= SCRH)
    cy2 = SCRH - 1;
}

void putpixel(int x, int y, u8 color) {
  if (x >= cx1 && x <= cx2 && y >= cy1 && y <= cy2) {
    y = SCRH - y;
    u8 *p = screen + (x * SCRH + y) * 3;
    *(p + 0) = curpal[color].b;
    *(p + 1) = curpal[color].g;
    *(p + 2) = curpal[color].r;
    p = vgascreen + x * SCRH + y;
    *p = color;
  }
}

void draw_rect(int sx, int sy, int w, int h, int c) {
  for (int y = sy; y < sy + h; ++y)
    for (int x = sx; x < sx + w; ++x)
      putpixel(x, y, c);
}

byte getpixel(int x, int y) {
  if (x >= cx1 && x <= cx2 && y >= cy1 && y <= cy2) {
    y = SCRH - y;
    u8 *p = vgascreen + x * SCRH + y;
    return *p;
  }
  return 0;
}

void mappixel(int x, int y, byte *cmap) {
  byte c = getpixel(x, y);
  putpixel(x, y, cmap[c]);
}

int offx = 0;
int offy = 0;

void V_center(int f) {
  if (f)
    V_offset(SCRW / 2 - 320 / 2, SCRH / 2 - 200 / 2);
  else
    V_offset(0, 0);
}

void V_offset(int ox, int oy) {
  offx = ox;
  offy = oy;
}

void draw_spr(short x, short y, vgaimg *i, int d, int c) {
  if (i == NULL)
    return;
  x += offx;
  y += offy;
  if (d & 1)
    x = x - i->w + i->sx;
  else
    x -= i->sx;
  if (d & 2)
    y = y - i->h + i->sy;
  else
    y -= i->sy;
  if (x + i->w >= cx1 && x <= cx2 && y + i->h >= cy1 && y <= cy2) {
    int lx, ly;
    byte *p = (byte *)i + sizeof(vgaimg);
    for (ly = 0; ly < i->h; ly++) {
      for (lx = 0; lx < i->w; lx++) {
        int rx, ry;
        rx = (d & 1) ? (i->w - lx - 1) : (rx = lx);
        ry = (d & 2) ? (i->h - ly - 1) : (ry = ly);
        if (*p) {
          byte t = *p;
          if (c)
            if (t >= 0x70 && t <= 0x7F)
              t = t - 0x70 + c;
          putpixel(x + rx, y + ry, t);
        }
        p++;
      }
    }
  }
}

void V_rotspr(int x, int y, vgaimg *i, int d) {
  x += i->w * ((d & 1) ? 1 : 0);
  y += i->h * ((d & 2) ? 1 : 0);
  draw_spr(x, y, i, d, 0);
}

void V_pic(short x, short y, vgaimg *i) { draw_spr(x, y, i, 0, 0); }

void V_manspr(int x, int y, void *p, unsigned char c) {
  draw_spr(x, y, p, 0, c);
}

void V_manspr2(int x, int y, void *p, unsigned char c) {
  draw_spr(x, y, p, 1, c);
}

// вывести точку цвета c в координатах (x,y)
void V_dot(short x, short y, unsigned char c) { putpixel(x, y, c); }

extern byte bright[256];
extern byte flametab[16];
extern byte mixmap[256][256];

void smoke_sprf(int x, int y, byte c) {
  byte t = getpixel(x, y);
  c = c + bright[t];
  c += 0x60;
  c ^= 0xF;
  putpixel(x, y, mixmap[c][t]);
}

void flame_sprf(int x, int y, byte c) {
  byte t = getpixel(x, y);
  c = c + bright[t];
  putpixel(x, y, flametab[c]);
}

void V_sprf(short x, short y, vgaimg *i, spr_f *f) {
  if (i == NULL)
    return;
  x -= i->sx;
  y -= i->sy;
  int cx, cy;
  byte *p = (byte *)i;
  p += sizeof(vgaimg);
  for (cy = y; cy < y + i->h; cy++) {
    for (cx = x; cx < x + i->w; cx++) {
      if (*p) {
        (*f)(cx, cy, *p);
      }
      p++;
    }
  }
}

void V_spr(short x, short y, vgaimg *i) { draw_spr(x, y, i, 0, 0); }

void V_spr2(short x, short y, vgaimg *i) { draw_spr(x, y, i, 1, 0); }

void V_clr(short x, short w, short y, short h, unsigned char c) {
  draw_rect(x, y, w, h, c);
}

// установить палитру из массива p
void VP_setall(void *p) { VP_set(p, 0, 256); }

// установить n цветов, начиная с f, из массива p
void VP_set(void *p, short f, short n) {
  byte *ptr = (byte *)p;
  for (int i = f; i < f + n; i++) {
    curpal[i].r = ptr[0] * 4;
    curpal[i].g = ptr[1] * 4;
    curpal[i].b = ptr[2] * 4;
    ptr += 3;
  }
}

void V_flush(void) {
  if (screen) {
    memcpy(framebuffer, screen, SCRW * SCRH * 3);
  }
}

// установить адрес экранного буфера
// NULL - реальный экран
void V_setscr(void *p) {
  // FIXME: think of how to do this shit
}

// скопировать прямоугольник на экран
void V_copytoscr(short x, short w, short y, short h) {
  // FIXME: think of how to do this shit
}

void V_maptoscr(int x, int w, int y, int h, void *cmap) {
  int cx, cy;
  for (cx = x; cx < x + w; cx++)
    for (cy = y; cy < y + h; cy++)
      mappixel(cx, cy, (byte *)cmap);
  V_copytoscr(x, w, y, h);
}

void V_remap_rect(int x, int y, int w, int h, byte *cmap) {
  int cx, cy;
  for (cx = x; cx < x + w; cx++)
    for (cy = y; cy < y + h; cy++)
      mappixel(cx, cy, cmap);
}

extern void *walp[256];
extern byte clrmap[256 * 12];

void Z_drawfld(byte *fld, int bg) {
  byte *p = fld;
  int x, y;
  for (y = 0; y < FLDH; y++) {
    for (x = 0; x < FLDW; x++) {
      int sx = x * CELW - w_x + WD / 2;
      int sy = y * CELH - w_y + HT / 2 + 1 + w_o;

      if (*p) {
        vgaimg *pic = walp[*p];
        if ((int)pic <= 3) {
          if (!bg) {
            byte *cmap = clrmap + ((int)pic + 7) * 256;
            V_remap_rect(sx, sy, CELW, CELH, cmap);
          }
        } else {
          V_pic(sx, sy, pic);
        }
      }

      p++;
    }
  }
}

void V_toggle() {
  // fuck
}

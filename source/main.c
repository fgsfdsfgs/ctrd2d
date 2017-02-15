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

#include "glob.h"
#include <stdio.h>
///#include <process.h>
#include <stdarg.h>
///#include <conio.h>
//#include <time.h>
///#include <dos.h>
#include "config.h"
#include "error.h"
#include "files.h"
#include "keyb.h"
#include "memory.h"
#include "menu.h"
#include "misc.h"
#include "player.h"
#include "sound.h"
#include "vga.h"
#include "view.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

#include <3ds.h>
#include <sys/time.h>

int gammaa = 0;

char main_pal[256][3], std_pal[256][3];
byte mixmap[256][256];
byte clrmap[256 * 12];

void logo(const char *s, ...) {
  va_list ap;
  int x, y;

  va_start(ap, s);
  vprintf(s, ap);
  va_end(ap);
  fflush(stdout);
  gfxFlushBuffers();
  gfxSwapBuffers();
  gspWaitForVBlank();
}

void logo_gas(int cur, int all) {
  logo(".");
}

byte gamcor[5][64] = {
#include "gamma.dat"
};

void setgamma(int g) {
  int t;

  if (g > 4)
    g = 4;
  if (g < 0)
    g = 0;
  gammaa = g;
  for (t = 0; t < 256; ++t) {
    std_pal[t][0] = gamcor[gammaa][main_pal[t][0]];
    std_pal[t][1] = gamcor[gammaa][main_pal[t][1]];
    std_pal[t][2] = gamcor[gammaa][main_pal[t][2]];
  }
  VP_setall(std_pal);
}

void setdefaultcontrols(void) {
  pl1.ku = VK_DUP;
  pl1.kd = VK_DDOWN;
  pl1.kl = VK_DLEFT;
  pl1.kr = VK_DRIGHT;
  pl1.kf = VK_B;
  pl1.kj = VK_A;
  pl1.kwl = VK_L;
  pl1.kwr = VK_R;
  pl1.kp = VK_Y;
  pl1.id = -1;
  pl2.ku = VK_DUP;
  pl2.kd = VK_DDOWN;
  pl2.kl = VK_DLEFT;
  pl2.kr = VK_DRIGHT;
  pl2.kf = VK_B;
  pl2.kj = VK_A;
  pl2.kwl = VK_L;
  pl2.kwr = VK_R;
  pl2.kp = VK_Y;
  pl2.id = -2;
}

u32 getmsec(void) { return svcGetSystemTick() / TICKS_PER_MSEC; }

void delaymsec(u32 ms) {
  /*int was_error;
    static struct timeval tv;
    u32 then, now, elapsed;
    then = getmsec;
    do {
      now = getmsec();
      elapsed = (now - then);
      then = now;
      if (elapsed >= ms) break;
      ms -= elapsed;
      tv.tv_sec = ms / 1000;
      tv.tv_usec = (ms % 1000) * 1000;
      was_error = select(0, NULL, NULL, NULL, &tv);
    } while (was_error);
  */
  svcSleepThread(ms * 1000000);
}

void myrandomize(void);

byte bright[256];

u8 isN3DS = 0;

int main(int argc, char *argv[]) {
  APT_CheckNew3DS(&isN3DS);
  if (isN3DS)
    osSetSpeedupEnable(true);

  gfxInitDefault();
  gfxSetDoubleBuffering(GFX_TOP, false);
  consoleInit(GFX_BOTTOM, NULL);

  if (chdir(ROOTDIR))
    ERR_failinit(ROOTDIR "does not exist");

  setdefaultcontrols();
  myrandomize();
  F_startup();
  F_addwad("doom2d.wad");
  CFG_args(argc, argv);
  CFG_load();
  F_initwads();
  M_startup();
  F_allocres();
  F_loadres(F_getresid("PLAYPAL"), main_pal, 0, 768);
  for (int i = 0; i < 256; ++i)
    bright[i] =
        ((int)main_pal[i][0] + main_pal[i][1] + main_pal[i][2]) * 8 / (63 * 3);
  F_loadres(F_getresid("MIXMAP"), mixmap, 0, 0x10000);
  F_loadres(F_getresid("COLORMAP"), clrmap, 0, 256 * 12);
  G_init();
  K_init();
  logo("S_init: init sound\n");
  S_init();
  S_initmusic();
  logo("V_init: init video\n");
  if (V_init() != 0)
    ERR_failinit("could not init video");
  setgamma(gammaa);
  logo("M_init: init menu\n");
  //GM_init();
  logo("loading music\n");
  F_loadmus("MENU");
  S_startmusic(0);

  logo("\nGAME STARTUP\n");
  u32 ticks = 0, t = 0;
  while (aptMainLoop()) {
    updatee_keys();
    S_updatemusic();
    G_act();
    G_draw();
    V_flush();
    gspWaitForVBlank();
    while ((t = getmsec()) - ticks < FRAMETIME)
      delaymsec(1);
    ticks = t - t % FRAMETIME;
  }

  gfxExit();
  return 0;
}

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

#include "keyb.h"
#include "config.h"
#include <3ds.h>

// стандартная функция обработки клавиш
key_f def_key_proc;

// массив клавиш: 0 - отпущена, иначе - нажата
unsigned char keys[32] = {0};

static key_f *key_proc = NULL;

void K_init() {}

void K_done() {}
// установить функцию обработки клавиш
void K_setkeyproc(key_f *k) { key_proc = k; }

void updatee_keys() {
  hidScanInput();
  u32 down = hidKeysDown();
  u32 held = hidKeysHeld();
  u32 up = hidKeysUp();
  u32 mask = 0;
  for (int i = 0; i < 32; ++i) {
    mask = BIT(i);
    if (down & mask)
      keys[i] = 1;
    else if (up & mask)
      keys[i] = 0;
    if (key_proc) {
      if (down & mask)
        key_proc(i, 1);
      if (up & mask)
        key_proc(i, 0);
    }
  }
}

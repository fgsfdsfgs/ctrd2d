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

#include "sound.h"
#include "3dsmixer.h"
#include "files.h"
#include "glob.h"

short snd_vol = 50;

int snddisabled = 1;

struct {
  snd_t *s;
  Mix_Chunk *c;
} chunks[NUM_CHUNKS];

void S_init(void) {
  if (!mixenabled()) {
    if (!mixinit(MIX_FORMAT_S8, 22050, NUM_CHANNELS)) {
      logo("   could not init csnd mixer\n   sound is disabled\n");
      snddisabled = 1;
    }
  }

  int i;
  for (i = 0; i < NUM_CHUNKS; i++) {
    chunks[i].s = NULL;
    chunks[i].c = NULL;
  }

  snddisabled = (snd_vol==0);

  S_volume(snd_vol);
}

void S_done(void) {
  free_chunks();
  if (mixenabled()) {
    mixkill();
  }
}

Mix_Chunk *get_chunk(snd_t *s, int r, int v) {
  int i, fi = -1;
  for (i = 0; i < NUM_CHUNKS; i++) {
    if (chunks[i].s == s)
      return chunks[i].c;
    if (chunks[i].s == NULL && fi == -1)
      fi = i;
  }

  if (fi == -1)
    return NULL;

  u8 *data = (u8 *)s + sizeof(snd_t);
  u32 dlen = s->len;

  Mix_Chunk *chunk = mixdatachunk(data, dlen, MIX_FORMAT_S8, s->rate);
  chunk->volume = (float)v / 255 * MIX_MAXVOLUME;

  chunks[fi].s = s;
  chunks[fi].c = chunk;

  return chunk;
}

void free_chunks() {
  if (snddisabled)
    return;
  mixstopchan(-1);
  int i;
  for (i = 0; i < NUM_CHUNKS; i++) {
    if (chunks[i].c) {
      mixfreechunk(chunks[i].c);
      chunks[i].c = NULL;
      chunks[i].s = NULL;
    }
  }
}

short S_play(snd_t *s, short c, unsigned r, short v) {
  if (snddisabled)
    return 0;
  Mix_Chunk *chunk = get_chunk(s, r, v);
  if (chunk == NULL)
    return 0;
  return mixstartchan(c, chunk, 0);
}

void S_stop(short c) { mixstopchan(c); }

void S_volume(int v) {
  if (snddisabled)
    return;
  snd_vol = v;
  if (snd_vol > 128)
    snd_vol = 128;
  if (snd_vol < 0)
    snd_vol = 0;
  mixvolchan(-1, snd_vol);
}

void S_wait() {
  if (snddisabled)
    return;
  while (mixplayingchan(-1)) {
    delaymsec(10);
  }
}

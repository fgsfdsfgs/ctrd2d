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

#include "3dsmixer.h"
#include "glob.h"
#include "sound.h"

short mus_vol = 50;

Mix_Music *muslo;

char music_random = ON;
int music_time = 3;
int music_fade = 5;

u32 muscount;

int musdisabled = 1;

void S_initmusic(void) {
  if (!mixenabled()) {
    if (!mixinit(MIX_FORMAT_S8, 22050, NUM_CHANNELS)) {
      logo("   could not init csnd mixer\n   sound is disabled\n");
      musdisabled = 1;
    }
  }

  muslo = NULL;
  muscount = 0;
  musdisabled = 1; //(mus_vol==0);
  S_volumemusic(mus_vol);
}

void S_donemusic(void) {
  if (mixenabled()) {
    F_freemus();
    mixkill();
  }
}

void S_startmusic(int time) {
  if (musdisabled)
    return;
  mixstartmus(muslo, -1);
  mixvolmus(mus_vol);
  muscount = time * 60 * 1000 / FRAMETIME;
}

void S_stopmusic(void) {
  if (musdisabled)
    return;
  mixstopmus();
  muscount = 0;
}

void S_volumemusic(int v) {
  if (musdisabled)
    return;
  mus_vol = v;
  if (mus_vol > 128)
    mus_vol = 128;
  if (mus_vol < 0)
    mus_vol = 0;
  if (mus_vol == 0 && mixplayingmus()) {
    S_stopmusic();
  } else if (mus_vol > 0 && !mixplayingmus()) {
    S_startmusic(music_time);
  } else {
    mixvolmus(v);
  }
}

struct {
  u8 ascii;
  u8 asciilc;
  char *ch;
} atrans[] = {{0x80, 0xA0, "A"},   //А
              {0x81, 0xA1, "B"},   //Б
              {0x82, 0xA2, "V"},   //В
              {0x83, 0xA3, "G"},   //Г
              {0x84, 0xA4, "D"},   //Д
              {0x85, 0xA5, "E"},   //Е
              {0x86, 0xA6, "ZH"},  //Ж
              {0x87, 0xA7, "Z"},   //З
              {0x88, 0xA8, "I"},   //И
              {0x89, 0xA9, "J"},   //Й
              {0x8A, 0xAA, "K"},   //К
              {0x8B, 0xAB, "L"},   //Л
              {0x8C, 0xAC, "M"},   //М
              {0x8D, 0xAD, "N"},   //Н
              {0x8E, 0xAE, "O"},   //О
              {0x8F, 0xAF, "P"},   //П
              {0x90, 0xE0, "R"},   //Р
              {0x91, 0xE1, "S"},   //С
              {0x92, 0xE2, "T"},   //Т
              {0x93, 0xE3, "U"},   //У
              {0x94, 0xE4, "F"},   //Ф
              {0x95, 0xE5, "H"},   //Х
              {0x96, 0xE6, "C"},   //Ц
              {0x97, 0xE7, "CH"},  //Ч
              {0x98, 0xE8, "SH"},  //Ш
              {0x99, 0xE9, "SCH"}, //Щ
              {0x9A, 0xEA, "X"},   //Ъ
              {0x9B, 0xEB, "Y"},   //Ы
              {0x9C, 0xEC, "J"},   //Ь
              {0x9D, 0xED, "E"},   //Э
              {0x9E, 0xEE, "JU"},  //Ю
              {0x9F, 0xEF, "JA"},  //Я
              {0}};

char *get_trans_char(u8 c) {
  int i = 0;
  while (atrans[i].ascii) {
    if (atrans[i].ascii == c || atrans[i].asciilc == c) {
      return atrans[i].ch;
    }
    i++;
  }
  return NULL;
}

void trans_ascii_str(char *dest, char *src) {
  char *p = dest;
  int i;
  for (i = 0; i < strlen(src); i++) {
    char *ch = get_trans_char(src[i]);
    if (ch) {
      strcpy(p, ch);
      p += strlen(ch);
    } else {
      strncpy(p, &src[i], 1);
      p++;
    }
  }
  *p = '\0';
}

void F_loadmus(char n[8]) {
  if (musdisabled)
    return;
  char f[50];
  char name[50];
  strcpy(f, "music/");
  int l = strlen(f);
  strncpy(&f[l], n, 8);
  f[l + 8] = '\0';
  trans_ascii_str(name, f);
  muslo = mixloadmus(name);
  if (!muslo)
    logo("could not load music '%s'", name);
}

void F_freemus(void) {
  if (musdisabled)
    return;
  if (muslo) {
    mixstopmus();
    mixfreemus(muslo);
  }
  muslo = NULL;
}

extern byte g_music[8];

static int volsetcount = 0;

void S_updatemusic() {
  if (musdisabled)
    return;

  //періодично встановлюю гучність музикі, так як вона сама підвищується до
  //максимуму через певний час
  volsetcount++;
  if (volsetcount % (5 * 1000 / FRAMETIME) == 0) {
    S_volumemusic(mus_vol);
  }

  if (muscount > 0) {
    if (muscount < music_fade * 1100 / FRAMETIME) {
      // Mix_FadeOutMusic(music_fade*1000);
       //TODO: fadeout
    }
    muscount--;
    if (muscount == 0) {
      if (music_random)
        F_randmus(g_music);
      else
        F_nextmus(g_music);
      F_freemus();
      F_loadmus(g_music);
      S_startmusic(music_time);
    }
  }
}

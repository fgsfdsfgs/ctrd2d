#include "3dsmixer.h"
#include <3ds.h>
#include <stdlib.h>
#include <string.h>

#define MUS_CHANS 1
#define SFX_CHANS 16

static int m_enabled = 0;
static int m_numchans = 0;
static char m_volmus = 0;
static int m_chan = 0;

int mixinit(u32 flags, u16 freq, int chans) {
  if (csndInit()) return 0;
  m_enabled = 1;
  return 1;
}

int mixenabled(void) { return m_enabled; }

void mixkill(void) {
  csndExit();
  m_enabled = 0;
}

Mix_Music *mixloadmus(const char *fname) { return NULL; }

void mixfreemus(Mix_Music *m) {}

int mixstartmus(Mix_Music *m, int ch) { return 0; }

int mixstopmus(void) { return 0; }

int mixplayingmus(void) { return 0; }

char mixvolmus(char vol) {
  char old = m_volmus;
  if (vol >= 0)
    m_volmus = vol;
  return old;
}

Mix_Chunk *mixdatachunk(u8 *data, u32 len, u32 flags, u32 freq) {
  Mix_Chunk *ch = calloc(1, sizeof(Mix_Chunk));
  ch->abuf = linearAlloc(len);
  ch->alen = len;
  ch->allocated = 1;
  ch->flags = flags | SOUND_ONE_SHOT;
  ch->freq = freq;
  memcpy(ch->abuf, data, len);
  return ch;
}

void mixfreechunk(Mix_Chunk *c) {
  if (c->allocated)
    linearFree(c->abuf);
  free(c);
}

void mixstopchan(int ch) {}

int mixstartchan(int ch, Mix_Chunk *chunk, int loop) {
  if (ch == -1) {ch = m_chan++; m_chan %= SFX_CHANS;}
  csndPlaySound(SOUND_CHANNEL(0x8 + MUS_CHANS + ch), chunk->flags, chunk->freq, 
                (float)(chunk->volume) / 255.f, 0.0,
                (u32*)chunk->abuf, (u32*)chunk->abuf, chunk->alen);
  return 0;
}

char mixvolchan(int ch, char vol) {
  return 0;
}

int mixplayingchan(int ch) { return 0; }

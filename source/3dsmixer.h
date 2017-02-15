#pragma once

#include <3ds.h>

#define MIX_MAXVOLUME 255
#define MIX_FORMAT_S8 SOUND_FORMAT_8BIT

typedef struct {
  int allocated;
  u8 *abuf;
  u32 alen;
  u32 flags;
  u32 freq;
  u8 volume;
} Mix_Chunk;

typedef struct {
  int error;
} Mix_Music;

int mixinit(u32, u16, int);
int mixenabled(void);
void mixkill(void);

Mix_Music *mixloadmus(const char *);
void mixfreemus(Mix_Music *);
int mixstartmus(Mix_Music *, int);
int mixstopmus(void);
int mixplayingmus(void);
char mixvolmus(char);

Mix_Chunk *mixdatachunk(u8 *, u32, u32, u32);
void mixfreechunk(Mix_Chunk *);
void mixstopchan(int);
int mixstartchan(int, Mix_Chunk *, int);
char mixvolchan(int, char);
int mixplayingchan(int);

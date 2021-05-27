/*
    2SF Decoder: for playing Nintendo DS Sound Format files (.2SF/.MINI2SF).

    Based on kode54's: https://www.foobar2000.org/components/view/foo_input_vio2sf (version 0.24.16)
    and on my earlier port of the similar GSF player.

    As compared to kode54's original 2sf.cpp, the code has been patched to NOT
    rely on any fubar2000 base impls.  (The migration must have messed up
    the original "silence suppression" impl - which seems to be rather crappy
    anyway and not worth the trouble - and it has just been disabled here.)

    Note: kode54's various psflib based decoder impls (see gsf, PSX, etc) use a copy/paste
    approach that amounts to 90% of the respective impls. It just seems bloody
    stupid to not pull out the respective code into some common shared base impl -
    but I really don't feel like cleaning up kode54's mess.

    NOTE: This emulator seems to make a total mess of the big/little endian handling
    (see mem.h where the respective impls do exactly the inverse of what their names suggest),
    i.e. it seems that in *some* cases the memory layout is inverse - but not
    always (see LE_TO_LOCAL_32).

    DeSmuME v0.8.0 Copyright (C) 2006 yopyop Copyright (C) 2006-2007 DeSmuME team
    foo_input_2sf Copyright Christopher Snowhill
    web port Copyright © 2019 Juergen Wothke. It is distributed under the same GPL license (NOTE:
    The GPL license DOES NOT apply to the example code in the "htdocs" folder which is
    merely provided as an example).
 */

#ifndef FILE2SF_H
#define FILE2SF_H

#include "psflib/psfcore.h"

struct NDS_state;

struct twosf_loader_state
{
  uint8_t* state;
  uint8_t* rom;
  size_t rom_size;
  size_t state_size;

  int initial_frames;
  int sync_type;
  int clockdown;
  int arm9_clockdown_level;
  int arm7_clockdown_level;

  twosf_loader_state()
    : state(0)
    , rom(0)
    , rom_size(0)
    , state_size(0)
    , initial_frames(-1)
    , sync_type(0)
    , clockdown(0)
    , arm9_clockdown_level(0)
    , arm7_clockdown_level(0)
  {
  }

  ~twosf_loader_state()
  {
    if (rom)
    {
      free(rom);
      rom = 0;
    }
    if (state)
    {
      free(state);
      state = 0;
    }
  }
};


struct twosf_running_state
{
  int16_t samples[AUDIO_BUF_SIZE * 2];
  int16_t* available_buffer;
  uint16_t available_buffer_size;
};


class File2SFReader : public FileReader
{
public:
  File2SFReader();
  virtual ~File2SFReader();

  virtual bool load(const char* path, bool meta) override;
  virtual int  read(short* buffer, int size) override;
  virtual int  length() override;
  virtual void seek(int ms) override;

private:
  void reset_playback();
  void reset();
  void shutdown();
  int  open(const char* path);

  void decode_initialize();
  bool decode_run(int16_t* * output_buffer, uint16_t* output_samples);

  twosf_loader_state*        m_state;
  struct twosf_running_state m_output;
  NDS_state*                 m_module;
};

#endif

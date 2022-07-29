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
    : state(nullptr)
    , rom(nullptr)
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
      rom = nullptr;
    }
    if (state)
    {
      free(state);
      state = nullptr;
    }
  }
};

struct twosf_running_state
{
  int16_t samples[AUDIO_BUF_SIZE * 2];
  int16_t* available_buffer;
  uint16_t available_buffer_size;
};


class File2SFReader : public AbstractReader
{
public:
  File2SFReader();
  virtual ~File2SFReader();

  virtual bool load(const char* path, bool meta) override;
  virtual int  read(short* buffer, int size) override;
  virtual void seek(int ms) override;

private:
  void reset_playback();
  void reset();
  void shutdown();
  int  open(const char* path);

  void decode_initialize();
  bool decode_run(int16_t* * output_buffer, uint16_t* output_samples);

  twosf_loader_state* m_state;
  twosf_running_state m_output;
  NDS_state*          m_module;
};

#endif

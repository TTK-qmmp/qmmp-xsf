#ifndef FILE2SF_H
#define FILE2SF_H

#include "psflib/psfcore.h"

struct twosf_loader_state;
struct twosf_running_state;
struct NDS_state;

class File2SFReader : public AbstractReader
{
public:
  File2SFReader();
  virtual ~File2SFReader();

  virtual bool load(const char* path, bool meta) override final;
  virtual int read(short* buffer, int size) override final;
  virtual void seek(int ms) override final;

  virtual const char * format() const override final;

private:
  void reset_playback();
  void reset();
  void shutdown();
  int open(const char* path);

  void decode_initialize();
  bool decode_run(int16_t* * output_buffer, uint16_t* output_samples);

  twosf_loader_state* m_state;
  twosf_running_state* m_output;
  NDS_state* m_module;
};

#endif

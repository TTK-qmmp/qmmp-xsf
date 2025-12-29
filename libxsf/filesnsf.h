#ifndef FILESNSF_H
#define FILESNSF_H

#include "psflib/psfcore.h"

struct snsf_loader_state;
struct snsf_running_state;
class SNESSystem;

class FileSNSFReader : public AbstractReader
{
public:
  FileSNSFReader();
  virtual ~FileSNSFReader();

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
  int decode_run(int16_t* output_buffer, uint16_t size);

  snsf_loader_state* m_state;
  snsf_running_state* m_output;
  SNESSystem* m_module;

  int m_remainder;
};

#endif

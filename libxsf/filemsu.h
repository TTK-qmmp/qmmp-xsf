#ifndef FILEMSU_H
#define FILEMSU_H

#include "psflib/psfcore.h"

struct msu_loader_state;

class FileMSUReader : public AbstractReader
{
public:
  FileMSUReader();
  virtual ~FileMSUReader();

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

  msu_loader_state* m_state;
};

#endif

#ifndef FILEMSU_H
#define FILEMSU_H

#include "psflib/psfcore.h"

struct msu_loader_state
{
  int64_t len = 0;
  int64_t pos = 0;
  void* buffer;

  msu_loader_state()
    : buffer(nullptr)
  {
  }

  ~msu_loader_state()
  {
    if ( buffer )
      free(buffer);
  }
};


class FileMSUReader : public AbstractReader
{
public:
  FileMSUReader();
  virtual ~FileMSUReader();

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
  int  decode_run(int16_t* output_buffer, uint16_t size);

  msu_loader_state* m_state;
};

#endif

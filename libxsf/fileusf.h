#ifndef FILEUSF_H
#define FILEUSF_H

#include "psflib/psfcore.h"

struct usf_loader_state
{
  int64_t len = 0;
  void* state;

  usf_loader_state()
    : state(nullptr)
  {
  }

  ~usf_loader_state()
  {
    if ( state )
      free(state);
  }
};


class FileUSFReader : public AbstractReader
{
public:
  FileUSFReader();
  virtual ~FileUSFReader();

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

  usf_loader_state* m_state;
};

#endif

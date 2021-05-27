#ifndef FILEGSF_H
#define FILEGSF_H

#include "psflib/psfcore.h"
#include "mgba/mgba/core/core.h"

struct gsf_loader_state
{
  int entry_set;
  uint32_t entry;
  uint8_t* data;
  size_t data_size;

  gsf_loader_state()
    : entry_set(0)
    , data(0)
    , data_size(0)
  {
  }

  ~gsf_loader_state()
  {
    if ( data )
      free(data);
  }
};


struct gsf_running_state
{
  struct mAVStream stream;
  int16_t samples[2048 * 2];
  int buffered;
  int16_t* available_buffer;
  uint16_t available_buffer_size;
};


class FileGSFReader : public FileReader
{
public:
  FileGSFReader();
  virtual ~FileGSFReader();

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

  gsf_loader_state*        m_state;
  struct gsf_running_state m_output;
  struct mCore*            m_module;
};

#endif

#ifndef FILESNSF_H
#define FILESNSF_H

#include "psflib/psfcore.h"

struct snsf_loader_state
{
  int base_set;
  uint32_t base;
  uint8_t* data;
  size_t data_size;
  uint8_t* sram;
  size_t sram_size;

  snsf_loader_state()
    : base_set(0)
    , data(nullptr)
    , data_size(0)
    , sram(nullptr)
    , sram_size(0)
  {
  }

  ~snsf_loader_state()
  {
    if (data)
      free(data);
    if (sram)
      free(sram);
  }
};

struct snsf_running_state
{
  unsigned long bytes_in_buffer;
  std::vector<uint8_t> sample_buffer;
};


class SNESSystem;
class FileSNSFReader : public AbstractReader
{
public:
  FileSNSFReader();
  virtual ~FileSNSFReader();

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

  snsf_loader_state* m_state;
  snsf_running_state m_output;
  SNESSystem*        m_module;

  int m_remainder;
};

#endif

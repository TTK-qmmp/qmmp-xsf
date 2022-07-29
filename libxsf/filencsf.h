#ifndef FILENCSF_H
#define FILENCSF_H

#include <memory>
#include "psflib/psfcore.h"

struct SDAT;
struct Player;

struct ncsf_loader_state
{
  uint32_t sseq = 0;
  std::vector<uint8_t> sdatData;
  std::unique_ptr<SDAT> sdat;
};


class FileNCSFReader : public AbstractReader
{
public:
  FileNCSFReader();
  virtual ~FileNCSFReader();

  virtual bool load(const char* path, bool meta) override;
  virtual int  read(short* buffer, int size) override;
  virtual void seek(int ms) override;

private:
  void reset_playback();
  void reset();
  void shutdown();
  int  open(const char* path);

  void decode_initialize();
  int  decode_run(int16_t* output_buffer, uint16_t size);

  std::vector<uint8_t> m_sample_buffer;
  ncsf_loader_state*   m_state;
  Player*              m_module;

  int m_remainder;
};

#endif

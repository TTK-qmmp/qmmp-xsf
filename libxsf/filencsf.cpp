#include "filencsf.h"
#include "sseq/Player.h"
#include "sseq/SDAT.h"

struct ncsf_loader_state
{
  uint32_t sseq = 0;
  std::vector<uint8_t> sdatData;
  std::unique_ptr<SDAT> sdat;
};


static int ncsf_loader(void* context, const uint8_t* exe, size_t exe_size, const uint8_t* reserved, size_t reserved_size)
{
  struct ncsf_loader_state* state = ( struct ncsf_loader_state * ) context;
  if (reserved_size >= 4)
  {
    state->sseq = get_le32(reserved);
  }

  if (exe_size >= 12)
  {
    uint32_t sdat_size = get_le32(exe + 8);
    if (sdat_size > exe_size)
      return -1;

    if (state->sdatData.empty())
      state->sdatData.resize(sdat_size, 0);
    else if (state->sdatData.size() < sdat_size)
      state->sdatData.resize(sdat_size);
    memcpy(&state->sdatData[0], exe, sdat_size);
  }
  return 0;
}


static struct FileAccess_t fncsf_file;

static void * fncsf_file_fopen(const char* uri)
{
  return fncsf_file.em_fopen(uri);
}

static size_t fncsf_file_fread(void* buffer, size_t size, size_t count, void* handle)
{
  return fncsf_file.em_fread(buffer, size, count, handle);
}

static int fncsf_file_fseek(void* handle, int64_t offset, int whence)
{
  return fncsf_file.em_fseek(handle, offset, whence);
}

static int fncsf_file_fclose(void* handle)
{
  fncsf_file.em_fclose(handle);
  return 0;
}

static long fncsf_file_ftell(void* handle)
{
  return fncsf_file.em_ftell(handle);
}

const psf_file_callbacks fncsf_file_system = {
  "\\/|:",
  fncsf_file_fopen,
  fncsf_file_fread,
  fncsf_file_fseek,
  fncsf_file_fclose,
  fncsf_file_ftell
};


FileNCSFReader::FileNCSFReader()
  : m_state(nullptr)
  , m_module(nullptr)
{
}

FileNCSFReader::~FileNCSFReader()
{
  shutdown();
}

bool FileNCSFReader::load(const char* path, bool meta)
{
  try {
    if(open(path) < 0)
    {
      return false;
    }

    if(!meta)
    {
      decode_initialize();
    }
    return true;
  } catch(...) {
    return false;
  }
}

int FileNCSFReader::read(short* buffer, int size)
{
  return decode_run(buffer, size);
}

void FileNCSFReader::seek(int time)
{
  double p_seconds = double(time) / 1000.0;
  if (p_seconds < m_emu_pos)
  {
    decode_initialize();
  }

  unsigned int howmany = (int)(p_seconds - m_emu_pos, m_sample_rate);
  // more abortable, and emu doesn't like doing huge numbers of samples per call anyway
  while (howmany)
  {
    unsigned int samples = 1024;
    m_module->GenerateSamples(m_sample_buffer, 0, samples);
    if (samples > howmany)
    {
      memmove(m_sample_buffer.data(), reinterpret_cast<int16_t *>(m_sample_buffer.data()) + howmany * 2, (samples - howmany) * 4);
      m_remainder = samples - howmany;
      samples     = howmany;
    }
    howmany -= samples;
  }

  m_data_written = 0;
  m_pos_delta    = (int)(p_seconds * 1000.);
  m_emu_pos      = p_seconds;

  calcfade();
}

const char * FileNCSFReader::format() const
{
  return "Nintendo DS Nitro Composer Sound Format";
}

void FileNCSFReader::reset_playback()
{
  m_emu_pos      = 0.;
  m_data_written = 0;
  m_pos_delta    = 0;
  m_remainder    = 0;

  calcfade();
}

void FileNCSFReader::reset()
{
  reset_playback();

  m_song_len    = 0;
  m_fade_len    = 0;
  m_tag_song_ms = 0;
  m_tag_fade_ms = 0;

  m_meta.reset();
}

void FileNCSFReader::shutdown()
{
  delete m_state;
  if(m_module)
  {
    m_module->Stop(true);
    delete m_module;
  }
}

int FileNCSFReader::open(const char* path)
{
  reset();
  m_path = path;

  psf_info_meta_state info_state;
  info_state.meta = &m_meta;
  if (psf_load(path, &fncsf_file_system, 0x25, nullptr, nullptr, psf_info_meta, &info_state, 0) <= 0)
    return -1;

  m_tag_song_ms = info_state.tag_song_ms;
  m_tag_fade_ms = info_state.tag_fade_ms;

  if (!m_tag_song_ms)
  {
    m_tag_song_ms = 170000;
    m_tag_fade_ms = 10000;
  }

  update_duration();
  m_sample_rate = 44100;
  return 0;
}

void FileNCSFReader::decode_initialize()
{
  shutdown();

  m_state  = new ncsf_loader_state();
  m_module = new Player();

  if (psf_load(m_path.c_str(), &fncsf_file_system, 0x25, ncsf_loader, m_state, nullptr, nullptr, 0) < 0)
    throw std::bad_alloc();

  PseudoFile file;
  file.data = &m_state->sdatData;
  m_state->sdat.reset(new SDAT(file, m_state->sseq));
  auto* sseq = m_state->sdat->sseq.get();

  m_module->sseqVol       = Cnv_Scale(sseq->info.vol);
  m_module->sampleRate    = m_sample_rate;
  m_module->interpolation = INTERPOLATION_SINC;
  m_module->Setup(sseq);
  m_module->Timer();

  reset_playback();

  m_sample_buffer.resize(4096 * 2 * sizeof(int16_t), 0);
}

int FileNCSFReader::decode_run(int16_t* output_buffer, uint16_t size)
{
  if ( !m_state )
    return 0;

  if (m_tag_song_ms && (m_pos_delta + mul_div(m_data_written, 1000, m_module->sampleRate)) >= m_tag_song_ms + m_tag_fade_ms)
    return 0;

  if (size > m_sample_buffer.size())
    m_sample_buffer.resize(size * 2);

  unsigned int written = 0;

  int usedSize = size / 2 / sizeof(int16_t);

  int samples = (m_song_len + m_fade_len) - m_data_written;
  if (samples > usedSize)
    samples = usedSize;

  if (m_remainder)
  {
    written     = m_remainder;
    m_remainder = 0;
  }
  else
  {
    written = samples;
    m_module->GenerateSamples(m_sample_buffer, 0, written);
  }

  m_emu_pos += double(written) / double(m_module->sampleRate);

  int d_start, d_end;
  d_start         = m_data_written;
  m_data_written += written;
  d_end           = m_data_written;

  if (m_tag_song_ms && d_end > m_song_len)
  {
    int16_t* foo = reinterpret_cast<int16_t *>(m_sample_buffer.data());
    for (int n = d_start; n < d_end; ++n)
    {
      if (n > m_song_len)
      {
        if (n > m_song_len + m_fade_len)
        {
          *(uint32_t *)foo = 0;
        }
        else
        {
          int bleh = m_song_len + m_fade_len - n;
          foo[0] = mul_div(foo[0], bleh, m_fade_len);
          foo[1] = mul_div(foo[1], bleh, m_fade_len);
        }
      }
      foo += 2;
    }
  }

  memcpy(output_buffer, m_sample_buffer.data(), written * 2 * sizeof(int16_t));
  return written;
}

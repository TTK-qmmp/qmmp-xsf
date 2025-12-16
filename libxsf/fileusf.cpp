#include "fileusf.h"
#include "n64/usf.h"

static int usf_loader(void* context, const uint8_t *, size_t exe_size, const uint8_t* data, size_t size)
{
  if (exe_size > 0)
    return -1;

  usf_upload_section(context, data, size);
  return 0;
}


static struct FileAccess_t fusf_file;

static void * fusf_file_fopen(const char* uri)
{
  return fusf_file.em_fopen(uri);
}

static size_t fusf_file_fread(void* buffer, size_t size, size_t count, void* handle)
{
  return fusf_file.em_fread(buffer, size, count, handle);
}

static int fusf_file_fseek(void* handle, int64_t offset, int whence)
{
  return fusf_file.em_fseek(handle, offset, whence);
}

static int fusf_file_fclose(void* handle)
{
  fusf_file.em_fclose(handle);
  return 0;
}

static long fusf_file_ftell(void* handle)
{
  return fusf_file.em_ftell(handle);
}

const psf_file_callbacks fusf_file_system = {
  "\\/|:",
  fusf_file_fopen,
  fusf_file_fread,
  fusf_file_fseek,
  fusf_file_fclose,
  fusf_file_ftell
};


FileUSFReader::FileUSFReader()
  : m_state(nullptr)
{
}

FileUSFReader::~FileUSFReader()
{
  shutdown();
}

bool FileUSFReader::load(const char* path, bool meta)
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

int FileUSFReader::read(short* buffer, int size)
{
  return decode_run(buffer, size);
}

void FileUSFReader::seek(int ms)
{
  if (ms / 1000 * m_sample_rate < m_emu_pos)
  {
    usf_restart(m_state->state);
    m_emu_pos = 0;
  }

  int left = ms / 1000 * m_sample_rate - m_emu_pos;
  while (left > 1024)
  {
    usf_render(m_state->state, 0, 1024, &m_sample_rate);
    m_emu_pos += 1024;
    left      -= 1024;
  }
}

const char * FileUSFReader::format() const
{
  return "Nintendo 64 Song Format";
}

void FileUSFReader::reset_playback()
{
  m_emu_pos      = 0.;
  m_data_written = 0;
  m_pos_delta    = 0;

  calcfade();
}

void FileUSFReader::reset()
{
  reset_playback();

  m_song_len    = 0;
  m_fade_len    = 0;
  m_tag_song_ms = 0;
  m_tag_fade_ms = 0;

  m_meta.reset();
}

void FileUSFReader::shutdown()
{
  if ( m_state )
  {
    usf_shutdown(m_state->state);
    delete m_state;
    m_state = 0;
  }
}

int FileUSFReader::open(const char* path)
{
  reset();
  m_path = path;

  psf_info_meta_state info_state;
  info_state.meta = &m_meta;

  // INFO: das info_state wird spaeter in den callbacks als "context" mitgegeben
  // psf_info_meta ist das "target"
  if ( psf_load(path, &fusf_file_system, 0x21, nullptr, nullptr, psf_info_meta, &info_state, 0) <= 0 )
    throw std::bad_alloc();

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

void FileUSFReader::decode_initialize()
{
  shutdown();

  m_state        = new usf_loader_state();
  m_state->state = malloc(usf_get_state_size());

  usf_clear(m_state->state);
  usf_set_hle_audio(m_state->state, 0);

  if ( psf_load(m_path.c_str(), &fusf_file_system, 0x21, usf_loader, m_state->state, nullptr, nullptr, 0) < 0 )
    throw std::bad_alloc();

  usf_set_compare(m_state->state, 0);
  usf_set_fifo_full(m_state->state, 0);
  usf_set_hle_audio(m_state->state, 1);


  usf_render(m_state->state, 0, 0, &m_sample_rate);
  m_state->len = m_sample_rate * (m_tag_song_ms / 1000);

  reset_playback();
}

int FileUSFReader::decode_run(int16_t* output_buffer, uint16_t size)
{
  if ( !m_state )
    return 0;

  if (m_state->len > 0 && m_emu_pos >= m_state->len)
    return 0;

  if (usf_render(m_state->state, (int16_t *)output_buffer, size, &m_sample_rate))
    return 0;

  m_emu_pos += size;
  return size;
}

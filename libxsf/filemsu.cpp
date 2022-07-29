#include "filemsu.h"

#define BUFFER_SIZE     1024

FileMSUReader::FileMSUReader()
  : m_state(nullptr)
{
}

FileMSUReader::~FileMSUReader()
{
  shutdown();
}

bool FileMSUReader::load(const char* path, bool meta)
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

int FileMSUReader::read(short* buffer, int size)
{
  return decode_run(buffer, size);
}

void FileMSUReader::seek(int ms)
{
  m_state->pos = m_state->len * ms / m_tag_song_ms;
  m_state->pos = (m_state->pos / (BUFFER_SIZE * 2)) * (BUFFER_SIZE * 2);
}

void FileMSUReader::reset_playback()
{
  m_emu_pos      = 0.;
  m_data_written = 0;
  m_pos_delta    = 0;

  calcfade();
}

void FileMSUReader::reset()
{
  reset_playback();

  m_song_len    = 0;
  m_fade_len    = 0;
  m_tag_song_ms = 0;
  m_tag_fade_ms = 0;

  m_info.reset();
}

void FileMSUReader::shutdown()
{
  if ( m_state )
  {
    delete m_state;
    m_state = nullptr;
  }
}

int FileMSUReader::open(const char* path)
{
  reset();
  m_path = path;

  shutdown();
  m_state = new msu_loader_state();

  FILE* fp = fopen(path, "r");
  fseek(fp, 0, SEEK_END);
  m_state->len = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  m_state->pos    = 8;
  m_state->buffer = malloc(m_state->len);
  fread(m_state->buffer, m_state->len, 1, fp);
  fclose(fp);

  if(memcmp(m_state->buffer, "MSU1", 4))
    return -1;

  m_sample_rate = 44100;
  m_tag_song_ms = (m_state->len - 8.0) / (m_sample_rate * 4.0) * 1000;
  m_tag_fade_ms = 0;

  m_info.set_length(m_tag_song_ms * .001);
  m_info.info_set_int("samplerate", m_sample_rate);
  m_info.info_set_int("channels", 2);
  return 0;
}

void FileMSUReader::decode_initialize()
{
}

int FileMSUReader::decode_run(int16_t* output_buffer, uint16_t size)
{
  if ( !m_state || m_state->pos >= m_state->len)
    return 0;

  int16_t        buf[size];
  const uint32_t buf_size = m_state->pos + sizeof(buf) < m_state->len ? sizeof(buf) : m_state->len - m_state->pos;
  memcpy(output_buffer, (char*)m_state->buffer + m_state->pos, buf_size);
  m_state->pos += buf_size;
  return buf_size / 4;
}

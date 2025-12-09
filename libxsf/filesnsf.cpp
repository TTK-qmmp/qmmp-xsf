#include "filesnsf.h"
#include "snes9x/SNESSystem.h"

static int snsf_loader(void* context, const uint8_t* exe, size_t exe_size, const uint8_t* reserved, size_t reserved_size)
{
  if ( exe_size < 8 )
    return -1;

  struct snsf_loader_state* state = (struct snsf_loader_state *) context;

  unsigned char* iptr;
  unsigned       isize;
  unsigned char* xptr;
  unsigned       xofs  = get_le32(exe + 0);
  unsigned       xsize = get_le32(exe + 4);
  if ( xsize > exe_size - 8 )
    return -1;
  if (!state->base_set)
  {
    state->base     = xofs;
    state->base_set = 1;
  }
  else
  {
    xofs += state->base;
  }
  {
    iptr             = state->data;
    isize            = state->data_size;
    state->data      = 0;
    state->data_size = 0;
  }
  if (!iptr)
  {
    unsigned rsize = xofs + xsize;
    {
      rsize -= 1;
      rsize |= rsize >> 1;
      rsize |= rsize >> 2;
      rsize |= rsize >> 4;
      rsize |= rsize >> 8;
      rsize |= rsize >> 16;
      rsize += 1;
    }
    iptr = (unsigned char *) malloc(rsize + 10);
    if (!iptr)
      return -1;
    memset(iptr, 0, rsize + 10);
    isize = rsize;
  }
  else if (isize < xofs + xsize)
  {
    unsigned rsize = xofs + xsize;
    {
      rsize -= 1;
      rsize |= rsize >> 1;
      rsize |= rsize >> 2;
      rsize |= rsize >> 4;
      rsize |= rsize >> 8;
      rsize |= rsize >> 16;
      rsize += 1;
    }
    xptr = (unsigned char *) realloc(iptr, xofs + rsize + 10);
    if (!xptr)
    {
      free(iptr);
      return -1;
    }
    iptr  = xptr;
    isize = rsize;
  }
  memcpy(iptr + xofs, exe + 8, xsize);
  {
    state->data      = iptr;
    state->data_size = isize;
  }

  // reserved section
  if (reserved_size >= 8)
  {
    unsigned rsvtype = get_le32(reserved + 0);
    unsigned rsvsize = get_le32(reserved + 4);

    if (rsvtype == 0)
    {
      // SRAM block
      if (reserved_size < 12 || rsvsize < 4)
      {
        return -1;
      }

      // check offset and size
      unsigned sram_offset     = get_le32(reserved + 8);
      unsigned sram_patch_size = rsvsize - 4;
      if (sram_offset + sram_patch_size > 0x20000)
      {
        return -1;
      }

      if (!state->sram)
      {
        state->sram = (unsigned char *) malloc(0x20000);
        if (!state->sram)
          return -1;
        memset(state->sram, 0, 0x20000);
      }

      // load SRAM data
      memcpy(state->sram + sram_offset, reserved + 12, sram_patch_size);
      // update SRAM size
      if (state->sram_size < sram_offset + sram_patch_size)
      {
        state->sram_size = sram_offset + sram_patch_size;
      }
    }
    else
    {
      return -1;
    }
  }

  return 0;
}


static struct FileAccess_t fsnsf_file;

static void * fsnsf_file_fopen(const char* uri)
{
  return fsnsf_file.em_fopen(uri);
}

static size_t fsnsf_file_fread(void* buffer, size_t size, size_t count, void* handle)
{
  return fsnsf_file.em_fread(buffer, size, count, handle);
}

static int fsnsf_file_fseek(void* handle, int64_t offset, int whence)
{
  return fsnsf_file.em_fseek(handle, offset, whence);
}

static int fsnsf_file_fclose(void* handle)
{
  fsnsf_file.em_fclose(handle);
  return 0;
}

static long fsnsf_file_ftell(void* handle)
{
  return fsnsf_file.em_ftell(handle);
}

const psf_file_callbacks fsnsf_file_system = {
  "\\/|:",
  fsnsf_file_fopen,
  fsnsf_file_fread,
  fsnsf_file_fseek,
  fsnsf_file_fclose,
  fsnsf_file_ftell
};


FileSNSFReader::FileSNSFReader()
  : m_state(nullptr)
  , m_module(nullptr)
{
}

FileSNSFReader::~FileSNSFReader()
{
  shutdown();
}

bool FileSNSFReader::load(const char* path, bool meta)
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

int FileSNSFReader::read(short* buffer, int size)
{
  return decode_run(buffer, size);
}

void FileSNSFReader::seek(int ms)
{
  double p_seconds = ms / 1000.0;
  if ( p_seconds < m_emu_pos )
  {
    decode_initialize();
  }

  unsigned int howmany = ( int )( ( p_seconds - m_emu_pos ) * m_sample_rate);
  // more abortable, and emu doesn't like doing huge numbers of samples per call anyway
  while ( howmany )
  {
    m_output.bytes_in_buffer  = 0;
    m_module->soundEnableFlag = (uint8_t)0 ^ 0xff;
    m_module->CPULoop();
    unsigned samples = m_output.bytes_in_buffer / 4;
    if ( samples > howmany )
    {
      memmove(m_output.sample_buffer.data(), ((int16_t *) m_output.sample_buffer.data()) + howmany * 2, ( samples - howmany ) * 4);
      m_remainder = samples - howmany;
      samples     = howmany;
    }
    howmany -= samples;
  }

  m_data_written = 0;
  m_pos_delta    = ( int )( p_seconds * 1000. );
  m_emu_pos      = p_seconds;

  calcfade();
}

const char * FileSNSFReader::format() const
{
  return "Super Nintendo Sound Format";
}

void FileSNSFReader::reset_playback()
{
  m_emu_pos      = 0.;
  m_data_written = 0;
  m_pos_delta    = 0;
  m_remainder    = 0;

  calcfade();
}

void FileSNSFReader::reset()
{
  reset_playback();

  m_song_len    = 0;
  m_fade_len    = 0;
  m_tag_song_ms = 0;
  m_tag_fade_ms = 0;

  memset(&m_output, 0, sizeof(m_output));

  m_info.reset();
}

void FileSNSFReader::shutdown()
{
  delete m_state;
  if ( m_module )
  {
    m_module->Term();
    delete m_module;
    m_module = NULL;
  }
}

int FileSNSFReader::open(const char* path)
{
  reset();
  m_path = path;

  psf_info_meta_state info_state;
  info_state.info = &m_info;
  if (psf_load(path, &fsnsf_file_system, 0x23, nullptr, nullptr, psf_info_meta, &info_state, 0) <= 0)
    return -1;

  m_tag_song_ms = info_state.tag_song_ms;
  m_tag_fade_ms = info_state.tag_fade_ms;

  if (!m_tag_song_ms)
  {
    m_tag_song_ms = 170000;
    m_tag_fade_ms = 10000;
  }

  m_sample_rate = 44100;
  return 0;
}

void FileSNSFReader::decode_initialize()
{
  shutdown();

  m_state  = new snsf_loader_state();
  m_module = new SNESSystem();

  if ( psf_load(m_path.c_str(), &fsnsf_file_system, 0x23, snsf_loader, m_state, nullptr, nullptr, 0) < 0 )
    throw std::bad_alloc();

  m_module->Load(m_state->data, m_state->data_size, m_state->sram, m_state->sram_size);
  m_module->soundSampleRate = m_sample_rate;
  m_module->SoundInit(&m_output.sample_buffer, &m_output.bytes_in_buffer);
  m_module->SoundReset();
  m_module->Init();
  m_module->Reset();

  reset_playback();
}

int FileSNSFReader::decode_run(int16_t* output_buffer, uint16_t)
{
  if ( !m_state )
    return 0;

  if ( m_tag_song_ms && ( m_pos_delta + mul_div(m_data_written, 1000, m_sample_rate) ) >= m_tag_song_ms + m_tag_fade_ms )
    return 0;

  unsigned int written = 0;
  if ( m_remainder )
  {
    written     = m_remainder;
    m_remainder = 0;
  }
  else
  {
    m_output.bytes_in_buffer  = 0;
    m_module->soundEnableFlag = (uint8_t)0 ^ 0xff;
    m_module->CPULoop();
    written = m_output.bytes_in_buffer / 4;
  }

  m_emu_pos += double(written) / m_sample_rate;

  int d_start, d_end;
  d_start         = m_data_written;
  m_data_written += written;
  d_end           = m_data_written;

  if ( m_tag_song_ms && d_end > m_song_len )
  {
    int16_t* foo = reinterpret_cast<int16_t *>(m_output.sample_buffer.data());
    int      n;
    for( n = d_start; n < d_end; ++n )
    {
      if ( n > m_song_len )
      {
        if ( n > m_song_len + m_fade_len )
        {
          *( uint32_t * ) foo = 0;
        }
        else
        {
          int bleh = m_song_len + m_fade_len - n;
          foo[ 0 ] = mul_div(foo[ 0 ], bleh, m_fade_len);
          foo[ 1 ] = mul_div(foo[ 1 ], bleh, m_fade_len);
        }
      }
      foo += 2;
    }
  }

  memcpy(output_buffer, m_output.sample_buffer.data(), written * 2 * sizeof(int16_t));
  return written;
}

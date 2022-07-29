#include "file2sf.h"
#include "vio2sf/desmume/state.h"
#include <zlib.h>

// note: used both in "issave=0" and "issave=1" mode
static int load_2sf_map(struct twosf_loader_state* state, int issave, const unsigned char* udata, unsigned usize)
{
  if (usize < 8)
    return -1;

  unsigned char* iptr;
  size_t         isize;
  unsigned char* xptr;
  unsigned       xsize = get_le32(udata + 4);
  unsigned       xofs  = get_le32(udata + 0);
  if (issave)
  {
    iptr              = state->state;
    isize             = state->state_size;
    state->state      = nullptr;
    state->state_size = 0;
  }
  else
  {
    iptr            = state->rom;
    isize           = state->rom_size;
    state->rom      = nullptr;
    state->rom_size = 0;
  }
  if (!iptr)
  {
    size_t rsize = xofs + xsize;
    if (!issave)
    {
      rsize -= 1;
      rsize |= rsize >> 1;
      rsize |= rsize >> 2;
      rsize |= rsize >> 4;
      rsize |= rsize >> 8;
      rsize |= rsize >> 16;
      rsize += 1;
    }
    iptr = (unsigned char *) malloc(rsize + 10);                // note: cleaned up upon size change and "state deletion"
    if (!iptr)
      return -1;
    memset(iptr, 0, rsize + 10);
    isize = rsize;
  }
  else if (isize < xofs + xsize)
  {
    size_t rsize = xofs + xsize;
    if (!issave)
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
      free(iptr);                               // note: previous buffer is cleanup up
      return -1;
    }
    iptr  = xptr;
    isize = rsize;
  }
  memcpy(iptr + xofs, udata + 8, xsize);
  if (issave)
  {
    state->state      = iptr;
    state->state_size = isize;
  }
  else
  {
    state->rom      = iptr;
    state->rom_size = isize;
  }
  return 0;
}

// note: only used in "issave=1" mode
static int load_2sf_mapz(struct twosf_loader_state* state, int issave, const unsigned char* zdata, unsigned zsize, unsigned zcrc)
{
  int            ret;
  int            zerr;
  uLongf         usize = 8;
  uLongf         rsize = usize;
  unsigned char* udata;
  unsigned char* rdata;

  udata = (unsigned char *) malloc(usize);
  if (!udata)
    return -1;

  while (Z_OK != (zerr = uncompress(udata, &usize, zdata, zsize)))
  {
    if (Z_MEM_ERROR != zerr && Z_BUF_ERROR != zerr)
    {
      free(udata);
      return -1;
    }
    if (usize >= 8)
    {
      usize = get_le32(udata + 4) + 8;
      if (usize < rsize)
      {
        rsize += rsize;
        usize  = rsize;
      }
      else
        rsize = usize;
    }
    else
    {
      rsize += rsize;
      usize  = rsize;
    }
    rdata = (unsigned char *) realloc(udata, usize);
    if (!rdata)
    {
      free(udata);
      return -1;
    }
    udata = rdata;
  }

  rdata = (unsigned char *) realloc(udata, usize);
  if (!rdata)
  {
    free(udata);
    return -1;
  }

  ret = load_2sf_map(state, issave, rdata, (unsigned) usize);
  free(rdata);
  return ret;
}

static int twosf_loader(void* context, const uint8_t* exe, size_t exe_size, const uint8_t* reserved, size_t reserved_size)
{
  struct twosf_loader_state* state = ( struct twosf_loader_state * ) context;

  if ( exe_size >= 8 )
  {
    if ( load_2sf_map(state, 0, exe, (unsigned) exe_size) )
      return -1;
  }

  if ( reserved_size )
  {
    size_t resv_pos = 0;
    if ( reserved_size < 16 )
      return -1;
    while ( resv_pos + 12 < reserved_size )
    {
      unsigned save_size = get_le32(reserved + resv_pos + 4);
      unsigned save_crc  = get_le32(reserved + resv_pos + 8);
      if (get_le32(reserved + resv_pos + 0) == 0x45564153)
      {
        if (resv_pos + 12 + save_size > reserved_size)
          return -1;
        if (load_2sf_mapz(state, 1, reserved + resv_pos + 12, save_size, save_crc))
          return -1;
      }
      resv_pos += 12 + save_size;
    }
  }

  return 0;
}

static int twosf_info(void* context, const char* name, const char* value)
{
  struct twosf_loader_state* state = ( struct twosf_loader_state * ) context;
  char*                      end;

  if ( !stricmp_utf8(name, "_frames") )
  {
    state->initial_frames = strtol(value, &end, 10);
  }
  else if ( !stricmp_utf8(name, "_clockdown") )
  {
    state->clockdown = strtol(value, &end, 10);
  }
  else if ( !stricmp_utf8(name, "_vio2sf_sync_type") )
  {
    state->sync_type = strtol(value, &end, 10);
  }
  else if ( !stricmp_utf8(name, "_vio2sf_arm9_clockdown_level") )
  {
    state->arm9_clockdown_level = strtol(value, &end, 10);
  }
  else if ( !stricmp_utf8(name, "_vio2sf_arm7_clockdown_level") )
  {
    state->arm7_clockdown_level = strtol(value, &end, 10);
  }

  return 0;
}


static struct FileAccess_t f2sf_file;

static void * f2sf_file_fopen(const char* uri)
{
  return f2sf_file.em_fopen(uri);
}

static size_t f2sf_file_fread(void* buffer, size_t size, size_t count, void* handle)
{
  return f2sf_file.em_fread(buffer, size, count, handle);
}

static int f2sf_file_fseek(void* handle, int64_t offset, int whence)
{
  return f2sf_file.em_fseek(handle, offset, whence);
}

static int f2sf_file_fclose(void* handle)
{
  f2sf_file.em_fclose(handle);
  return 0;
}

static long f2sf_file_ftell(void* handle)
{
  return f2sf_file.em_ftell(handle);
}

const psf_file_callbacks f2sf_file_system = {
  "\\/|:",
  f2sf_file_fopen,
  f2sf_file_fread,
  f2sf_file_fseek,
  f2sf_file_fclose,
  f2sf_file_ftell
};



File2SFReader::File2SFReader()
  : m_state(nullptr)
  , m_module(nullptr)
{
}

File2SFReader::~File2SFReader()
{
  shutdown();
}

bool File2SFReader::load(const char* path, bool meta)
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

int File2SFReader::read(short* buffer, int size)
{
  uint16_t requested_size = size;

  while (size)
  {
    if (m_output.available_buffer_size)
    {
      if (m_output.available_buffer_size >= size)
      {
        memcpy(buffer, m_output.available_buffer, size << 2);
        m_output.available_buffer      += size << 2;
        m_output.available_buffer_size -= size;
        return requested_size;
      }
      else
      {
        memcpy(buffer, m_output.available_buffer, m_output.available_buffer_size << 2);
        m_output.available_buffer = nullptr;

        buffer += m_output.available_buffer_size<<2;
        size   -= m_output.available_buffer_size;
      }
    }
    else
    {
      if(!decode_run(&m_output.available_buffer, &m_output.available_buffer_size))
      {
        return 0;                               // end song (just ignore whatever output might actually be there)
      }
    }
  }
  return requested_size;
}

void File2SFReader::seek(int ms)
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
    // 2sf start
    unsigned int samples = (howmany > AUDIO_BUF_SIZE) ? AUDIO_BUF_SIZE : howmany;
    state_render(m_module, m_output.samples, samples);
    // 2sf end
    howmany -= samples;
  }

  m_data_written = 0;
  m_pos_delta    = ( int )( p_seconds * 1000. );
  m_emu_pos      = p_seconds;

  calcfade();
}

void File2SFReader::reset_playback()
{
  m_emu_pos      = 0.;
  m_data_written = 0;
  m_pos_delta    = 0;

  calcfade();
}

void File2SFReader::reset()
{
  reset_playback();

  m_song_len    = 0;
  m_fade_len    = 0;
  m_tag_song_ms = 0;
  m_tag_fade_ms = 0;

  memset(&m_output, 0, sizeof(m_output));

  m_info.reset();
}

void File2SFReader::shutdown()
{
  if ( m_module )
  {
    state_deinit(m_module);
    free(m_module);
    m_module = NULL;
  }
  delete m_state;
}

int File2SFReader::open(const char* path)
{
  reset();
  m_path = path;

  psf_info_meta_state info_state;
  info_state.info = &m_info;
  if ( psf_load(path, &f2sf_file_system, 0x24, nullptr, nullptr, psf_info_meta, &info_state, 0) <= 0 )
    return -1;

  m_tag_song_ms = info_state.tag_song_ms;
  m_tag_fade_ms = info_state.tag_fade_ms;

  if (!m_tag_song_ms)
  {
    m_tag_song_ms = 170000;
    m_tag_fade_ms = 10000;
  }

  m_sample_rate = 44100;
  m_info.set_length( (double)( m_tag_song_ms + m_tag_fade_ms ) * .001);
  m_info.info_set_int("samplerate", m_sample_rate);
  m_info.info_set_int("channels", 2);
  return 0;
}

void File2SFReader::decode_initialize()
{
  shutdown();

  m_state = new twosf_loader_state();
  // 2sf impl start
  m_module = ( NDS_state * ) calloc(1, sizeof(NDS_state) );
  if ( !m_module )
    throw std::bad_alloc();

  if ( state_init(m_module) )
    throw std::bad_alloc();

  if ( !m_state->rom && !m_state->state )
  {
    if ( psf_load(m_path.c_str(), &f2sf_file_system, 0x24, twosf_loader, m_state, twosf_info, m_state, 1) < 0 )
      throw std::bad_alloc();

    if (!m_state->arm7_clockdown_level)
      m_state->arm7_clockdown_level = m_state->clockdown;
    if (!m_state->arm9_clockdown_level)
      m_state->arm9_clockdown_level = m_state->clockdown;
  }

  m_module->dwInterpolation = 4;
  m_module->dwChannelMute   = 0;

  m_module->initial_frames       = m_state->initial_frames;
  m_module->sync_type            = m_state->sync_type;
  m_module->arm7_clockdown_level = m_state->arm7_clockdown_level;
  m_module->arm9_clockdown_level = m_state->arm9_clockdown_level;

  if ( m_state->rom )
  {
    state_setrom(m_module, m_state->rom, m_state->rom_size, 0);
  }
  state_loadstate(m_module, m_state->state, m_state->state_size);

  reset_playback();
}

// same (old) impl already used for GSF
bool File2SFReader::decode_run(int16_t* * output_buffer, uint16_t* output_samples)
{
  if ( !m_state )
    return false;

  if ( m_tag_song_ms && ( m_data_written  >= (m_song_len + m_fade_len)) )
    return false;

  unsigned int written = AUDIO_BUF_SIZE;

  short* ptr;
  // 2sf start
  state_render(m_module, m_output.samples, written);
  // 2sf end
  ptr = m_output.samples;

  // note: copy/paste standard impl
  m_emu_pos += double( written ) / m_sample_rate;

  int d_start, d_end;
  d_start         = m_data_written;
  m_data_written += written;
  d_end           = m_data_written;

  if ( m_tag_song_ms && (d_end > m_song_len))
  {
    short* foo = ptr;
    int    n;
    for( n = d_start; n < d_end; ++n )
    {
      if ( n > m_song_len )
      {
        if ( n > (m_song_len + m_fade_len) )
        {
          foo[ 0 ] = 0;
          foo[ 1 ] = 0;
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

  *output_buffer  = ptr;
  *output_samples = written;
  return true;
}

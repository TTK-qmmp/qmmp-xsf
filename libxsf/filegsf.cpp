#include "filegsf.h"
#include "mgba/core/blip_buf.h"
#include "mgba-util/vfs.h"

extern "C" void * anonymousMemoryMap(size_t size)
{
  return malloc(size);
}

extern "C" void mappedMemoryFree(void* memory, size_t size)
{
  UNUSED(size);
  free (memory);
}

static void gsf_postAudioBuffer(struct mAVStream* stream, blip_t* left, blip_t* right)
{
  // note: the amount of samples delivered per channel actually ranges from 2048 to 2050!
// fprintf(stderr, "a: %d %d \n", blip_samples_avail(left), blip_samples_avail(right));
  struct gsf_running_state* state = (struct gsf_running_state *) stream;
  blip_read_samples(left, state->samples, 1024, true);
  blip_read_samples(right, state->samples + 1, 1024, true);
  state->buffered = 1024;
}

static int gsf_loader(void* context, const uint8_t* exe, size_t exe_size, const uint8_t *, size_t)
{
  if ( exe_size < 12 )
    return -1;

  struct gsf_loader_state* state = ( struct gsf_loader_state * ) context;

  unsigned char* iptr;
  unsigned       isize;
  unsigned char* xptr;
  unsigned       xentry = get_le32(exe + 0);
  unsigned       xsize  = get_le32(exe + 8);
  unsigned       xofs   = get_le32(exe + 4) & 0x1ffffff;
  if ( xsize < exe_size - 12 )
    return -1;
  if (!state->entry_set)
  {
    state->entry     = xentry;
    state->entry_set = 1;
  }
  {
    iptr             = state->data;
    isize            = state->data_size;
    state->data      = nullptr;
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
  memcpy(iptr + xofs, exe + 12, xsize);
  {
    state->data      = iptr;
    state->data_size = isize;
  }
  return 0;
}


static struct FileAccess_t fgsf_file;

static void * fgsf_file_fopen(const char* uri)
{
  return fgsf_file.em_fopen(uri);
}

static size_t fgsf_file_fread(void* buffer, size_t size, size_t count, void* handle)
{
  return fgsf_file.em_fread(buffer, size, count, handle);
}

static int fgsf_file_fseek(void* handle, int64_t offset, int whence)
{
  return fgsf_file.em_fseek(handle, offset, whence);
}

static int fgsf_file_fclose(void* handle)
{
  fgsf_file.em_fclose(handle);
  return 0;
}

static long fgsf_file_ftell(void* handle)
{
  return fgsf_file.em_ftell(handle);
}

const psf_file_callbacks fgsf_file_system = {
  "\\/|:",
  fgsf_file_fopen,
  fgsf_file_fread,
  fgsf_file_fseek,
  fgsf_file_fclose,
  fgsf_file_ftell
};



FileGSFReader::FileGSFReader()
  : m_state(nullptr)
  , m_module(nullptr)
{
}

FileGSFReader::~FileGSFReader()
{
  shutdown();
}

bool FileGSFReader::load(const char* path, bool meta)
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

int FileGSFReader::read(short* buffer, int size)
{
  uint16_t requested_size = size;

  while (size)
  {
    if (m_output.available_buffer_size)
    {
      if (m_output.available_buffer_size >= size)
      {
        memcpy(buffer, m_output.available_buffer, size<<2);
        m_output.available_buffer      += size<<2;
        m_output.available_buffer_size -= size;
        return requested_size;
      }
      else
      {
        memcpy(buffer, m_output.available_buffer, m_output.available_buffer_size<<2);
        m_output.available_buffer = nullptr;

        buffer += m_output.available_buffer_size<<2;
        size   -= m_output.available_buffer_size;
      }
    }
    else
    {
      if(!decode_run(&m_output.available_buffer, &m_output.available_buffer_size))
      {
        return 0;                                 // end song (just ignore whatever output might actually be there)
      }
    }
  }
  return requested_size;
}

int FileGSFReader::length()
{
  return m_tag_song_ms;
}

void FileGSFReader::seek(int ms)
{
  double p_seconds = ms / 1000.0;
  if ( p_seconds < m_emu_pos )
  {
    decode_initialize();
  }

  unsigned int howmany = ( int )( ( p_seconds - m_emu_pos ) * 44100 );
  // more abortable, and emu doesn't like doing huge numbers of samples per call anyway
  while ( howmany )
  {
    m_output.buffered = 0;
    while (!m_output.buffered)
      m_module->runFrame(m_module);
    unsigned samples = m_output.buffered;
    if ( samples > howmany )
    {
      memmove(m_output.samples, m_output.samples + howmany * 2, ( samples - howmany ) * 4);
      samples = howmany;
    }
    howmany -= samples;
  }

  m_data_written = 0;
  m_pos_delta    = ( int )( p_seconds * 1000. );
  m_emu_pos      = p_seconds;

  calcfade();
}

void FileGSFReader::reset_playback()
{
  m_emu_pos      = 0.;
  m_data_written = 0;
  m_pos_delta    = 0;

  calcfade();
}

void FileGSFReader::reset()
{
  reset_playback();

  m_song_len    = 0;
  m_fade_len    = 0;
  m_tag_song_ms = 0;
  m_tag_fade_ms = 0;

  memset(&m_output, 0, sizeof(m_output));

  m_info.reset();
}

void FileGSFReader::shutdown()
{
  if ( m_module )
  {
    m_module->deinit(m_module);
    m_module = NULL;
  }
  delete m_state;
}

int FileGSFReader::open(const char* path)
{
  reset();
  m_path = path;

  psf_info_meta_state info_state;
  info_state.info = &m_info;

  // INFO: info_state is what is later passed as "context" in the callbacks
  // psf_info_meta then is the "target"
  if ( psf_load(path, &fgsf_file_system, 0x22, nullptr, nullptr, psf_info_meta, &info_state, 0) <= 0 )
    return -1;

  m_tag_song_ms = info_state.tag_song_ms;
  m_tag_fade_ms = info_state.tag_fade_ms;

  if (!m_tag_song_ms)
  {
    m_tag_song_ms = 170000;
    m_tag_fade_ms = 10000;
  }

  m_info.set_length( (double)( m_tag_song_ms + m_tag_fade_ms ) * .001);
  m_info.info_set_int("samplerate", 44100);
  m_info.info_set_int("channels", 2);
  return 0;
}

void FileGSFReader::decode_initialize()
{
  shutdown();

  m_state = new gsf_loader_state();
  if ( psf_load(m_path.c_str(), &fgsf_file_system, 0x22, gsf_loader, m_state, nullptr, nullptr, 0) < 0 )
    throw std::bad_alloc();

  if (m_state->data_size > UINT_MAX)
    throw std::bad_alloc();

  struct VFile* rom = VFileFromConstMemory(m_state->data, m_state->data_size);
  if (!rom)
    throw std::bad_alloc();

  struct mCore* core = mCoreFindVF(rom);
  if (!core)
  {
    rom->close(rom);
    throw std::bad_alloc();
  }

  memset(&m_output, 0, sizeof(m_output));
  m_output.stream.postAudioBuffer = gsf_postAudioBuffer;

  core->init(core);
  core->setAVStream(core, &m_output.stream);
  mCoreInitConfig(core, NULL);

  core->setAudioBufferSize(core, 2048);

  blip_set_rates(core->getAudioChannel(core, 0), core->frequency(core), 44100);
  blip_set_rates(core->getAudioChannel(core, 1), core->frequency(core), 44100);

  struct mCoreOptions opts = {};
  opts.useBios    = false;
  opts.skipBios   = true;
  opts.volume     = 0x100;
  opts.sampleRate = 44100;

  mCoreConfigLoadDefaults(&core->config, &opts);
  core->loadROM(core, rom);
  core->reset(core);
  m_module = core;

  reset_playback();
}

bool FileGSFReader::decode_run(int16_t* * output_buffer, uint16_t* output_samples)
{
  if ( !m_state )
    return false;
    
  if ( m_tag_song_ms && ( m_pos_delta + mul_div(m_data_written, 1000, 44100) ) >= m_tag_song_ms + m_tag_fade_ms )
    return false;

  int samples = ( m_song_len + m_fade_len ) - m_data_written;
  if ( samples > 2048 )
    samples = 2048;

  m_output.buffered = 0;
  while (!m_output.buffered)
    m_module->runFrame(m_module);

  unsigned int written = m_output.buffered;
  m_emu_pos += double( written ) / 44100.;

  int d_start, d_end;
  d_start         = m_data_written;
  m_data_written += written;
  d_end           = m_data_written;

  if ( m_tag_song_ms && d_end > m_song_len )
  {
    short* foo = m_output.samples;
    int    n;
    for( n = d_start; n < d_end; ++n )
    {
      if ( n > m_song_len )
      {
        if ( n > m_song_len + m_fade_len )
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

  *output_buffer  = m_output.samples;
  *output_samples = written;
  return true;
}

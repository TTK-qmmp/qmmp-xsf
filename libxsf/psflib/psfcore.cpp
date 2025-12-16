#include "psfcore.h"

void meta_info::add(const char* tag, const char* value)
{
  if(m.find(tag) == m.end())
  {
    m.insert(std::make_pair(tag, value));
  }
  else
  {
    m[tag] = value;
  }
}

void meta_info::add(std::string& tag, const char* value)
{
  if(m.find(tag) == m.end())
  {
    m.insert(std::make_pair(tag, value));
  }
  else
  {
    m[tag] = value;
  }
}

int stricmp_utf8(std::string const& s1, const char* s2)
{
  return strcasecmp(s1.c_str(), s2);
}

int stricmp_utf8(const char* s1, const char* s2)
{
  return strcasecmp(s1, s2);
}

unsigned long parse_time_crap(const char* input)
{
  if (!input)
    return BORK_TIME;
  int len = strlen(input);
  if (!len)
    return BORK_TIME;
  int value = 0;
  {
    int i;
    for (i = len - 1; i >= 0; i--)
    {
      if ((input[i] < '0' || input[i] > '9') && input[i] != ':' && input[i] != ',' && input[i] != '.')
      {
        return BORK_TIME;
      }
    }
  }

  char* foo = strdup_(input);

  if ( !foo )
    return BORK_TIME;

  char* bar  = foo;
  char* strs = bar + strlen(foo) - 1;
  char* end;
  while (strs > bar && (*strs >= '0' && *strs <= '9'))
  {
    strs--;
  }
  if (*strs == '.' || *strs == ',')
  {
    // fraction of a second
    strs++;
    if (strlen(strs) > 3)
      strs[3] = 0;
    value = strtoul(strs, &end, 10);
    switch (strlen(strs))
    {
    case 1:
      value *= 100;
      break;
    case 2:
      value *= 10;
      break;
    }
    strs--;
    *strs = 0;
    strs--;
  }
  while (strs > bar && (*strs >= '0' && *strs <= '9'))
  {
    strs--;
  }
  // seconds
  if (*strs < '0' || *strs > '9')
    strs++;
  value += strtoul(strs, &end, 10) * 1000;
  if (strs > bar)
  {
    strs--;
    *strs = 0;
    strs--;
    while (strs > bar && (*strs >= '0' && *strs <= '9'))
    {
      strs--;
    }
    if (*strs < '0' || *strs > '9')
      strs++;
    value += strtoul(strs, &end, 10) * 60000;
    if (strs > bar)
    {
      strs--;
      *strs = 0;
      strs--;
      while (strs > bar && (*strs >= '0' && *strs <= '9'))
      {
        strs--;
      }
      value += strtoul(strs, &end, 10) * 3600000;
    }
  }
  free(foo);
  return value;
}

unsigned get_le32(void const* p)
{
  return (unsigned) ((unsigned char const *) p) [3] << 24 |
         (unsigned) ((unsigned char const *) p) [2] << 16 |
         (unsigned) ((unsigned char const *) p) [1] <<  8 |
         (unsigned) ((unsigned char const *) p) [0];
}

int psf_info_meta(void* context, const char* name, const char* value)
{
  // typical tags: _lib, _enablecompare(on), _enableFIFOfull(on), fade, volume
  // game, genre, year, copyright, track, title, length(x:x.xxx), artist

  // FIXME: various "_"-settings are currently not used to configure the emulator

  psf_info_meta_state* state = ( psf_info_meta_state * ) context;
  std::string&         tag   = state->name;

  tag.assign(name);

  if (!stricmp_utf8(tag, "game"))
  {
    tag.assign("album");
  }
  else if (!stricmp_utf8(tag, "year"))
  {
    tag.assign("date");
  }

  if (!stricmp_utf8(tag, "length"))
  {
    const int temp = parse_time_crap(value);
    if (temp != BORK_TIME)
    {
      state->tag_song_ms = temp;
    }
  }
  else if (!stricmp_utf8(tag, "fade"))
  {
    const int temp = parse_time_crap(value);
    if (temp != BORK_TIME)
    {
      state->tag_fade_ms = temp;
    }
  }
  else if (!stricmp_utf8(tag, "utf8"))
  {
    state->utf8 = true;
  }

  if (!stricmp_utf8(tag, "title"))
  {
    state->meta->add("title", value);
  }
  else if (!stricmp_utf8(tag, "artist"))
  {
    state->meta->add("artist", value);
  }
  else if (!stricmp_utf8(tag, "album"))
  {
    state->meta->add("album", value);
  }
  else if (!stricmp_utf8(tag, "date"))
  {
    state->meta->add("year", value);
  }
  else if (!stricmp_utf8(tag, "genre"))
  {
    state->meta->add("genre", value);
  }
  else if (!stricmp_utf8(tag, "copyright"))
  {
    state->meta->add("copyright", value);
  }
  else if (!stricmp_utf8(tag, "comment"))
  {
    state->meta->add("comment", value);
  }

  return 0;
}


AbstractReader::AbstractReader()
{
  m_emu_pos      = 0.0;
  m_sample_rate  = 0;
  m_data_written = 0;
  m_pos_delta    = 0;
  m_song_len     = 0;
  m_fade_len     = 0;
  m_tag_song_ms  = 0;
  m_tag_fade_ms  = 0;
}

AbstractReader::~AbstractReader()
{
}

double AbstractReader::mul_div(int ms, int sampleRate, int d)
{
  return ((double)ms)*sampleRate / d;
}

void AbstractReader::calcfade()
{
  m_song_len = mul_div(m_tag_song_ms - m_pos_delta, 44100, 1000);
  m_fade_len = mul_div(m_tag_fade_ms, 44100, 1000);
}

void AbstractReader::update_duration()
{
  m_meta.add("song_ms", std::to_string(m_tag_song_ms).c_str());
  m_meta.add("fade_ms", std::to_string(m_tag_fade_ms).c_str());
}

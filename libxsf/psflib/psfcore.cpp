#include "psfcore.h"

file_info::file_info()
{

}

file_info::~file_info()
{

}

void file_info::meta_add(const char* tag, const char* value)
{
  meta_map.insert(std::make_pair(tag, value));
}

void file_info::meta_add(std::string& tag, const char* value)
{
  meta_map.insert(std::make_pair(tag, value));
}

int stricmp_utf8(std::string const& s1, const char* s2)
{
  return strcasecmp(s1.c_str(), s2);
}

int stricmp_utf8(const char* s1, const char* s2)
{
  return strcasecmp(s1, s2);
}

int stricmp_utf8_partial(std::string const& s1,  const char* s2)
{
  std::string s1pref = s1.substr(0, strlen(s2));
  return strcasecmp(s1pref.c_str(), s2);
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
    int temp = parse_time_crap(value);
    if (temp != BORK_TIME)
    {
      state->tag_song_ms = temp;
    }
  }
  else if (!stricmp_utf8(tag, "fade"))
  {
    int temp = parse_time_crap(value);
    if (temp != BORK_TIME)
    {
      state->tag_fade_ms = temp;
    }
  }
  else if (!stricmp_utf8(tag, "utf8"))
  {
    state->utf8 = true;
  }

  if (!strcasecmp(tag.c_str(), "title"))
  {
    state->info->meta_add("title", value);
  }
  else if (!strcasecmp(tag.c_str(), "artist"))
  {
    state->info->meta_add("artist", value);
  }
  else if (!strcasecmp(tag.c_str(), "album"))
  {
    state->info->meta_add("album", value);
  }
  else if (!strcasecmp(tag.c_str(), "date"))
  {
    state->info->meta_add("year", value);
  }
  else if (!strcasecmp(tag.c_str(), "genre"))
  {
    state->info->meta_add("genre", value);
  }
  else if (!strcasecmp(tag.c_str(), "copyright"))
  {
    state->info->meta_add("copyright", value);
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

int AbstractReader::length() const
{
  return m_tag_song_ms + m_tag_fade_ms;
}

int AbstractReader::rate() const
{
  return m_sample_rate;
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


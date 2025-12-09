/*
   PSFLIB - Main PSF parser implementation

   Copyright (c) 2012-2015 Christopher Snowhill

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
 */

#ifndef CORE_H
#define CORE_H

#include <map>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <vector>
#include "psflib.h"

// emulation logic seems to be hardcoded to this..
#define stricmp strcasecmp
#define AUDIO_BUF_SIZE 1024
#define MAX_INFO_LEN  10
#define BORK_TIME 0xC0CAC01A

# define strdup_(s)                                                           \
  (__extension__                                                              \
     ({                                                                        \
    const char* __old = (s);                                                \
    size_t __len = strlen (__old) + 1;                                      \
    char* __new = (char *) malloc (__len);                          \
    (char *) memcpy (__new, __old, __len);                                  \
  }))


int stricmp_utf8(std::string const& s1, const char* s2);
int stricmp_utf8(const char* s1, const char* s2);
int stricmp_utf8_partial(std::string const& s1,  const char* s2);
unsigned long parse_time_crap(const char* input);

unsigned get_le32(void const* p);

int psf_info_meta(void* context, const char* name, const char* value);

struct FileAccess_t
{
  void * em_fopen(const char* uri)
  {
    return (void *)fopen(uri, "rb");
  }

  size_t em_fread(void* buffer, size_t size, size_t count, void* handle)
  {
    return fread(buffer, size, count, (FILE *)handle);
  }

  int em_fseek(void* handle, int64_t offset, int whence)
  {
    return fseek( (FILE *) handle, offset, whence);
  }

  long int em_ftell(void* handle)
  {
    return ftell( (FILE *) handle);
  }

  int em_fclose(void* handle)
  {
    return fclose( (FILE *) handle);
  }

  size_t em_fgetlength(FILE* f)
  {
    int         fd = fileno(f);
    struct stat buf;
    fstat(fd, &buf);
    return buf.st_size;
  }
};


//////////////////////////////////////////////////////////////////////////////
typedef std::map<std::string, std::string> file_meta;

class file_info
{
public:
  file_info();
  ~file_info();

  void meta_add(const char* tag, const char* value);
  void meta_add(std::string& tag, const char* value);

  inline void reset() { meta_map.clear(); }
  inline unsigned int meta_get_count() const { return meta_map.size(); }
  inline file_meta &get_meta() { return meta_map; }

private:
  file_meta meta_map;
};

struct psf_info_meta_state
{
  file_info* info;
  std::string name;
  bool utf8;
  int tag_song_ms;
  int tag_fade_ms;

  psf_info_meta_state()
    : info(nullptr)
    , utf8(false)
    , tag_song_ms(0)
    , tag_fade_ms(0)
  {
  }
};


class AbstractReader
{
public:
  AbstractReader();
  virtual ~AbstractReader();

  virtual bool load(const char* path, bool meta) = 0;
  virtual int  read(short* buffer, int size) = 0;
  virtual void seek(int ms) = 0;

  virtual const char * format() const = 0;

  int length() const;
  int rate() const;

  inline file_meta &get_meta() { return m_info.get_meta(); }

protected:
  double mul_div(int ms, int sampleRate, int d);
  void   calcfade();

  std::string m_path;
  file_info   m_info;

  double m_emu_pos;    // in seconds
  int    m_sample_rate;
  int    m_data_written, m_pos_delta;
  int    m_song_len, m_fade_len;
  int    m_tag_song_ms, m_tag_fade_ms;
};

#endif // CORE_H

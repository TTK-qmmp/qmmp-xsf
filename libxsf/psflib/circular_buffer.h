#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

#include <vector>
#include <algorithm>

long const silence_threshold = 8;

template <typename T>
class circular_buffer
{
public:
  circular_buffer()
    : readptr(0)
    , writeptr(0)
    , size(0)
    , used(0)
    , silence_count(0)
  {
    memset(last_written, 0, sizeof(last_written) );
    memset(last_read, 0, sizeof(last_read) );
  }

  unsigned long data_available()
  {
    return used;
  }

  unsigned long free_space()
  {
    return size - used;
  }

  T * get_write_ptr(unsigned long& count_out)
  {
    count_out = size - writeptr;
    if ( count_out > size - used )
      count_out = size - used;
    return &buffer[writeptr];
  }

  bool samples_written(unsigned long count)
  {
    unsigned long max_count = size - writeptr;
    if ( max_count > size - used )
      max_count = size - used;
    if ( count > max_count )
      return false;
    silence_count += count_silent(&buffer[ 0 ] + writeptr, &buffer[ 0 ] + writeptr + count, last_written);
    used          += count;
    writeptr       = ( writeptr + count ) % size;
    return true;
  }

  bool write(const T* src, unsigned count)
  {
    if (count > free_space())
      return false;
    while (count)
    {
      unsigned delta = size - writeptr;
      if (delta > count)
        delta = count;
      std::copy(src, src + delta, buffer.begin() + writeptr);
      used    += delta;
      writeptr = (writeptr + delta) % size;
      src     += delta;
      count   -= delta;
    }
    return true;
  }

  unsigned long read(T* dst, unsigned long count)
  {
    unsigned long done = 0;
    for(;; )
    {
      unsigned long delta = size - readptr;
      if ( delta > used )
        delta = used;
      if ( delta > count )
        delta = count;
      if ( !delta )
        break;

      if ( dst )
        std::copy(buffer.begin() + readptr, buffer.begin() + readptr + delta, dst);
      silence_count -= count_silent(&buffer[ 0 ] + readptr, &buffer[ 0 ] + readptr + delta, last_read);
      if ( dst )
        dst += delta;
      done   += delta;
      readptr = ( readptr + delta ) % size;
      count  -= delta;
      used   -= delta;
    }
    return done;
  }

  void reset()
  {
    readptr = writeptr = used = 0;
    memset(last_written, 0, sizeof(last_written) );
    memset(last_read, 0, sizeof(last_read) );
  }

  void resize(unsigned long p_size)
  {
    size = p_size;
    buffer.resize(p_size);
    reset();
  }

  bool test_silence() const
  {
    return silence_count == used;
  }

private:
  static unsigned long count_silent(T const* begin, T const* end, T* last)
  {
    unsigned long count = 0;
    T const*      p     = begin;
    long          delta[ 2 ];
    while ( p < end )
    {
      delta[ 0 ] = p[ 0 ] - last[ 0 ];
      delta[ 1 ] = p[ 1 ] - last[ 1 ];
      if ( ( (unsigned long)( delta[ 0 ] + silence_threshold ) <= ( unsigned long ) silence_threshold * 2 ) ||
           ( ( unsigned long )( delta[ 1 ] + silence_threshold ) <= ( unsigned long ) silence_threshold * 2 ) )
        count += 2;
      last[ 0 ] += (T) delta[ 0 ];
      last[ 1 ] += (T) delta[ 1 ];
      p         += 2;
    }
    return count;
  }

  std::vector<T> buffer;
  unsigned long  readptr, writeptr, used, size;
  unsigned long  silence_count;
  T              last_written[2];
  T              last_read[2];
};

#endif

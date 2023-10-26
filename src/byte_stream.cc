#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  if ( is_closed() or data.empty() or error_ )
    return;
  uint64_t size = min( data.size(), capacity_ );
  buffer_ += data.substr( 0, size );
  capacity_ -= size;
  bytes_pushed_ += size;
}

void Writer::close() // 关闭写入端，读取端没关
{
  closed_ = true;
}

void Writer::set_error()
{
  error_ = true;
}

bool Writer::is_closed() const
{
  return closed_;
}

uint64_t Writer::available_capacity() const
{
  return capacity_;
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

string_view Reader::peek() const
{
  return { buffer_.begin(), buffer_.end() };
}

bool Reader::is_finished() const // 这才是判断eof的条件，而不是close就为eof
{
  return buffer_.empty() and closed_;
}

bool Reader::has_error() const
{
  return error_;
}

void Reader::pop( uint64_t len )
{
  uint64_t size = min( len, buffer_.size() );
  bytes_poped_ += size;
  buffer_.erase( 0, size );
  capacity_ += size;
}

uint64_t Reader::bytes_buffered() const
{
  return buffer_.size();
}

uint64_t Reader::bytes_popped() const
{
  return bytes_poped_;
}

#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // 先把当前到达的子串放到buffer里，和暂存的子串一起处理一下
  // 注意：存在字符串内容为空，带有 EOF 标志的碎片
  if ( !data.empty() ) {
    preprocess( first_index, data, output );
  }

  // 准备交给ByteStream
  while ( !buffer_.empty() and buffer_.begin()->first == next_idx_ ) {
    const auto& iter = buffer_.begin();
    output.push( iter->second );
    byte_unassembled_ -= iter->second.size();
    buffer_.erase( iter );
  }

  // 查看EOF标志
  if ( is_last_substring ) {
    end_ = true;
    eof_idx_ = first_index + data.size();
  }

  // 查看是否结束
  if ( end_ and output.bytes_pushed() == eof_idx_ ) {
    output.close();
  }
}

void Reassembler::preprocess( uint64_t& first_index, std::string& data, Writer& output )
{
  uint64_t cap = output.available_capacity();
  next_idx_ = output.bytes_pushed();
  uint64_t end_idx = next_idx_ + cap;

  // 太后面 or 太前面
  if ( first_index >= end_idx or first_index + data.size() <= next_idx_ ) {
    return;
  }
  // 需要去尾
  if ( first_index + data.size() > end_idx ) {
    data = data.substr( 0, end_idx - first_index );
  }
  // 需要去头
  if ( first_index < next_idx_ ) {
    data = data.substr( next_idx_ - first_index );
    first_index = next_idx_;
  }

  // 把子串截取好后，准备去重
  if ( buffer_.empty() ) {
    byte_unassembled_ += data.size();
    buffer_.insert( { first_index, data } );
    return;
  } else {
    store( first_index, data );
  }
}

void Reassembler::store( uint64_t idx, std::string str )
{
  for ( auto iter = buffer_.begin(); iter != buffer_.end(); ) {
    uint64_t str_end = idx + str.size() - 1;
    uint64_t iter_end = iter->first + iter->second.size() - 1;

    if ( ( idx >= iter->first && idx <= iter_end ) or ( iter->first >= idx && iter->first <= str_end )
         or ( str_end + 1 == iter->first ) ) {
      merge( idx, str, iter->first, iter->second ); // 消除存在的重叠
      byte_unassembled_ -= iter->second.size();
      buffer_.erase( iter++ ); // 合并后先删除已存在的 str
    } else {
      ++iter;
    }
  }
  byte_unassembled_ += str.size();
  buffer_.insert( { idx, str } ); // 把处理完重叠并合并后的 seg 插入缓冲区
}

// 注意，前两个是引用！相当于直接在上面修改
void Reassembler::merge( uint64_t& idx, string& str, uint64_t buf_idx, string buf )
{
  size_t seg_tail = idx + str.length() - 1;
  size_t cache_tail = buf_idx + buf.length() - 1;
  if ( seg_tail + 1 == 1 ) {
    str = str + buf;
    return;
  }
  // 情况一：str在前buf在后
  if ( idx < buf_idx and seg_tail <= cache_tail ) {
    str = str.substr( 0, buf_idx - idx ) + buf;
    return;
  }
  // 情况二：buf在前str在后
  else if ( idx >= buf_idx && seg_tail > cache_tail ) {
    str = buf + str.substr( buf_idx + buf.length() - idx );
    idx = buf_idx;
    return;
  }
  // 情况三: str被buf覆盖
  else if ( idx >= buf_idx && seg_tail <= cache_tail ) {
    str = buf;
    idx = buf_idx;
    return;
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return byte_unassembled_;
}

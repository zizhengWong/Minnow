#include "wrapping_integers.hh"

using namespace std;

// 把absolute seqno包装成seqno
// zero_point就是ISN
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { zero_point + static_cast<uint32_t>( n ) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint32_t offset = raw_value_ - zero_point.raw_value_;
  uint32_t low32 = (uint32_t)checkpoint;
  uint64_t high32 = ( checkpoint & 0xffffffff00000000 );
  if ( offset == low32 )
    return high32 + offset;
  // 放心，uint运算不会得到负数，只会得到相对距离
  else if ( ( offset - low32 ) < ( low32 - offset ) )
    return offset > low32 ? high32 + offset : high32 + offset + ( 1L << 32 );
  else
    return offset < low32 ? high32 + offset : ( high32 == 0 ? offset : high32 + offset - ( 1L << 32 ) );
}

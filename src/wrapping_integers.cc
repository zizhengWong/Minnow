#include "wrapping_integers.hh"

using namespace std;

// 把absolute seqno包装成seqno
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32{(uint32_t)((n + zero_point.raw_value_) % ((uint64_t)1 << 32))};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t INT32_RANGE = 1L << 32;
    uint32_t offset = this->raw_value_ - zero_point.raw_value_;
    if (checkpoint > offset) {
        uint64_t real_checkpoint = (checkpoint - offset) + (INT32_RANGE >> 1);
        uint64_t wrap_num = real_checkpoint / INT32_RANGE;
        return wrap_num * INT32_RANGE + offset;
    }
    return offset;
}

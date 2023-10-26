#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if ( !syn_ ) {
    if ( !message.SYN ) {
      return;
    }
    syn_ = true;
    zero_point_ = message.seqno; // 设置ISN
  }
  if ( message.FIN ) {
    fin_ = true;
  }
  uint64_t check_point = inbound_stream.bytes_pushed() + 1;
  uint64_t absolute_seqno = Wrap32( message.seqno ).unwrap( zero_point_, check_point );
  uint64_t stream_idx = absolute_seqno + message.SYN - 1;
  reassembler.insert( stream_idx, message.payload, message.FIN, inbound_stream );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage msg;
  uint64_t cap = inbound_stream.available_capacity();
  msg.window_size = cap > 65535 ? 65535 : cap;
  if ( !syn_ ) {
    msg.ackno = nullopt;
  } else {
    uint64_t check_point = inbound_stream.bytes_pushed() + 1;
    if ( fin_ && inbound_stream.is_closed() ) {
      ++check_point;
    }
    msg.ackno = zero_point_ + check_point;
  }
  return msg;
}

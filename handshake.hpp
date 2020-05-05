#ifndef HANDSHAKE_H__
#define HANDSHAKE_H__

#include "room.hpp"


class handshake_session : private nr_session,
			  public std::enable_shared_from_this<handshake_session>
{
public:
  handshake_session(tcp::socket socket, network_room &room) : socket_(std::move(socket)),
							      room_(room),
							      keep_alive(true)
  {
    IP = socket_.remote_endpoint().address().to_string();
    room_.join(shared_from_this());
    do_read_handshake();
  }

  void start()
  {
    while(!m_b_ready){}; // wait until be reading 
    start_handshake();
  }
  
private:

  // variables
  std::atomic_bool m_b_ready(false);

  // methods
  void process_handshake_message(std::byte *buffer, uint32_t buffersize)
  {
    ST_HANDSHAKE_HELLO_ACK *message = std::static_cast<ST_HANDSHAKE_HELLO_ACK*>(buffer);
    switch(message.head){
    case EN_HANDSHAKE::
    };
  }
  
  void do_read_handshake()
  {
    auto self(shared_from_this());
    if(!m_b_rady) m_b_ready = true;
    boost::asio::async_read(socket_,
			    boost::asio::buffer(read_buffer_.data(), read_buffer_.capacity()),
			    [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
			      if (!ec) {
				ST_RAW_MESSAGE *message = std::static_cast<ST_RAW_MESSAGE*>(read_buffer.data());
				switch(message.head){
				case EN_MESSAGE_HEAD::HANDSHAKE:
				  process_handshake_message(message->buffer,message->buffersize);
				  break;
			        default:
				  LOG_WARNING("attempt to do handshake in runtime mode.");
				  break;
				};
				do_read_handshake();
			      }
			      else
				{
				  room_.leave(shared_from_this());
				}

			    });
			    
  }
  
  void do_handshake_()
  {
    LOG_INFO("handshake begin");
    auto self(shared_from_this());
    boost::asio::async_
  }
  
};

#endif

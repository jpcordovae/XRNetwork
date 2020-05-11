#ifndef HANDSHAKE_H__
#define HANDSHAKE_H__

#include "session.hpp"

class handshake_session : public nr_session
{
public:
  handshake_session(tcp::socket socket, network_room &room) : nr_session(std::move(socket),room),
                                                              last_status(EN_RAW_MESSAGE_HEAD::NONE)
  {
  }

  void start()
  {
    IP = socket_.remote_endpoint().address().to_string();
    room_.join(shared_from_this());
    do_read_handshake();
    while(!m_b_ready){}; // wait until be reading
    do_read_handshake();
  }

private:

  // Variables
  std::atomic_bool m_b_ready;
  EN_RAW_MESSAGE_HEAD last_status;
  // Methods
  void handshake_hello_ack(std::byte *buffer, uint32_t buffersize)
  {
    //ST_HANDSHAKE_HELLO_ACK *message = static_cast<ST_HANDSHAKE_HELLO_ACK*>((void*)buffer);
    //message->participant_id;
    ST_HANDSHAKE_CREDENTIALS *hc = new ST_HANDSHAKE_CREDENTIALS();
    //hc->server_certificate_buffer;
    ST_RAW_MESSAGE msg;
    msg.head = EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS;
    msg.buffersize = sizeof(msg);
    memcpy(msg.buffer,hc,sizeof(msg));
    deliver_byte((std::byte*)&msg,sizeof(msg));
  }

  void handshake_credentials_ack(std::byte *buffer, uint32_t buffersize)
  {
    ST_HANDSHAKE_CREDENTIALS_ACK *msg = (ST_HANDSHAKE_CREDENTIALS_ACK*)buffer;
    if(strcmp((char*)msg->login_buffer,"login")!=0 || strcmp((char*)msg->password_buffer,"password")!=0) {
      room_.leave(shared_from_this());
    }
  }

  void do_read_handshake()
  {
    auto self(shared_from_this());
    if(!m_b_ready) m_b_ready = true;
    boost::asio::async_read(socket_,
                            boost::asio::buffer(read_buffer_.data(), read_buffer_.capacity()),
                            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                              if (!ec) {
                                ST_RAW_MESSAGE *message = static_cast<ST_RAW_MESSAGE*>((void*)read_buffer_.data());
                                switch(message->head){
                                case EN_RAW_MESSAGE_HEAD::HANDSHAKE_HELLO_ACK:
                                  handshake_hello_ack(message->buffer,message->buffersize);
                                  break;
                                case EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS_ACK:
                                  handshake_credentials_ack(message->buffer,message->buffersize);
                                  break;
                                case EN_RAW_MESSAGE_HEAD::PARTICIPANT_INFO_REQUEST_ACK:
                                  //participant_info_request(message->buffer,message->buffersize);
                                  break;
                                case EN_RAW_MESSAGE_HEAD::NEW_PARTICIPANT_INFO_ACK:
                                  //new_participant_info_ack(message->buffer,message->buffersize);
                                  break;
                                default:
                                  //LOG_WARNING("attempt to do handshake in runtime mode.");
                                  break;
                                };
                                do_read_handshake();
                              }
                              else{
                                room_.leave(shared_from_this());
                              }
                            });
  }
  
  void do_handshake_()
  {
    //LOG_INFO("handshake begin");
    auto self(shared_from_this());
    //boost::asio::async_write
  }
  
};

#endif

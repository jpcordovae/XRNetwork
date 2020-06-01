#ifndef HANDSHAKE_H__
#define HANDSHAKE_H__

#include "session.hpp"
#include "error_messages.hpp"

class handshake_session : public nr_session
{
public:
  handshake_session(std::shared_ptr<tcp::socket> socket,
                    network_room &room,
                    const uint64_t service_id,
                    const std::string server_name) : nr_session(socket,
                                                                room,
                                                                service_id,
                                                                server_name),
                                                     last_status(EN_RAW_MESSAGE_HEAD::NONE)
  {
  }

  /*handshake_session(tcp::socket socket_,
    network_room &room,
    //network_room &handshake_room,
    uint64_t service_id,
    std::string server_name) : nr_session(std::move(socket_),
    handshake_room(room),
    service_id,
    server_name),
    last_status(EN_RAW_MESSAGE_HEAD::NONE)
    {
    }*/

  void start()
  {
    nr_session::start();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    do_start_handshake();
  }

private:

  // Variables
  std::atomic_bool m_b_ready;
  EN_RAW_MESSAGE_HEAD last_status;
  // Methods

  void do_start_handshake()
  {
    std::cout << "starting handshake" << std::endl;
    //timestamp
    //auto now_ms = std::chrono::system_clock::now();
    //auto value = std::chrono::duration_cast<std::chrono::milliseconds>(now_ms.time_since_epoch());
    //uint64_t timestamp = std::chrono::duration<uint64_t>(value.count());
    uint64_t timestamp = get_timestamp_now();
    ST_HANDSHAKE_HELLO hh = build_handshake_hello(m_service_id,m_id,timestamp,m_server_name);
    ST_RAW_MESSAGE raw = build_raw_message((uint16_t)EN_RAW_MESSAGE_HEAD::HANDSHAKE_HELLO,(std::byte*)&hh,sizeof(hh));
    deliver_byte((std::byte*)&raw,sizeof(ST_RAW_MESSAGE));
  }

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

  bool handshake_credentials_ack(std::byte *buffer, uint32_t buffersize)
  {
    ST_HANDSHAKE_CREDENTIALS_ACK *msg = (ST_HANDSHAKE_CREDENTIALS_ACK*)buffer;
    if(strcmp((char*)msg->login_buffer,"login")!=0 || strcmp((char*)msg->password_buffer,"password")!=0) {
      return false;
    }
    return true;
  }

  void do_read_handshake()
  {
    auto self(shared_from_this());
    if(!m_b_ready) m_b_ready = true;
    //boost::asio::async_read(socket_,
    boost::asio::async_read(*m_socket_ptr,
                            boost::asio::buffer(read_buffer_.data(), read_buffer_.capacity()),
                            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                              if (!ec) {
                                ST_RAW_MESSAGE *message = static_cast<ST_RAW_MESSAGE*>((void*)read_buffer_.data());
                                switch(message->head){
                                case EN_RAW_MESSAGE_HEAD::HANDSHAKE_HELLO_ACK:
                                  {
                                    ST_HANDSHAKE_HELLO_ACK *hha = static_cast<ST_HANDSHAKE_HELLO_ACK*>((void*)message->buffer);
                                    m_name = std::string((char*)hha->participant_name_buffer);
                                  }
                                  break;
                                case EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS_ACK:
                                  {
                                    ST_HANDSHAKE_CREDENTIALS_ACK *msg = static_cast<ST_HANDSHAKE_CREDENTIALS_ACK*>((void*)message->buffer);
                                    if(strcmp((char*)msg->login_buffer,"login") != 0 || strcmp((char*)msg->password_buffer,"password") != 0) {
                                      m_room.leave(shared_from_this());
                                    }
                                  }
                                  break;
                                case EN_RAW_MESSAGE_HEAD::PARTICIPANT_INFO_REQUEST_ACK:
                                  //participant_info_request(message->buffer,message->buffersize);
                                  {
                                  }
                                  break;
                                case EN_RAW_MESSAGE_HEAD::PARTICIPANT_UPDATE_ACK:
                                  //new_participant_info_ack(message->buffer,message->buffersize);
                                  {
                                  }
                                  break;
                                default:
                                  //LOG_WARNING("meader message not recognized.");
                                  break;
                                };
                                //room_.new_message(this->ID_, read_buffer_.data(), (uint32_t)read_buffer_.capacity());

                                //if (room_.get_broadcast_messages())
                                //  room_.deliver_byte(read_buffer_.data(),read_buffer_.capacity());

                                //memset(read_buffer_.data(), 0x00, read_buffer_.capacity());

                                do_read_handshake();
                              }
                              else{
                                //check_system_error_code(ec);
                                std::cout << ec.message() << std::endl;
                                m_room.leave(shared_from_this());
                              }
                            });
  }
};

#endif


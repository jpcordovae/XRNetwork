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

  void start()
  {
    //nr_session::start();
    IP = m_socket_ptr->remote_endpoint().address().to_string();
    m_room.join(shared_from_this());
    //std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
    do_read_handshake();
    uint64_t timestamp = get_timestamp_now();
    ST_HANDSHAKE_HELLO hh = build_handshake_hello(m_service_id,m_id,timestamp,m_server_name);
    std::cout << "ST_HANDSHAKE_HELLO :" << std::endl << hh << std::endl;
    XRMessage msg((uint16_t)EN_RAW_MESSAGE_HEAD::HANDSHAKE_HELLO,(std::byte*)&hh,(uint32_t) sizeof(ST_HANDSHAKE_HELLO));
    deliver_byte((std::byte*)msg.data(),msg.size());
  }

  void check_handshake_hello_ack(ST_HANDSHAKE_HELLO_ACK *st_hello_ack, uint32_t buffersize)
  {
    ST_HANDSHAKE_CREDENTIALS *hc = new ST_HANDSHAKE_CREDENTIALS();
    //hc->server_certificate_buffer;
    //ST_RAW_MESSAGE msg;
    //msg.head = EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS;
    //msg.buffersize = sizeof(msg);
    //memcpy(msg.buffer,hc,sizeof(msg));
    XRMessage msg = XRMessage((uint16_t)EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS,(std::byte*)hc,(uint32_t)sizeof(ST_HANDSHAKE_CREDENTIALS));
    deliver_byte((std::byte*)&msg,msg.size());
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
                                std::cout << "receiving " << std::to_string(bytes_transferred) << std::endl;
                                XRMessage_ptr message_ptr(new XRMessage(read_buffer_.data(),bytes_transferred,true));
                                xr_message_header *header = message_ptr->get_header();
                                switch((uint16_t)header->head){
                                case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::HANDSHAKE_HELLO_ACK):
                                  {
                                    std::cout << "HANDSHAKE_HELLO_ACK: " << std::endl;
                                    ST_HANDSHAKE_HELLO_ACK *st_hello_ack = (ST_HANDSHAKE_HELLO_ACK*)message_ptr->payload();
                                    check_handshake_hello_ack(st_hello_ack,header->buffersize);
                                    //m_name = std::string((char*)hha->participant_name_buffer);
                                  }
                                  break;
                                case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS_ACK):
                                  {
                                    //ST_HANDSHAKE_CREDENTIALS_ACK *msg = static_cast<ST_HANDSHAKE_CREDENTIALS_ACK*>((void*)message->buffer);
                                    //if(strcmp((char*)msg->login_buffer,"login") != 0 || strcmp((char*)msg->password_buffer,"password") != 0) {
                                    //m_room.leave(shared_from_this());
                                    //}
                                  }
                                  break;
                                case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::PARTICIPANT_INFO_REQUEST_ACK):
                                  //participant_info_request(message->buffer,message->buffersize);
                                  {
                                  }
                                  break;
                                case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::PARTICIPANT_UPDATE_ACK):
                                  //new_participant_info_ack(message->buffer,message->buffersize);
                                  {
                                  }
                                  break;
                                default:
                                  //LOG_WARNING("meader message not recognized.");
                                  break;
                                };
                                do_read_handshake();
                              }
                              else{
                                //check_system_error_code(ec);
                                std::cout << "do_read_handshake: " << ec.message() << std::endl;
                                m_room.leave(shared_from_this());
                              }
                            });
  }
};

#endif


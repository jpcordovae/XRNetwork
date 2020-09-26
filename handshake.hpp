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
    //m_room.join(shared_from_this());
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
    print_buffer(msg.data(),60);
    deliver_byte((std::byte*)msg.data(),msg.size());
  }

  void check_handshake_hello_ack(ST_HANDSHAKE_HELLO_ACK *st_hello_ack, uint32_t buffersize)
  {
    /*if(!m_room.participant_exist(st_hello_ack->participant_id)){
      //LOG THIS !!!
      this->disconnect();
      }*/
    //m_name = std::string(st_hello_ack->participant_name_buffer);
    std::copy(st_hello_ack->configuration_buffer,st_hello_ack->configuration_buffer+st_hello_ack->configuration_buffersize,std::back_inserter(m_participant_devices_configuration));
      
    ST_HANDSHAKE_CREDENTIALS *hc = new ST_HANDSHAKE_CREDENTIALS();
    //hc->server_certificate_buffer;
    XRMessage msg((uint16_t)EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS,
                  (std::byte*)hc,
                  (uint32_t)sizeof(ST_HANDSHAKE_CREDENTIALS));
    std::cout << "HANDSHAKE_CREDENTIALS" << std::endl;
    print_buffer(msg.data(),60);
    std::cout << std::endl;
    deliver_byte(msg.data(),msg.size());
    delete hc;
  }

  void check_handshake_credentials_ack(ST_HANDSHAKE_CREDENTIALS_ACK *msg)
  {
    if(strcmp((char*)msg->login_buffer,"login")!=0 || strcmp((char*)msg->password_buffer,"password")!=0) {
      disconnect();
    }
    do_read_byte();
    // send pariticpant_join
    std::string welcome_message = "accepted in room";
    m_room.join(shared_from_this());
    ST_PARTICIPANT_JOIN join_ptr;
    join_ptr.participant_id = m_id;
    join_ptr.max_data_rate = 1; // transactions per second
    join_ptr.allow_asynchronous_messages = 0xFFFF;
    join_ptr.message_buffersize = welcome_message.size();
    memcpy(join_ptr.message_buffer,
           welcome_message.c_str(),
           join_ptr.message_buffersize);

    //m_room.init_new_participant(shared_from_this());

    std::cout << "PARTICIPANT_JOIN" << std::endl;
    std::cout << join_ptr << std::endl;
    XRMessage xrmsg((uint16_t)EN_RAW_MESSAGE_HEAD::PARTICIPANT_JOIN,
                    (std::byte*)&join_ptr,
                    (uint32_t)sizeof(ST_PARTICIPANT_JOIN));

    deliver_byte(xrmsg.data(),xrmsg.size());
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

                                std::cout << std::endl << "receiving " << std::to_string(bytes_transferred) << std::endl;
                                XRMessage_ptr message_ptr(new XRMessage(read_buffer_.data(),bytes_transferred,false));
                                xr_message_header *header = message_ptr->get_header();
                                switch((uint16_t)header->head){
                                case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::HANDSHAKE_HELLO_ACK):
                                  {
                                    std::cout << "HANDSHAKE_HELLO_ACK: " << std::endl;
                                    ST_HANDSHAKE_HELLO_ACK *st_hello_ack = (ST_HANDSHAKE_HELLO_ACK*)message_ptr->payload();
                                    print_buffer(read_buffer_.data(),60);
                                    std::cout << std::endl << *st_hello_ack << std::endl;
                                    check_handshake_hello_ack(st_hello_ack,header->buffersize);
                                  }
                                  break;
                                case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS_ACK):
                                  {
                                    std::cout << "HANDSHAKE_CREDENTIALS_ACK" << std::endl;
                                    ST_HANDSHAKE_CREDENTIALS_ACK *msg = static_cast<ST_HANDSHAKE_CREDENTIALS_ACK*>((void*)message_ptr->payload());
                                    print_buffer(read_buffer_.data(),60);
                                    std::cout << std::endl << *msg << std::endl;
                                    check_handshake_credentials_ack(msg);
                                    return;
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
                                //m_room.leave(shared_from_this());
                                disconnect();
                              }
                            });
  }
};

#endif


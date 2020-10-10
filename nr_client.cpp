//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/asio/high_resolution_timer.hpp>
#include "protocol.hpp"
#include "XRMessage.hpp"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

typedef std::deque<nr_message> chat_message_queue;

class chat_client
{
public:
  chat_client(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints) 
    : io_context_(io_context), socket_(io_context), timer_(io_context)
  {
    read_buffer_ = std::shared_ptr<std::vector<std::byte>>(new std::vector<std::byte>());
    //write_buffer_ = std::shared_ptr<std::vector<std::byte>>(new std::vector<std::byte>());
    read_buffer_->resize(1024 * 64);
    //write_buffer_->resize(1024 * 64);
    do_connect(endpoints);
  }

  void write_byte(XRMessage_ptr msg_ptr)
  {
    boost::asio::post(  io_context_,
			[this, msg_ptr]()
			{
			  bool write_in_progress = !deque_write_buffer_.empty();
			  /*std::shared_ptr<std::vector<std::byte>> buffer2(new std::vector<std::byte>());
			  buffer2->resize(1024 * 64);
			  memset(buffer2->data(), 0x00, 1024 * 64);
			  std::copy(buffer.begin(), buffer.end(), std::begin(*buffer2));
			  */
			  deque_write_buffer_.push_back(msg_ptr);
			  //memset((void*)buffer.data(), 0x00, buffer.capacity());
			  if (!write_in_progress)
			    {
			      do_write_byte();
			    }
                                
			});
  }

  /*void write(const nr_message& msg)
    {
    boost::asio::post(io_context_,
    [this, msg]()
    {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
    do_write();
    }
    });
    }*/

  void close()
  {
    boost::asio::post(io_context_, [&]() { socket_.close(); });
    std::cout << "closing" << std::endl;
  }
  
  void begin_participant_update()
  {
    //boost::asio::high_resolution_timer timer(io_context_);
    timer_.expires_after(std::chrono::milliseconds(50));
    timer_.async_wait([&](const boost::system::error_code &ec)
    {
      if(!ec){
	ST_PARTICIPANT_UPDATE_ACK puack;
	puack.participant_id = this->participant_id_;
	puack.origin_timestamp = get_timestamp_now();
	char json_buffer[] = { "{\"m_position\":{\"x\":0.0,\"y\":0.0,\"z\":0.0},\"m_rotation\":{\"x\":0.0,\"y\":0.0,\"z\":0.0,\"w\":0.0},\"m_name\":\"Pedro Urdemales\",\"m_id\":14514284786278117030,\"m_id_hex\":\"C96D191CF6F6AEA6\",\"m_prefab_path\":\"Assets/Resources/XRNetwork/Characters/MarkerMan.fbx\",\"m_auto_update\":false,\"objectType\":4,\"m_devices\":[],\"m_prefab\":{\"instanceID\":13558}}" };
	puack.buffersize = strlen(json_buffer);
	memcpy(puack.buffer,json_buffer,sizeof(json_buffer));
	XRMessage_ptr msg(new XRMessage((uint16_t)EN_RAW_MESSAGE_HEAD::PARTICIPANT_UPDATE_ACK,
					(std::byte*)&puack,
					sizeof(ST_PARTICIPANT_UPDATE)));
	std::cout << ">> UPDATE_PARTICIPANT_ACK" << std::endl
		<< puack << std::endl;
	write_byte(msg);
	begin_participant_update();
      }
    });
  }
  
private:
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
    //std::cout << "connecting to " << socket_.remote_endpoint().address().to_string() << " at port " << socket_.remote_endpoint().port() << std::endl;
    boost::asio::async_connect( socket_, endpoints,
				[this](boost::system::error_code ec, tcp::endpoint)
				{
				  if (!ec)
				    {
				      std::cout << "connected !!!\r\n";
				      //do_read_header();
				      do_read_byte();
				    }
				  else
				    {
				      std::cout << "failed to connect" << std::endl;
				      this->close();
				    }
				});
  }

  void do_read_byte()
  {
    //size_t cap = read_buffer_->capacity();
    //std::cout << "Capacity: " << std::to_string(cap) << " bytes\r\n";
    boost::asio::async_read(
			    socket_,
			    boost::asio::buffer(read_buffer_->data(),read_buffer_->capacity()),
			    [this](boost::system::error_code ec, size_t bytes_transferred) {
			      if (!ec)
				{
				  {  
				    std::cout << std::endl << "receiving " << std::to_string(bytes_transferred) << std::endl;
				    XRMessage_ptr message_ptr(new XRMessage(read_buffer_->data(),bytes_transferred,false));
				    xr_message_header *header = message_ptr->get_header();
				    std::cout << *header << std::endl;
				    switch((uint16_t)header->head){
				    case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::HANDSHAKE_HELLO):
				      {
					ST_HANDSHAKE_HELLO *st_hello_ack = (ST_HANDSHAKE_HELLO*)message_ptr->payload();
					std::cout << "<< HANDSHAKE_HELLO: " << std::endl;
					print_buffer(read_buffer_->data(),60);
					std::cout << std::endl << *st_hello_ack << std::endl;
					
					participant_id_ = st_hello_ack->service_id;
					service_id_ = st_hello_ack->service_id;
					
					ST_HANDSHAKE_HELLO_ACK hhack;
					hhack.participant_id = participant_id_;
					hhack.client_timestamp = get_timestamp_now();
					char configuration[] = { "ABCDEFGHIJKLMNOPQRST" };
					hhack.configuration_buffersize = strlen(configuration); // max buffer size
					bzero(hhack.configuration_buffer,1024*20); // clean the buffer
					
					memcpy((void*)hhack.configuration_buffer,
					       (void*)configuration,
					       strlen(configuration));
					
					XRMessage_ptr msg(new XRMessage((uint16_t)EN_RAW_MESSAGE_HEAD::HANDSHAKE_HELLO_ACK,
									(std::byte*)&hhack,
									sizeof(ST_HANDSHAKE_HELLO_ACK)));
					
					std::cout << std::endl << ">> HANDSHAKE_HELLO_ACK" << std::endl << hhack << std::endl;
					print_buffer(msg->data(),60);
					write_byte(msg);
				      }
				      break;
				      /*case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::HANDSHAKE_HELLO_ACK):
				      {
					std::cout << "<<HANDSHAKE_HELLO_ACK: " << std::endl;
					ST_HANDSHAKE_HELLO_ACK *st_hello_ack = (ST_HANDSHAKE_HELLO_ACK*)message_ptr->payload();
					print_buffer(read_buffer_->data(),60);
					std::cout << std::endl << *st_hello_ack << std::endl;
					//check_handshake_hello_ack(st_hello_ack,header->buffersize);
				      }
				      break;*/
				    case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS):
				      {
					ST_HANDSHAKE_CREDENTIALS *hcred = (ST_HANDSHAKE_CREDENTIALS*)message_ptr->payload();
					std::cout << "<< HANDSHAKE_CREDENTIALS " << std::endl
						  << *hcred << std::endl;
					
					// we suposse to check the credentials here, but ehhh....
					ST_HANDSHAKE_CREDENTIALS_ACK hcack;
					char login[] = { "login" };
					char password[] = { "password" };
					hcack.participant_id = 0x00;
					hcack.login_buffersize = strlen(login);
					memcpy(hcack.login_buffer,login,strlen(login));
					hcack.password_buffersize = strlen(password);
					memcpy(hcack.password_buffer,password,strlen(password));
					
					std::cout << ">> HANDSHAKE_CREDENTIALS_ACK ("
						  << std::to_string(sizeof(ST_HANDSHAKE_CREDENTIALS_ACK)) << ")" << std::endl
						  << hcack << std::endl;
					
					XRMessage_ptr msg(new XRMessage((uint16_t)EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS_ACK,
									(std::byte*)&hcack,
									sizeof(ST_HANDSHAKE_CREDENTIALS)));
					
					write_byte(msg);
				      }
				      break;
				      /*case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::HANDSHAKE_CREDENTIALS_ACK):
				      {
					std::cout << "HANDSHAKE_CREDENTIALS_ACK" << std::endl;
					ST_HANDSHAKE_CREDENTIALS_ACK *msg = (ST_HANDSHAKE_CREDENTIALS_ACK*)message_ptr->payload();
					print_buffer(read_buffer_->data(),60);
					std::cout << std::endl << *msg << std::endl;
					//check_handshake_credentials_ack(msg);
					return;
				      }
				      break;*/
				    case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::PARTICIPANT_JOIN):
				      {
					ST_PARTICIPANT_JOIN *pj = (ST_PARTICIPANT_JOIN*)message_ptr->payload();
					std::cout << "<< PARTICIPANT_JOIN" << std::endl
						  << *pj << std::endl;
					participant_id_ = pj->participant_id;// the only important hting on this message
					
					begin_participant_update();
				      }
				      break;
				    case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::PARTICIPANT_NEW):
				      {
					ST_PARTICIPANT_NEW *np = (ST_PARTICIPANT_NEW*)message_ptr->payload();
					std::cout << "<< NEW PARtICIPANT" << std::endl
						  << *np << std::endl;
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
					ST_PARTICIPANT_UPDATE_ACK *puack = (ST_PARTICIPANT_UPDATE_ACK*)message_ptr->payload();
					std::cout << "<< PARTICIPANT_UPCATE_ACK " << std::endl
						  << *puack << std::endl;
					
				      }
				      break;
				    default:
				      //LOG_WARNING("meader message not recognized.");
				      break;
				    };
				  }
				  read_buffer_->clear();
				  do_read_byte();
				}
			      else {
				std::cerr << "do_read error";
				this->close();
			      }
			    });
  }
  
  /*void do_read_header()
    {
    boost::asio::async_read(socket_,
    boost::asio::buffer(read_msg_.data(), nr_message::header_length),
    [this](boost::system::error_code ec, std::size_t lenght)
    {
    if (!ec && read_msg_.decode_header())
    {
    do_read_body();
    }
    else
    {
    std::cout << "disconnected" << std::endl;
    //socket_.close();
    this->close();
    }
    });
    }

    void do_read_body()
    {
    boost::asio::async_read(socket_,
    boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
    [this](boost::system::error_code ec, std::size_t length)
    {
    if (!ec)
    {
    std::cout.write(read_msg_.body(), read_msg_.body_length());
    std::cout << "\n";
    do_read_header();
    }
    else
    {
    std::cout << "async_read error" << std::endl;
    //socket_.close();
    this->close();
    }
    });
    }*/

  void do_write_byte()
  {
    //std::cout << "capacity: " << deque_write_buffer_.front()->capacity() << std::endl;
    boost::asio::async_write(socket_,
			     boost::asio::buffer(deque_write_buffer_.front()->data(), deque_write_buffer_.front()->size()),
			     [this](boost::system::error_code ec, std::size_t sentbytes)
			     {
			       if (!ec)
				 {
				   deque_write_buffer_.pop_front();
				   if (!deque_write_buffer_.empty())
				     {
				       do_write_byte();
				     }
				   std::cout << " sent " << std::to_string(sentbytes) << " bytes\r\n";
				 }
			       else
				 {
				   std::cout << "do_write error" << std::endl;
				   this->close();
				 }
			     });
  }

  /* void do_write()
     {
     boost::asio::async_write(socket_,
     boost::asio::buffer(write_msgs_.front().data(),write_msgs_.front().length()),
     [this](boost::system::error_code ec, std::size_t length)
     {
     if (!ec)
     {
     write_msgs_.pop_front();
     if (!write_msgs_.empty())
     {
     do_write();
     }
     std::cout << " sent " << std::to_string(lenght) << " bytes\r\n";
     write_buffer_->clear();;
     }
     else
     {
     std::cout << "do_write error" << std::endl;
     this->close();
     }
     });
     }*/

private:
  boost::asio::io_context& io_context_;
  tcp::socket socket_;
  //udp::socket udp_socket_;
  std::shared_ptr<std::vector<std::byte>> write_buffer_;
  std::shared_ptr<std::vector<std::byte>> read_buffer_;
  
  std::deque<XRMessage_ptr> deque_write_buffer_;
  std::deque<XRMessage_ptr> deque_read_buffer_;
  uint64_t participant_id_;
  uint64_t service_id_;
  nr_message read_msg_;
  chat_message_queue write_msgs_;
  boost::asio::high_resolution_timer timer_;
}; // end chat_client

int main(int argc, char* argv[])
{
  try
    {
      boost::asio::io_context io_context;

      tcp::resolver resolver(io_context);
      //auto endpoints = resolver.resolve(argv[1], argv[2]);
      //auto endpoints = resolver.resolve("localhost", "1080");
      auto endpoints = resolver.resolve("localhost", "1080");
      chat_client c(io_context, endpoints);
        
      std::thread t([&io_context]() { io_context.run(); });

      //char line[nr_message::max_body_length + 1];
      char line[64 * 1024];
      std::vector<std::byte> ptr;
      while (std::cin.getline(line, 1024) )
        {
	  /*nr_message msg;
            msg.body_length(std::strlen(line));
            std::memcpy(msg.body(), line, msg.body_length());
            msg.encode_header();
            c.write(msg);*/
	  ptr.insert(std::begin(ptr),(std::byte*)line, (std::byte*)line + strlen(line));
	  //c.write_byte(ptr);
	  ptr.clear();
	  memset(line, 0x00, 64 * 1024);
	  //407 273 9676
        }

      c.close();
      t.join();
    }
  catch (std::exception & e)
    {
      std::cerr << "Exception: " << e.what() << "\n";
    }

  return 0;
}

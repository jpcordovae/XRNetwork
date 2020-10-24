#ifndef SESSION_H__
#define SESSION_H__

#include "nr_base.hpp"
#include "room.hpp"
#include "XRMessage.hpp"
#include "map"

class nr_session : public nr_participant, public std::enable_shared_from_this<nr_session>
{
public:
  typedef std::vector<std::byte> buffer_type;
  typedef std::shared_ptr<buffer_type> buffer_type_ptr;
  typedef std::deque<buffer_type_ptr> deque_buffer_type_ptr;
  typedef std::shared_ptr<nr_session> nr_session_ptr;

  nr_session(std::shared_ptr<tcp::socket> socket,
             network_room& room,
             const uint64_t &service_id,
             const std::string &server_name) : m_socket_ptr(std::move(socket)),
                                               m_room(room),
                                               keep_alive(true),
                                               m_service_id(service_id),
                                               m_server_name(server_name)
  {
    //TODO: start timeout
    read_buffer_.reserve(64 * 1024);
    write_buffer_.reserve(64 * 1024);
  }

  ~nr_session()
  {
  }

  void start()
  {
    IP = m_socket_ptr->remote_endpoint().address().to_string();
    m_room.join(shared_from_this());
    do_read_byte();
  }

  /*void deliver(const nr_message& msg)
  {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress) {
      do_write();
    }
    }*/

  void deliver_byte(std::byte* buffer, size_t buffersize)
  {
    deliver_byte(buffer_type_ptr(new buffer_type(buffer,buffer+buffersize)));
  }

  void deliver_byte(buffer_type buffer)
  {
    deliver_byte(buffer_type_ptr(new buffer_type(buffer)));
  }

  void deliver_byte(buffer_type_ptr buffer_ptr)
  {
    boost::asio::post(m_socket_ptr->get_executor(),
		      [this,buffer_ptr]()
		      {
			m_deque_write_buffer_mutex.lock();
			bool write_byte_in_progress_ = !deque_write_buffer_.empty();
			deque_write_buffer_.push_back(buffer_ptr);
			m_deque_write_buffer_mutex.unlock();
			
			if (!write_byte_in_progress_) {
			  do_write_byte();
			}
		      });
  }
  
  void close()
  {
    do_close();
  }

  void disconnect()
  {
    do_close();
  }

  int set_keep_alive(bool keep_alive)
  {
    boost::asio::socket_base::keep_alive o(keep_alive);
    m_socket_ptr->set_option(boost::asio::socket_base::keep_alive(o));
    return NR_OK;
  }

  bool get_keep_alive()
  {
    boost::asio::socket_base::keep_alive o;
    m_socket_ptr->get_option(o);
    return o.value();
  }

protected:
  void do_read_byte()
  {
    auto self(shared_from_this());
    boost::asio::async_read(*m_socket_ptr,
                            boost::asio::buffer(read_buffer_.data(), read_buffer_.capacity()),
                            [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
			      
                              if (!ec) {
                                //std::cout << std::endl << "receiving " << std::to_string(bytes_transferred) << " bytes" << std::endl;
                                XRMessage_ptr message_ptr(new XRMessage(read_buffer_.data(),bytes_transferred,false));
                                xr_message_header *header = message_ptr->get_header();
                                //print_buffer(read_buffer_.data(),60);
                                //std::cout << *header << std::endl;
                                //std::cout << std::endl;
                                switch((uint16_t)header->head){
                                case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::PARTICIPANT_JOIN_ACK):
				  {
				    ST_PARTICIPANT_JOIN_ACK *pjack = static_cast<ST_PARTICIPANT_JOIN_ACK*>((void*)message_ptr->payload());
				    std::cout << std::endl << "<< PARTICIPANT_JOIN_ACK" << std::endl << *pjack << std::endl;
				    m_descriptor.clear();
				    // save descriptor of the participant, is client platform dependent, for the server is a bunch of bytes
				    std::copy(pjack->json_buffer,pjack->json_buffer+pjack->json_buffersize,std::back_inserter(m_descriptor));
				    m_room.init_new_participant(pjack->participant_id);
				  }
				  break;
                                case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::PARTICIPANT_NEW_ACK):
                                  break;
                                case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::PARTICIPANT_UPDATE_ACK):
                                  {
                                    //std::cout << std::endl << "<< PARTICIPANT_UPDATE_ACK" << std::endl;
                                    //ST_PARTICIPANT_UPDATE_ACK *upd = static_cast<ST_PARTICIPANT_UPDATE_ACK*>((void*)message_ptr->payload());
				    //std::cout << std::endl << *upd << std::endl;
				    m_room.deliver_to_all_except_to_one(message_ptr,m_id);
                                  }
                                  break;
				case static_cast<unsigned int>(EN_RAW_MESSAGE_HEAD::PARTICIPANT_LEAVE_ACK):
				  {
				    //ST_PARTICIPANT_LEAVE_ACK *leave = static_cast<ST_PARTICIPANT_LEAVE_ACK*>((void*)message_ptr->payload());
				    //m_room.disconnect_participant(m_id);
				    m_room.leave(shared_from_this());
				  }
				  break;
                                default:
                                  //disconnect();
				  //m_room.disconnect_participant(m_id);
				  m_room.leave(shared_from_this());
				  break;
                                }
                                do_read_byte();
                              }
                              else {
                                //m_room.leave(shared_from_this());
				do_close();
                              }
                            });
  }

private:

  void do_close()
  {
    try {
      //if (socket_.is_open()) {
      if(m_socket_ptr->is_open()) {
        std::cout << "nr_session::do_close(): disconnecting participant " << m_id << std::endl;
        //log this
        //LOG_NOTIFICATION(std::string("disconnecting participant ") << ID_);
        boost::system::error_code errorcode;
        //socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, errorcode);
        m_socket_ptr->shutdown(boost::asio::ip::tcp::socket::shutdown_both, errorcode);
        if (errorcode) {
          //trace("Closing failed: ", errorcode.message());
          std::cerr << "nr_session do_close shutdown socket error: " << errorcode.message();
        }
        //socket_.close(errorcode);
        m_socket_ptr->close(errorcode);
        if (errorcode) {
          //trace("Closing2 failed: ", errorcode.message());
          std::cerr << "nr_session do_close closing socket error: " << errorcode.message();
        }
	m_room.leave(shared_from_this());
      }
    } catch(std::exception &e) {
      std::cerr << e.what();
    }
  }

  /*
  void do_read_header()
  {
    // decoding header
    auto self(shared_from_this());
    boost::asio::async_read(*m_socket_ptr,
                            boost::asio::buffer(read_msg_.data(), nr_message::header_length),
                            [this, self](boost::system::error_code ec, std::size_t ){
                              if (!ec && read_msg_.decode_header()) {
                                do_read_body();
                              }
                              else {
                                std::cout << "nr_session:do_read_body error" << std::endl;
                                m_room.leave(shared_from_this());
                              }
                            });
  }

  void do_read_body()
  {
    auto self(shared_from_this());
    boost::asio::async_read(*m_socket_ptr,
                            boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                            [this, self](boost::system::error_code ec, std::size_t ) {
                              if (!ec) {
                                //add_message(,message_buffer_.push_back(read_msg_));
                                m_room.deliver(read_msg_);
                                do_read_header();
                              }
                              else{
                                std::cout << "nr_session:do_read_body error" << std::endl;
                                m_room.leave(shared_from_this());
                              }
                            });
  }
  */
  
  void do_write_byte()
  {
    auto self(shared_from_this());
    //std::cout << std::to_string(deque_write_buffer_.front()->size()) << " bytes to write" << std::endl;
    m_deque_write_buffer_mutex.lock();
    buffer_type_ptr btp = deque_write_buffer_.front();
    deque_write_buffer_.pop_front();
    m_deque_write_buffer_mutex.unlock();
    
    boost::asio::async_write(*m_socket_ptr,
                             boost::asio::buffer(btp->data(),
                                                 btp->size()),
			                         [this, self](boost::system::error_code ec, std::size_t bytes_writen) {
						   if (!ec) {
						     m_deque_write_buffer_mutex.lock();
						     bool queue_is_empty = deque_write_buffer_.empty();
						     m_deque_write_buffer_mutex.unlock();
						     //deque_write_buffer_.pop_front();
						     if (!queue_is_empty) {
						       do_write_byte();
						     }
						   }
						   else {
						     std::cerr << "session.hpp:do_write error at line " << __LINE__ << std::endl;
						     //m_room.leave(shared_from_this());
						     do_close();
						   }
						 });
    
    /*
    boost::asio::async_write(*m_socket_ptr,
                             boost::asio::buffer(deque_write_buffer_.front()->data(),
                                                 deque_write_buffer_.front()->size()),
			                         [this, self](boost::system::error_code ec, std::size_t bytes_writen) {
						   if (!ec) {
						     std::cout << std::to_string(bytes_writen) << " bytes written" << std::endl;
						     deque_write_buffer_.pop_front();
						     if (!deque_write_buffer_.empty()) {
						       do_write_byte();
						     }
						   }
						   else {
						     std::cerr << "nr_session:do_write error" << std::endl;
						     m_room.leave(shared_from_this());
						   }
						   });*/
  }

protected:

  // network
  std::shared_ptr<tcp::socket> m_socket_ptr;
  network_room& m_room;
  bool keep_alive;
  // buffers
  buffer_type read_buffer_;
  buffer_type write_buffer_;
  // 
  std::mutex m_deque_write_buffer_mutex;
  deque_buffer_type_ptr deque_write_buffer_;
  deque_buffer_type_ptr deque_read_buffer_;
  nr_message read_msg_;
  nr_message_queue write_msgs_;
  uint64_t m_service_id;
  std::string m_server_name;
  // XRMessage queue
  std::vector<XRMessage> m_message_queue;
};// END nr_session

typedef nr_session::nr_session_ptr nr_session_ptr;


#endif

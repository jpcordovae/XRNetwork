#ifndef SESSION_H__
#define SESSION_H__

#include "nr_base.hpp"
#include "room.hpp" 

class nr_session : public nr_participant, public std::enable_shared_from_this<nr_session>
{
public:
  typedef std::vector<std::byte> buffer_type;
  typedef std::shared_ptr<buffer_type> buffer_type_ptr;
  typedef std::deque<buffer_type_ptr> deque_buffer_type_ptr;
  typedef std::shared_ptr<nr_session> nr_session_ptr;

  nr_session(tcp::socket socket, network_room& room) : socket_(std::move(socket)), room_(room), keep_alive(true)
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
    IP = socket_.remote_endpoint().address().to_string();
    room_.join(shared_from_this());
    //do_read_header();
    do_read_byte();
  }

  void deliver(const nr_message& msg)
  {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
      {
	do_write();
      }
  }

  void deliver_byte(std::byte* buffer, size_t buffersize)
  {
    bool write_in_progress = !deque_write_buffer_.empty();
    deque_write_buffer_.push_back(buffer_type_ptr(new buffer_type(buffer,buffer+buffersize)));
    if (!write_in_progress)
      {
	do_write_byte();
      }
  }

  void deliver_byte(buffer_type buffer)
  {
    bool write_in_progres = !deque_write_buffer_.empty();
    deque_write_buffer_.push_back(buffer_type_ptr(new buffer_type(buffer)));
    if (!write_in_progres)
      {
	do_write_byte();
      }
  }

  void deliver_ptr(buffer_type_ptr buffer_ptr)
  {
    bool write_byte_in_progress_ = !deque_write_buffer_.empty();
    deque_write_buffer_.push_back(buffer_ptr);
    if (!write_byte_in_progress_)
      {
	do_write_byte();
      }
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
    socket_.set_option(boost::asio::socket_base::keep_alive(o));
    return NR_OK;
  }

  bool get_keep_alive() {
    boost::asio::socket_base::keep_alive o;
    socket_.get_option(o);
    return o.value();
  }

private:

  void do_close()
  {
    if (socket_.is_open()) {
      std::cout << "disconnecting participant " << ID_ << " socket id: " << std::endl;
      boost::system::error_code errorcode;
      socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, errorcode);
      if (errorcode) {
	//trace("Closing failed: ", errorcode.message());
	std::cerr << "nr_session do_close  shutdown socket error: " << errorcode.message();
      }
      socket_.close(errorcode);
      if (errorcode) {
	//trace("Closing2 failed: ", errorcode.message());
	std::cerr << "nr_session do_close closing socket error: " << errorcode.message();
      }
    }
  }

  void do_read_byte()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
			    boost::asio::buffer(read_buffer_.data(), read_buffer_.capacity()),
			    [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
			      if (!ec) {
				size_t cap = read_buffer_.capacity();
				// transfer completed
				room_.new_message(this->ID_, read_buffer_.data(), (uint32_t)read_buffer_.capacity());
				if (room_.get_broadcast_messages()) room_.deliver_byte(read_buffer_.data(),read_buffer_.capacity());
				memset(read_buffer_.data(), 0x00, read_buffer_.capacity());
				do_read_byte();
			      }
			      else
				{
				  room_.leave(shared_from_this());
				}

			    });
  }

  void do_read_header()
  {
    // decoding header
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
			    boost::asio::buffer(read_msg_.data(), nr_message::header_length),
			    [this, self](boost::system::error_code ec, std::size_t )
			    {
			      if (!ec && read_msg_.decode_header())
				{
				  do_read_body();
				}
			      else
				{
				  std::cout << "nr_session:do_read_body error" << std::endl;
				  room_.leave(shared_from_this());
				}
			    });
  }

  void do_read_body()
  {
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
			    boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
			    [this, self](boost::system::error_code ec, std::size_t )
			    {
			      if (!ec)
				{
				  //add_message(,message_buffer_.push_back(read_msg_));
				  room_.deliver(read_msg_);
				  do_read_header();
				}
			      else
				{
				  std::cout << "nr_session:do_read_body error" << std::endl;
				  room_.leave(shared_from_this());
				}
			    });
  }

  void do_write()
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
			     boost::asio::buffer(write_msgs_.front().data(),write_msgs_.front().length()),
			     [this, self](boost::system::error_code ec, std::size_t /*length*/)
			     {
			       if (!ec)
				 {
				   write_msgs_.pop_front();
				   if (!write_msgs_.empty())
				     {
				       do_write();
				     }
				 }
			       else
				 {
				   std::cout << "nr_session:do_write error" << std::endl;
				   room_.leave(shared_from_this());
				 }
			     });
  }

  void do_write_byte()
  {
    auto self(shared_from_this());
    boost::asio::async_write(   socket_,
				boost::asio::buffer(deque_write_buffer_.front()->data(), deque_write_buffer_.front()->capacity()),
				[this, self](boost::system::error_code ec, std::size_t bytes_writen)
				{
				  if (!ec)
				    {
				      deque_write_buffer_.pop_front();
				      if (!deque_write_buffer_.empty())
					{
					  do_write_byte();
					}
				    }
				  else
				    {
				      std::cout << "nr_session:do_write error" << std::endl;
				      room_.leave(shared_from_this());
				    }
				});

  }

  tcp::socket socket_;
  network_room& room_;
    
  buffer_type read_buffer_;
  buffer_type write_buffer_;
  deque_buffer_type_ptr deque_write_buffer_;
  deque_buffer_type_ptr deque_read_buffer_;

  nr_message read_msg_;
  nr_message_queue write_msgs_;
  // features
  bool keep_alive;
};// end nr_session

typedef nr_session::nr_session_ptr nr_session_ptr;


#endif

#ifndef NR_SERVER__H
#define NR_SERVER__H

#include "nr_base.hpp"
#include "message_node.hpp"
#include "participant.hpp"
#include "room.hpp"
#include "session.hpp"
#include "handshake.hpp"

#define DEFAULT_MAX_PARTICIPANTS 10

/*void fail_report(boost::system::error_code &ec, char const *what) 
  {
  // make log of this
  std::cerr << what << ": " << ec.message() << std::endl;
  }*/

class nr_server : public std::enable_shared_from_this<nr_server>
{
public:
  typedef std::shared_ptr<nr_server> nr_server_ptr;

  nr_server(boost::asio::io_context& io_context, const tcp::endpoint& endpoint) : acceptor_(io_context, endpoint),
                                                                                  b_server_has_started_(false)
  {
    //context = std::make_shared<boost::asio::io_context&>(io_context);
    room_.set_max_participants(DEFAULT_MAX_PARTICIPANTS);
    boost::system::error_code ec;
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    /*if (ec)
      {
      std::cout << "error at constructor nr_server " << ec.message() << std::endl;
      return;
      }*/
    io_context.post([&]() {
                      std::unique_lock<std::mutex> lock(server_has_started_mutex_);
                      while (!b_server_has_started_) server_has_started_.wait(lock);
                    });
    m_service_id = rng();
    //std::cout << "service ID: " << std::to_string(m_service_id) << std::endl;
    do_accept();
  }

  typedef struct ST_CREDENTIALS{
    std::string login;
    std::string password;
  } ST_CREDENTIALS;

  void set_credentials(const std::string login, const std::string password)
  {
    credentials.login = login;
    credentials.password = password;
  }

  void set_credentials(const ST_CREDENTIALS &credentials_)
  {
    credentials.login = credentials_.login;
    credentials.password = credentials_.password;
  }

  bool verify_credentials(const ST_CREDENTIALS &credentials_)
  {
    if(credentials.login.compare(credentials_.login) == 0 && credentials.password.compare(credentials_.password) == 0){
      return true;
    }
    return false;
  }

  void set_service_name(std::string service_name_)
  {
    m_service_name = service_name_;
    std::cout << "service name: " << m_service_name << std::endl;
  }

  std::string  get_service_name()
  {
    return m_service_name;
  }

  void set_keep_alive(bool keep_alive)
  {
    room_.set_keep_alive(keep_alive);
  }

  bool get_kep_alive()
  {
    return room_.get_keep_alive();
  }

  uint16_t participants_count()
  {
    return room_.participants_count();
  }

  void set_max_participants(uint16_t max_participants)
  {
    room_.set_max_participants(max_participants);
  }

  void set_max_handshake_connections(uint16_t max_connections)
  {
    m_handshake_room.set_max_participants(max_connections);
  }

  uint16_t get_max_participants()
  {
    return room_.get_max_participants();
  }

  void register_callback_new_participant(new_participant_callback callback)
  {
    room_.register_callback_new_participant(callback);
  }

  void register_callback_new_message(new_message_callback callback)
  {
    room_.register_callback_new_message(callback);
  }

  void register_callback_participant_leave(participant_leave_callback plc)
  {
    room_.register_callback_participant_leave(plc);
  }

  void disconnect_participant(uint64_t participant_id)
  {
    room_.disconnect_participant(participant_id);
  }

  void disconnect_all_participants()
  {
    room_.disconnect_all_participants();
  }

  void get_participant_info(uint64_t participant_id, nr_participant_info* pinfo)
  {
    assert(pinfo != nullptr);
    room_.get_participant_info(participant_id, pinfo);
  }

  void set_broadcasting_all_messages(bool br)
  {
    room_.set_broadcast_messages(br);
  }

  bool get_broadcasintg_all_messages()
  {
    return room_.get_broadcast_messages();
  }

  void disconnect_all_callbacks()
  {
    room_.disconnect_all_callbacks();
  }

  bool has_started()
  {
    return b_server_has_started_;
  }

  void stop()
  {
    room_.disconnect_all_participants();
    if (acceptor_.is_open()) {
      acceptor_.close();
    }
  }

  // participants functions
  void set_participant_deaf(const uint64_t participant_id, const bool deaf)
  {
    room_.set_participant_deaf(participant_id,deaf);
  }

  bool get_participant_deaf(const uint64_t participant_id)
  {
    return room_.get_participant_deaf(participant_id);
  }

  void set_auto_update_participants(bool au)
  {
    room_.set_auto_update_participants(au);
  }

  bool get_auto_update_participants()
  {
    return room_.get_auto_update_participants();
  }

private:

  void do_accept()
  {
    if (!b_server_has_started_) {
      std::lock_guard<std::mutex> guard(server_has_started_mutex_);
      b_server_has_started_ = true;
      server_has_started_.notify_all();
    }

    auto self(this);
    acceptor_.async_accept(
			   [this,self](boost::system::error_code ec, tcp::socket socket)
			   {
			     if (!ec) {
                   std::shared_ptr<tcp::socket> socket_ptr = std::make_shared<tcp::socket>(std::move(socket));
                   std::make_shared<handshake_session>(socket_ptr, room_, m_service_id, m_service_name)->start();
                   do_accept();
                 }
			     else {
			       std::cout << "nr_server::do_accept::acceptor_::async_accept : " << ec.message() << std::endl;
			     }
			   });
  }

  tcp::acceptor acceptor_;
  network_room room_;
  network_room m_handshake_room;
  // has started condition variable
  std::mutex server_has_started_mutex_;
  std::condition_variable server_has_started_;
  std::atomic_bool b_server_has_started_;
  std::atomic_bool b_started_sync;
  std::atomic_bool b_require_credentials;
  ST_CREDENTIALS credentials;
  uint64_t m_service_id;
  std::string m_service_name;
  std::mt19937_64 rng;
  std::shared_ptr<tcp::socket> m_socket;
};

typedef nr_server::nr_server_ptr nr_server_ptr;

#endif // NR_SERVER__H

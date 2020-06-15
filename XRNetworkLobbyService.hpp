#ifndef XRNETWORKLOBBY_H__
#define XRNETWORKLOBBY_H__

#include "nr_server.hpp"
//#include <boost/asio/ssl.hpp>
//namespace ssl = boost::asio::ssl;
using  tcp = boost::asio::ip::tcp;

typedef void(*on_running_callback)();
typedef void(*on_stopping_callback)();

class XRNetworkLobbyService //: public std::enable_shared_from_this<XRNetworkLobbyService>
{
public:
  typedef std::shared_ptr<XRNetworkLobbyService> xrnls_ptr;
  typedef std::shared_ptr<std::thread> thread_ptr;

  enum class PROTOCOL { TCP, UDP } PROTOCOL_ = PROTOCOL::TCP;
  enum class PROTOCOL_VERSION { V4 = 4, V6 = 6 } PROTOCOL_VERSION_ = PROTOCOL_VERSION::V4;

  XRNetworkLobbyService() : 
    b_thread_running_(false),
    port_(1080),
    max_participants_(5),
    b_socket_no_delay_(true),
    b_keep_alive_(true),
    message_buffer_size_(100),
    b_auto_broadcast_all_incoming_messages(false),
    id_(rand64b())
  {
    service_name_ = std::string("service_") + std::to_string(uint16_t(rand64b()));
  }

  XRNetworkLobbyService(std::string name,
			int port,
			uint16_t max_participants,
			std::chrono::duration<double> message_timeout,
			uint32_t message_buffer_size)
    : b_thread_running_(false),
      service_name_(name),
      port_(port),
      max_participants_(max_participants),
      buffer_message_timeout_(message_timeout),
      message_buffer_size_(message_buffer_size),
      b_keep_alive_(false),
      id_(rand64b())
  {
  }

  /*
    +-----------------------+-------------------+-----------------------------+------------------------+
    |       generator       | length of cycle   | approx. memory requirements | approx. relative speed |
    +-----------------------+-------------------+-----------------------------+------------------------+
    | minstd_rand           | 2^31-2            | sizeof(int32_t)             |                     40 |
    | rand48                | 2^48-1            | sizeof(uint64_t)            |                     80 |
    | lrand48 (C library)   | 2^48-1            | -                           |                     20 |
    | ecuyer1988            | approx. 2^61      | 2*sizeof(int32_t)           |                     20 |
    | kreutzer1986          | ?                 | 1368*sizeof(uint32_t)       |                     60 |
    | hellekalek1995        | 2^31-1            | sizeof(int32_t)             |                      3 |
    | mt11213b              | 2^11213-1         | 352*sizeof(uint32_t)        |                    100 |
    | mt19937               | 2^19937-1         | 625*sizeof(uint32_t)        |                    100 |
    | lagged_fibonacci607   | approx. 2^32000   | 607*sizeof(double)          |                    150 |
    | lagged_fibonacci1279  | approx. 2^67000   | 1279*sizeof(double)         |                    150 |
    | lagged_fibonacci2281  | approx. 2^120000  | 2281*sizeof(double)         |                    150 |
    | lagged_fibonacci3217  | approx. 2^170000  | 3217*sizeof(double)         |                    150 |
    | lagged_fibonacci4423  | approx. 2^230000  | 4423*sizeof(double)         |                    150 |
    | lagged_fibonacci9689  | approx. 2^510000  | 9689*sizeof(double)         |                    150 |
    | lagged_fibonacci19937 | approx. 2^1050000 | 19937*sizeof(double)        |                    150 |
    | lagged_fibonacci23209 | approx. 2^1200000 | 23209*sizeof(double)        |                    140 |
    | lagged_fibonacci44497 | approx. 2^2300000 | 44497*sizeof(double)        |                     60 |
    +-----------------------+-------------------+-----------------------------+------------------------+
  */

  uint64_t rand64b()
  {
    std::mt19937_64 prng;
    std::random_device device;
    uint64_t seed = (static_cast<uint64_t>(device()) << 32) | device();
    prng.seed(seed);
    return prng();
  }

  void start()
  {
    //TODO: log start command 
    std::cout << "------ start event -------\n";
    try {
      if (!b_thread_running_) {
	service_thread_ptr_ = thread_ptr(new std::thread(&XRNetworkLobbyService::thread_function, this));
      }
    }
    catch (std::exception & e) {
      std::cerr << "XRNetworkLobbyService start() : " << e.what() << std::endl;
    }
  }

  void stop()
  {
    //TODO: log stop command
    std::cout << " ------ stop event ------\n";
    try {
      if (b_thread_running_) {
	OnStopping();
	server->stop();
	io_context_.stop();
	service_thread_ptr_->join();
      }
    }
    catch (std::exception & e) {
      std::cerr << "XRNetworkLobbyService stop() : " << e.what() << std::endl;
    }
  }

  bool is_running() { return b_thread_running_; }

  void set_port(int port)
  {
    std::cout << "----- port " << port << std::endl;
    if (!b_thread_running_)
      port_ = port;
  }

  int get_port() { return port_; }

  void set_protocol_tcp() { PROTOCOL_ = PROTOCOL::TCP; }
  void set_protocol_udp() { PROTOCOL_ = PROTOCOL::UDP; }
  int get_protocol() { return (PROTOCOL_ == PROTOCOL::TCP ? 0 : 1); }

  void set_protocol_version(int prot) { prot == 4 ? PROTOCOL_VERSION_ = PROTOCOL_VERSION::V4 : PROTOCOL_VERSION_ = PROTOCOL_VERSION::V6; }
  void set_protocol_v4() { PROTOCOL_VERSION_ = PROTOCOL_VERSION::V4; }
  void set_protocol_v6() { PROTOCOL_VERSION_ = PROTOCOL_VERSION::V6; }
  int get_protocol_version() { return (PROTOCOL_VERSION_ == PROTOCOL_VERSION::V4 ? 4 : 6); }

  int set_max_participants(uint16_t max_participants) {
    try {
      max_participants_ = max_participants;
      if (b_thread_running_) {
        server->set_max_participants(max_participants_);
        return NR_OK;
      }
      return NR_FAIL;
    }
    catch (std::exception &e) {
      std::cout << "set_max_participants exception: " << e.what() << std::endl;
    }
    return NR_FAIL;
  }

  int set_max_handshake_connections(uint16_t max_connections)
  {
    try {
      m_handshake_max_connections = max_connections;
      if (b_thread_running_) {
        server->set_max_handshake_connections(max_connections);
        return NR_OK;
      }
      return NR_FAIL;
    }catch(std::exception &e) {
      std::cout << "set_max_participants exception: " << e.what() << std::endl;
    }
  }

  uint16_t get_max_participants() { return max_participants_; }
  
  bool get_socket_no_delay() { return b_socket_no_delay_; }
  void set_socket_no_delay(bool socket_no_delay) { b_socket_no_delay_ = socket_no_delay; }

  int set_keep_alive(bool keep_alive) { b_keep_alive_ = keep_alive; return NR_OK; }
  bool get_keep_alive() { return b_keep_alive_; }

  int set_broadcasting_all_messages(bool b)
  {
    b_auto_broadcast_all_incoming_messages = b;
    return NR_OK;
  }

  bool get_broadcasting_all_messages() {
    return b_auto_broadcast_all_incoming_messages;
  }

  uint64_t get_id() { return id_; }

  void register_callback_new_participant(new_participant_callback callback)
  {
    std::cout << "----- new participant callback register\n";
    if (b_thread_running_)
      server->register_callback_new_participant(callback);
  }

  void register_callback_new_message(new_message_callback callback)
  {
    std::cout << "----- new message callback register\n";
    if (b_thread_running_)
      {
	server->register_callback_new_message(callback);
      }
  }

  void register_callback_on_running(on_running_callback callback)
  {
    OnRunning.connect(callback);
  }

  void register_callback_participant_leave(participant_leave_callback plc)
  {
    std::cout << "----- participant leave callback register\n";
    if (b_thread_running_)
      {
	server->register_callback_participant_leave(plc);
      }
    else
      {
	std::cerr << "service instantiation thread not running";
      }
  }

  void register_callback_on_stopping(on_stopping_callback osc)
  {
    OnStopping.connect(osc);
  }

  void disconnect_all_callbacks()
  {
    std::cout << "----- disconnecting all callbacks\n";
    try {
      if (b_thread_running_) {
	server->disconnect_all_callbacks();
	//OnRunning.disconnect_all_slots();
      }
    }
    catch (std::exception & e) {
      std::cerr << "XRNEtwofkLobbyService disconnect_all_callbacks : " << e.what();
    }
  }

  void disconnect_all_participants()
  {
    std::cout << "----- disconnecting all participants\n";
    try {
      if (b_thread_running_) {
	server->disconnect_all_participants();
      }
    }
    catch (std::exception & e) {
      std::cerr << "XRNEtwofkLobbyService disconnect_all_participants : " << e.what();
    }
  }

  uint16_t participants_count()
  {
    try {
      if (b_thread_running_)
	return server->participants_count();
    }
    catch (std::exception &e) {
      std::cerr << "XRNetworkLobbyService get_number_of_participants : " << e.what();
    }
    return 0;
  };

  void get_participant_info(const uint64_t participant_id, nr_participant_info* pinfo)
  {
    assert(pinfo != nullptr);
    try {
      if (b_thread_running_)
	server->get_participant_info(participant_id, pinfo);
    }
    catch (std::exception & e) {
      std::cerr << "XRNetworkLobbyService get_participant_info : " << e.what();
    }
  }

  bool server_has_started()
  {
    try {
      if (b_thread_running_)
	return server->has_started();
    }
    catch (std::exception & e) {
      std::cerr << "XRNetworkLobbyService get_participant_info : " << e.what();
    }
    return false;
  }

  void set_service_name(std::string name) { service_name_ = name; }
  std::string get_service_name() { return service_name_; }

  ~XRNetworkLobbyService()
  {
    if (b_thread_running_)
      {
	this->stop();
      }
  }

  // participants_functions
  int set_participant_deaf(const uint64_t participant_id, const bool deaf)
  {
    if(! b_thread_running_) return NR_FAIL;
    server->set_participant_deaf(participant_id,deaf);
    return NR_OK;
  }

  int get_participant_deaf(const uint64_t participant_id, bool &deaf)
  {
    if(!b_thread_running_) return NR_FAIL;
    deaf = server->get_participant_deaf(participant_id);
    return NR_OK;
  }

  /*int get_participant_info(uint64_t participant_id, nr_participant_info *info)
  {
    if(!b_thread_running_) return NR_FAIL;
    server->get_participant_info(participant_id,info);
    return NR_OK;
    }*/
  
private:

  void thread_function()
  {
    std::cout << "----- start thread XRNetworkLobbyService -----\n";
    try {
      //TODO: log thread start
      io_context_.restart();
      // TODO: check port value and log any problem
      tcp::endpoint tcp_endpoint = tcp::endpoint(tcp::v4(), port_);
      //tcp::endpoint tcp_endpoint = tcp::endpoint(tcp::v6(), port_);
      server = std::shared_ptr<nr_server>(new nr_server(io_context_, tcp_endpoint));
      b_thread_running_ = true;
      while (!server->has_started()) {}
      OnRunning();
      server->set_keep_alive(b_keep_alive_);
      server->set_max_participants(max_participants_);
      server->set_broadcasting_all_messages(b_auto_broadcast_all_incoming_messages);
      server->set_service_name(service_name_);
      /*if (PROTOCOL == TCP) {
	if (PROTOCOL_VERSION == V4) {
	server = std::shared_ptr<nr_server>(new nr_server(io_context_, tcp_endpoint));
	}
	else {
	server = std::shared_ptr<nr_server>(new nr_server(io_context_, tcp_endpoint));
	}
	}
	else
	{
	if (PROTOCOL_VERSION == V4)
	udp_endpoint = udp::endpoint(udp::v4(), port_);
	else
	udp_endpoint = udp::endpoint(udp::v6(), port_);
	//server = std::shared_ptr<nr_server>(new nr_server(io_context_, udp_endpoint));
	}*/
      io_context_.run();
    }
    catch (std::exception &e) {
      std::cerr << "thread exception catch: " << e.what() << std::endl;
    }
    b_thread_running_ = false;
    std::cout << "----- finish thread XRNetworkLobbyService -----\n";

    //TODO: log thread stopped
  }

  boost::asio::io_context io_context_;
  std::shared_ptr<std::thread> service_thread_ptr_;
  std::atomic<bool> b_thread_running_;
  std::atomic<bool> b_socket_no_delay_;
  std::atomic<bool> b_keep_alive_;
  std::atomic<bool> b_auto_broadcast_all_incoming_messages;
  std::shared_ptr<nr_server> server;
  std::chrono::duration<double> buffer_message_timeout_; // timeout required to trigger OnNewMessage signal (if not empty)
  uint32_t message_buffer_size_; // number of messages required to trigger the OnNewMessage signal
  uint16_t max_participants_;
  std::string service_name_;
  uint64_t id_;
  uint16_t port_;
  boost::signals2::signal<void()> OnRunning;
  boost::signals2::signal<void()> OnStopping;
  uint16_t m_handshake_max_connections;
};

typedef XRNetworkLobbyService::xrnls_ptr xrnls_ptr;

#endif

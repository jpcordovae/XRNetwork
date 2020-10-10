#ifndef ERROR_MESSAGES__H_
#define ERROR_MESSAGES__H_

#include <boost/asio/error.hpp>
#include <iostream>
#include <thread>

using namespace boost;

std::string  check_system_error_code(boost::system::error_code ec)
{
  switch(ec.value()){
  case asio::error::address_family_not_supported: return "asio::error::address_family_not_supported "; break;
  case asio::error::address_in_use: return "asio::error::address_in_use "; break;
  case asio::error::already_connected: return "asio::error::already_connected "; break;
  case asio::error::already_started: return "asio::error::already_started "; break;
  case asio::error::broken_pipe: return "asio::error::broken_pipe "; break;
  case asio::error::connection_aborted: return "asio::error::connection_aborted "; break;
  case asio::error::connection_refused: return "asio::error::connection_refused "; break;
  case asio::error::connection_reset: return "asio::error::connection_reset "; break;
  case asio::error::bad_descriptor: return "asio::error::bad_descriptor "; break;
  case asio::error::fault: return "asio::error::fault "; break;
  case asio::error::host_unreachable: return "asio::error::host_unreachable "; break;
  case asio::error::in_progress: return "asio::error::in_progress "; break;
  case asio::error::interrupted: return "asio::error::interrupted "; break;
  case asio::error::invalid_argument: return "asio::error::invalid_argument "; break;
  case asio::error::message_size: return "asio::error::message_size "; break;
  case asio::error::name_too_long: return "asio::error::name_too_long "; break;
  case asio::error::network_down: return "asio::error::network_down "; break;
  case asio::error::network_reset: return "asio::error::network_reset "; break;
  case asio::error::network_unreachable: return "asio::error::network_unreachable "; break;
  case asio::error::no_descriptors: return "asio::error::no_descriptors "; break;
  case asio::error::no_buffer_space: return "asio::error::no_buffer_space "; break;
  case asio::error::no_memory: return "asio::error::no_memory "; break;
  case asio::error::no_permission: return "asio::error::no_permission "; break;
  case asio::error::no_protocol_option: return "asio::error::no_protocol_option "; break;
  case asio::error::not_connected: return "asio::error::not_connected "; break;
  case asio::error::not_socket: return "asio::error::not_socket "; break;
  case asio::error::operation_aborted: return "asio::error::operation_aborted "; break;
  case asio::error::operation_not_supported: return "asio::error::operation_not_supported "; break;
  case asio::error::shut_down: return "asio::error::shut_down "; break;
  case asio::error::timed_out: return "asio::error::timed_out "; break;
    //case asio::error::try_again: return "asio::error::try_again "; break;
  case asio::error::would_block: return "asio::error::would_block "; break;
  default: break;
  }
  return "";
}

/*
#define ASIO_ERROR_MESSAGE_CASE(MSG,EC) case  #EC: \
											std::cout << #MSG << std::endl; \
											break;

#define ASIO_ERROR_MESSAGE(EC,CASES)	switch(#EC) {	\
											#CASES \
											default:	\
											break;	\
										};



ASIO_ERROR_MESSAGE_CASE("asio::error::access_denied ",asio::error::access_denied)

std::cout << "asio::error::address_family_not_supported " << asio::error::address_family_not_supported << std::endl;
std::cout << "asio::error::address_in_use " << asio::error::address_in_use << std::endl;
std::cout << "asio::error::already_connected " << asio::error::already_connected << std::endl;
std::cout << "asio::error::already_started " << asio::error::already_started << std::endl;
std::cout << "asio::error::broken_pipe " << asio::error::broken_pipe << std::endl;
std::cout << "asio::error::connection_aborted " << asio::error::connection_aborted << std::endl;
std::cout << "asio::error::connection_refused " << asio::error::connection_refused << std::endl;
std::cout << "asio::error::connection_reset " << asio::error::connection_reset << std::endl;
std::cout << "asio::error::bad_descriptor " << asio::error::bad_descriptor << std::endl;
std::cout << "asio::error::fault " << asio::error::fault << std::endl;
std::cout << "asio::error::host_unreachable " << asio::error::host_unreachable << std::endl;
std::cout << "asio::error::in_progress " << asio::error::in_progress << std::endl;
std::cout << "asio::error::interrupted " << asio::error::interrupted << std::endl;
std::cout << "asio::error::invalid_argument " << asio::error::invalid_argument << std::endl;
std::cout << "asio::error::message_size " << asio::error::message_size << std::endl;
std::cout << "asio::error::name_too_long " << asio::error::name_too_long << std::endl;
std::cout << "asio::error::network_down " << asio::error::network_down << std::endl;
std::cout << "asio::error::network_reset " << asio::error::network_reset << std::endl;
std::cout << "asio::error::network_unreachable " << asio::error::network_unreachable << std::endl;
std::cout << "asio::error::no_descriptors " << asio::error::no_descriptors << std::endl;
std::cout << "asio::error::no_buffer_space " << asio::error::no_buffer_space << std::endl;
std::cout << "asio::error::no_memory " << asio::error::no_memory << std::endl;
std::cout << "asio::error::no_permission " << asio::error::no_permission << std::endl;
std::cout << "asio::error::no_protocol_option " << asio::error::no_protocol_option << std::endl;
std::cout << "asio::error::not_connected " << asio::error::not_connected << std::endl;
std::cout << "asio::error::not_socket " << asio::error::not_socket << std::endl;
std::cout << "asio::error::operation_aborted " << asio::error::operation_aborted << std::endl;
std::cout << "asio::error::operation_not_supported " << asio::error::operation_not_supported << std::endl;
std::cout << "asio::error::shut_down " << asio::error::shut_down << std::endl;
std::cout << "asio::error::timed_out " << asio::error::timed_out << std::endl;
std::cout << "asio::error::try_again " << asio::error::try_again << std::endl;
std::cout << "asio::error::would_block " << asio::error::would_block << std::endl;
*/

/*
class logger 
{
public:
	enum LEVEL { ERROR=0, VERBOSE, TRACE, INFO };
	
	logger() : level(LEVEL::VERBOSE)
	{}

	void set_level(LEVEL level_) 
	{
		level = level_;
	}

	void logMessage(LEVEL, char*)
	{
		mutex.lock();

		mutex.unlock();
	}

private:
	LEVEL level;
	std::mutex mutex;
	logger() {}
};

#define XR_LOG_INTERNAL(LEVEL,EXPR)			\	
	if (xrlogger::loggingEnabled(LEVEL)) {	\
	std::ostringstream os;					\
	os << EXPR;								\
	xrlogger::logMessage(LEVEL, os.str());	\
	}else(void) 0

#define XRLOG(TLEVEL,EXPR) \
	XR_LOG_INTERNAL(xrlogger::XRLOG_LEVEL_##TLEVEL,EXPR)


#define XRLOG_ERROR(EXP) XR_LOG_INTERNAL(ERROR,EXP)
#define XRLOG_VERBOSE(EXP) XR_LOG_INTERNAL(VERBOSE,EXP)
#define XRLOG_TRACE(EXP) XR_LOG_INTERNAL(TRACE,EXP)
#define XRLOG_INFO(EXP) XR_LOG_INTERNAL(INFO,EXP)
#define XRLOG_PROFILE(EXP) XR_LOG_INTERNAL(TRACE,EXP)
*/

#endif

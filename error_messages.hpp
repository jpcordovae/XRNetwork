#ifndef ERROR_MESSAGES__H_
#define ERROR_MESSAGES__H_

#include <boost/asio/error.hpp>
#include <iostream>
#include <thread>

using namespace boost;
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
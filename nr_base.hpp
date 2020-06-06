#ifndef NR_BASE_H__
#define NR_BASE_H__

#include <cstdint>
#include <iostream>
#include <random>
#include <cstdlib>
#include <cstddef>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <string>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <mutex>
#include "message.hpp"
#ifdef WINNT_
#include <boost\winapi\basic_types.hpp>
#endif
#include <boost/asio/error.hpp>
#include <boost/crc.hpp>
#include <condition_variable>
#include <chrono>

//#include <boost/range/algorithm.h>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/sources/severity_feature.hpp>
#include <boost/log/sources/severity_logger.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

//----------------------------------------------------------------------

#ifndef BOOST_LOG_STREAM_WITH_PARAMS
#define BOOST_LOG_STREAM_WITH_PARAMS
#endif

//----------------------------------------------------------------------

#define NR_FAIL -1
#define NR_OK    0

// We define our own severity levels
enum log_severity_level
{
    normal,
    notification,
    warning,
    error,
    critical
};

boost::log::sources::severity_logger<log_severity_level> slg;
//#define LOG_INFO(MSG)  BOOST_LOG_SEV(slg,normal) << (#MSG) ;
//#define LOG_ERROR(MSG) BOOST_LOG_SEV(slg,error) << (#MSG) ;
//#define LOG_WARNING(MSG) BOOST_LOG_SEV(slg,warning) << (#MSG) ;
//#define LOG_NOTIFICATION(MSG) BOOST_LOG_SEV(slg,notification) <<(#MSG) ;

void init_log()
{
    std::string logfile;
    //boost::log::add_file_log(logfile);
}

typedef void(*debug_message_callback)(char*buffer,uint32_t buffer_size);
typedef void(*new_participant_callback)(uint64_t);
typedef void(*new_message_callback)(uint64_t,char *buffer, uint32_t buffer_size);
typedef void(*participant_leave_callback)(uint64_t participant_id);
typedef void(*participant_network_event)(uint64_t participant_id, uint32_t event);

uint64_t get_timestamp_now()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

/*
boost::asio::ip::address_v6 v6_from_string(std::string s)
{
  using boost::asio::ip::address_v6;
  auto i = boost::find(s, '%');
  auto addr = address_v6::from_string(std::string(s.begin(), i));
  if (i != s.end()){
    const std::string if_(std::next(i), s.end());
    addr.scope_id(detail::scope_id_from_if(if_));
  }
  return addr;
}
*/

#endif // NR_BASE_H__

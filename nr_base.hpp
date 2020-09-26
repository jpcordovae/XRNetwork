#ifndef NR_BASE_H__
#define NR_BASE_H__

#include <cstdint>
#include <ostream>
#include <random>
#include <cstdlib>
#include <cstddef>
#include <deque>
#include <iostream>
#include <iomanip>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <string>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <mutex>
#include "message.hpp"
//#ifdef WINNT_
//#include <boost\winapi\basic_types.hpp>
//#endif
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
#include <endian.h>
#include <functional>

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

//----------------------------------------------------------------------

#ifndef BOOST_LOG_STREAM_WITH_PARAMS
#define BOOST_LOG_STREAM_WITH_PARAMS
#endif

//----------------------------------------------------------------------

#if __has_include(<bit>)
#include <bit>
#ifdef __cpp_lib_endian
#define HAS_ENDIAN 1
#endif
#endif

//#define IS_BIG_ENDIAN (*(uint16_t*)"\0\xFF" < 0x0100)

//#define IS_LITTLE_ENDIAN (((union { uint16_t u16; uint32_t u32; }) { 0xFFFF0000 }).u16 ) // need optimization to run this on compiler time, seems like -o1 is enough

/*#ifdef HAS_ENDIAN
constexpr bool is_little_endian = std::endian::native == std::endian::little;
#else
constexpr bool is_little_endian = 
#endif
*/
//----------------------------------------------------------------------

#define NR_FAIL -1
#define NR_OK    0

std::ostream &operator<<(std::ostream &os, const std::byte b)
{
  os << std::hex << std::setw(2) << (0xFF & (unsigned int)b);
  return os;
}

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

bool is_system_little_endian()
{
  const int value { 0x01 };
  const void * address = static_cast<const void*>(&value);
  const unsigned char * least_significant_address = static_cast<const unsigned char*>(address);
  return (*least_significant_address == 0x01);
}

template<class T> T swap_endian(const T &val)
{
  union U {
    T val;
    std::array<std::byte, sizeof(T)> raw;
  } src, dst;

  src.val = val;
  std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
  //val = dst.val;
  return dst.val;
}

std::vector<std::byte> swap_byte_vector(const std::vector<std::byte> origin)
{
  std::vector<std::byte> dst(origin.size(),(std::byte)0x00); // create and  clear the vector
  std::reverse_copy(origin.begin(),origin.end(),dst.begin());
  return dst;
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

std::ostream &operator<<(std::ostream &os, const std::vector<std::byte> buffer)
{
  std::for_each(buffer.begin(),buffer.end(),[&](const std::byte &b){
                                              os << std::setfill('0') << std::setw(2) <<  std::hex << (unsigned char)b;
                                            });
  return os;
}

void print_buffer(std::byte *buffer, size_t buffersize)
  {
    for(size_t i=0;i<buffersize;i++){
      printf("%02x ",(unsigned int)*(buffer+i));
    }
  }



#endif // NR_BASE_H__

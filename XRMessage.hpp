/*! \file XRMessage.hpp
  \brief this file contain the class used by XRNetwork to share messages between the clients and the server.
  This class is a especialization of a vector class std::vector<std::byte>., where the endianess and buffer control
  is encapsulated and mannaged by the  classs. Data buffer should be get from the functions exposed by  XRMessage
  and NOT using the std::vector functions.
*/

#ifndef XRMESSAGE_HPP__
#define XRMESSAGE_HPP__

#include "nr_base.hpp"
#include <cstdint>
#include <vector>
#include <cstring>
#include <bit>

struct xr_message_header{
  uint16_t head;
  uint32_t buffersize;
  uint16_t payload_is_big_endian; // options are 0xFFFF for big endian, and 0x0000 for little endian
  xr_message_header() : head(0),buffersize(0)
  {
    if(!is_system_little_endian()) payload_is_big_endian = 0xFFFF;
    else
      payload_is_big_endian = 0x0000;
  }
  xr_message_header(uint16_t _head, uint32_t _bsize) : head(_head), buffersize(_bsize)
  {
    if(!is_system_little_endian()) payload_is_big_endian = 0xFFFF;
    else
      payload_is_big_endian = 0x0000;
  }
};

xr_message_header swap_xr_message_header(const xr_message_header *msg)
{
  xr_message_header xrm = *msg;
  xrm.head = swap_endian<uint64_t>(msg->head);
  xrm.buffersize = swap_endian<uint64_t>(msg->buffersize);
  //xrm.payload_is_big_endian; // no need to swap this one
  return xrm;
}

std::ostream &operator<<(std::ostream &os, const xr_message_header header)
{
  os << "head                  : " << std::hex << std::setw(2) << header.head       << std::endl
     << "buffersize            : " << std::hex << std::setw(4) << header.buffersize << std::endl
     << "payload_is_big_endian : " << std::hex << std::setw(2) << header.payload_is_big_endian;
  return os;
}

std::ostream &operator<<(std::ostream &os, const xr_message_header *header_ptr)
{
  std::cout <<  "head | buffersieze | is_big_endian \n" << std::hex << header_ptr->head << " | " << std::hex << header_ptr->buffersize << " | " << std::hex << header_ptr->payload_is_big_endian;
  return os;
}

class XRMessage : public std::vector<std::byte>
{
public:

  typedef std::shared_ptr<XRMessage> XRMessage_ptr;

  XRMessage(const std::byte*buffer, const uint32_t &_buffersize, bool check_endianess)
  {
    this->resize(sizeof(xr_message_header) + _buffersize);
    memset(data(),0x00,size());
    std::copy(buffer,buffer+_buffersize,this->begin());
    /*if(check_endianess && is_system_little_endian()){
      this->set_header(swap_xr_message_header(this->get_header()));
      }*/
  }

  XRMessage(const uint16_t &_head, const std::byte *buffer, const uint32_t &_buffersize)
  {
    size_t tsize = sizeof(xr_message_header) + _buffersize;

    if(m_b_delimiter)
      tsize += 1;

    this->resize(tsize);

    //if(is_system_little_endian()){ // network is big endian
      // swap all buffers
      //uint16_t _head2 = swap_endian<uint16_t>(_head);
      //uint32_t _buffersize2 = swap_endian<uint32_t>(_buffersize);
      //xr_message_header header(_head2,_buffersize2);
      //memcpy(this->data(),&header,sizeof(xr_message_header));
    //}else{
      xr_message_header header(_head, _buffersize);
      memcpy(this->data(),&header,sizeof(xr_message_header));
      // }

    /*if(m_base64){
      // TODO: make codes for base64
    }else{
      memcpy(this->data()+sizeof(xr_message_header),&_buffersize,sizeof(uint32_t));
      }*/

    /*if(is_system_little_endian){
      std::reverse_copy(buffer,buffer+_buffersize,this->data()+sizeof(xr_message_header));
    }else{
      memcpy(this->data()+sizeof(xr_message_header),buffer,_buffersize);
      }*/
    memcpy(this->data()+sizeof(xr_message_header),buffer,_buffersize);

    if(m_b_delimiter)
      push_back((std::byte)m_delimiter);
  }

  uint16_t head()
  {
    return this->size() >= sizeof(xr_message_header) ? *((uint16_t*) data()) : (uint16_t)0x00;
  }

  uint32_t payload_size()
  {
    return this->size() >= sizeof(xr_message_header) ? get_header()->buffersize : (uint32_t) 0x00;
  }

  std::byte* payload()
  {
    return this->size() >= sizeof(xr_message_header) ? data()+sizeof(xr_message_header) : nullptr;
  }
  
  xr_message_header *get_header()
  {
    return this->size() >= sizeof(xr_message_header) ? (xr_message_header*)data() : nullptr;
  }

  void set_header(const xr_message_header *_header)
  {
    memcpy((void*)this->data(),(void*)_header,sizeof(xr_message_header));
  }

  void set_header(const xr_message_header &_header)
  {
    this->set_header(&_header);
  }

  void base64(const bool &b64)
  {
    m_base64 = b64;
  }

  bool use_delimiter()
  {
    return m_b_delimiter;
  }

  void use_delimiter(const bool del)
  {
    m_b_delimiter = del;
    if(size()>sizeof(xr_message_header)) this->at(size()-1) = m_delimiter;
  }

  void set_delimiter(const std::byte _delimiter)
  {
    m_delimiter = _delimiter;
  }

  std::byte get_delimiter()
  {
    return m_delimiter;
  }

private:
  bool m_b_delimiter; // enable or disable dlimiter use
  bool m_base64; // base64 buffer transform at in and out 
  std::byte m_delimiter;
  XRMessage(){};
};

typedef XRMessage::XRMessage_ptr XRMessage_ptr;

#endif

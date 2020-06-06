#ifndef XRMESSAGE_H__
#define XRMESSAGE_H__

#include "nr_base.hpp"
#include <cstdint>
#include <vector>
#include <cstring>

struct xr_message_header{
  uint16_t head;
  uint32_t buffersize;
  xr_message_header() : head(0),buffersize(0)
  {}
};

class XRMessage : public std::vector<std::byte>
{
public:

  typedef std::shared_ptr<XRMessage> XRMessage_ptr;

  XRMessage(const uint16_t &_head, const std::byte *buffer, const uint32_t &_buffersize)
  {
    if(m_b_set_delimeter)
      this->resize(sizeof(xr_message_header) + _buffersize + sizeof(m_delimeter));
    else
      this->resize(sizeof(xr_message_header) + _buffersize);

    memcpy(this->data(),&_head,sizeof(uint16_t));

    if(m_base64){
      // TODO: make codes for base64
    }else{
      memcpy(this->data()+sizeof(uint16_t),&_buffersize,sizeof(uint32_t));
    }

    memcpy(this->data()+sizeof(xr_message_header),buffer,_buffersize);

    if(m_b_set_delimeter)
      push_back((std::byte)m_delimeter);
  }

  uint16_t head()
  {
    return this->size() >= sizeof(xr_message_header) ? *((uint16_t*) data()) : (uint16_t)0x00;
  }

  uint32_t buffersize()
  {
    return this->size() >= sizeof(xr_message_header) ? *((uint32_t*)( data()+sizeof(uint16_t))) : (uint32_t) 0x00;
  }

  xr_message_header *header()
  {
    return this->size() >= sizeof(xr_message_header) ? (xr_message_header*)data() : nullptr;;
  }

  void header(const xr_message_header *_header)
  {
    memcpy((void*)this->data(),(void*)_header,sizeof(xr_message_header));
  }

  void header(const xr_message_header &_header)
  {
    this->header(&_header);
    //memcpy(std::vector<std::byte>::data(),&_header,sizeof(xr_message_header));
  }

  void base64(const bool &b64)
  {
    m_base64 = b64;
  }
  
private:
  bool m_b_set_delimeter;
  bool m_base64; // base64 buffer transform at in and out 
  std::byte m_delimeter;
  XRMessage(){};
};

typedef XRMessage::XRMessage_ptr XRMessage_ptr;

#endif

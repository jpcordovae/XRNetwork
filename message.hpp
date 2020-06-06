//

// message.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Juan Pablo Cordova E. (jpcordovae at gmail dot com)
//
// Distributed under MIT license software
//
#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <deque>

#define NR_MESSAGE_VERSION_MAYOR 0
#define NR_MESSAGE_VERSION_MINOR 1

class nr_message
{
public:
  enum { header_length = sizeof(uint16_t) }; //64bit header
  enum { max_body_length = 64*1024 - sizeof(uint16_t)};

  nr_message() : body_length_(64*1024)
  {
    memset(data_, 0x00, header_length + max_body_length);
  }

  const char* data() const
  {
    return data_;
  }

  char* data()
  {
    return data_;
  }

  std::size_t length() const
  {
    return header_length + body_length_;
  }

  const char* body() const
  {
    return data_ + header_length;
  }

  char* body()
  {
    return data_ + header_length;
  }

  std::size_t body_length() const
  {
    return body_length_;
  }

  void body_length(std::size_t new_length)
  {
    body_length_ = new_length;
    if (body_length_ > max_body_length)
      body_length_ = max_body_length;
  }

  bool decode_header()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    body_length_ = std::atoi(header);
    if (body_length_ > max_body_length)
      {
        body_length_ = 0;
        return false;
      }
    return true;
  }

  void encode_header()
  {
    char header[header_length + 1] = "";
    std::sprintf(header, "%4d", static_cast<int>(body_length_));
    std::memcpy(data_, header, header_length);
  }

  void set_participnant(uint64_t participant)
  {
    participant_id_ = participant;
  }

  uint64_t get_participant()
  {
    return participant_id_;
  }

private:
  char data_[header_length + max_body_length];
  std::size_t body_length_;
  uint64_t participant_id_; // owner of the message
};

typedef std::deque<nr_message> nr_message_queue;

#endif // CHAT_MESSAGE_HPP

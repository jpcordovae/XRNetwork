#ifndef PROTOCOL_H__
#define PROTOCOL_H__

#include "nr_base.hpp"

struct ST_MESSAGE {
  uint16_t m_head;
  uint32_t m_buffersize;
  std::byte *m_buffer;
};

enum class EN_RAW_MESSAGE_HEAD : uint16_t { NONE,
                                            HANDSHAKE_HELLO,
                                            HANDSHAKE_HELLO_ACK,
                                            HANDSHAKE_CREDENTIALS,
                                            HANDSHAKE_CREDENTIALS_ACK,
                                            HANDSHAKE_STATISTICS_REQUEST,
                                            HANDSHAKE_STATISTICS_REQUEST_ACK,
                                            PARTICIPANT_INFO_REQUEST,
                                            PARTICIPANT_INFO_REQUEST_ACK,
                                            NEW_PARTICIPANT_INFO,
                                            NEW_PARTICIPANT_INFO_ACK,
                                            PARTICIPANT_UPDATE,
                                            PARTICIPANT_UPDATE_ACK,
                                            EVENT,
                                            EVENT_ACK,
                                            AVAR_MESSAGE,
                                            CHAT_MESSAGE,
                                            CONTROL };

struct ST_RAW_MESSAGE
{
  EN_RAW_MESSAGE_HEAD head;
  uint32_t buffersize;
  std::byte buffer[1024*64-sizeof(EN_RAW_MESSAGE_HEAD)-sizeof(uint32_t)];
};

typedef std::shared_ptr<ST_RAW_MESSAGE> raw_network_message_ptr;

raw_network_message_ptr build_message(uint16_t header, std::byte *buffer, uint32_t buffersize)
{
  raw_network_message_ptr rmp(new ST_RAW_MESSAGE);
  rmp->head = (EN_RAW_MESSAGE_HEAD) header;
  rmp->buffersize = buffersize;
  memcpy(rmp->buffer,buffer,buffersize);
  return rmp;
}

struct ST_NEW_PARTICIPANT_INFO {
  uint64_t participant_id;
  uint32_t buffersize;
  std::byte buffer[1024*2];
};

struct ST_NEW_PARTICIPANT_INFO_ACK {
  uint8_t bool_response;
};

struct ST_PARTICIPANT_UPDATE {
  uint64_t participant_id;
  uint64_t timestamp;
  double position[3];
  double rotation[4]; // quaternion w,i,j,k
  double position_lhand[3];
  double rotation_lhand[4];
  double position_rhand[3];
  double rotation_rhand[4];
};

struct ST_PARTICIPANT_UPDATE_ACK {
  uint64_t timestamp;
};

struct ST_HANDSHAKE_HELLO {
  uint64_t service_id; // id of the server
  uint64_t participant_id; // id asigned by the server
  uint64_t server_timestamp; // timestamp of the server in UTC0
  uint64_t servername_buffersize;
  std::byte servername_buffer[1024]; // 1kb for the name, check this in code
};

struct ST_HANDSHAKE_HELLO_ACK
{
  uint64_t participant_id;
  uint64_t client_timestamp;
  uint32_t participant_name_buffersize;
  std::byte participant_name_buffer[1024];
  ST_HANDSHAKE_HELLO_ACK(): participant_id(0),
                            client_timestamp(0),
                            participant_name_buffersize(0)
  {
    memset(participant_name_buffer,0x00,1024);
  }
};

enum class EN_SECURITY : int16_t { LOGIN_PASSWORD,
                                   SSL,
                                   TLS };

struct ST_HANDSHAKE_CREDENTIALS
{
  uint32_t server_certificate_buffersize;
  std::byte server_certificate_buffer[20*1024]; // 20kb reserved for certificates
  ST_HANDSHAKE_CREDENTIALS() : server_certificate_buffersize(0)
  {
    memset(server_certificate_buffer,0x00,20*1024); // clean the buffer
  }
};

struct ST_HANDSHAKE_CREDENTIALS_ACK
{
  uint32_t login_buffersize;
  std::byte login_buffer[1024];
  uint32_t password_buffersize;
  std::byte password_buffer[1014];
  uint32_t server_certificate_buffersize;
  std::byte server_certificate_buffer[20*1024]; // 20kb reserved for certificates

  ST_HANDSHAKE_CREDENTIALS_ACK() : login_buffersize(0),
                                   password_buffersize(0),
                                   server_certificate_buffersize(0)
  {
    memset(login_buffer,0x00,1024);
    memset(password_buffer,0x00,1024);
    memset(server_certificate_buffer,0x00,20*1024); // clean the buffer
  }
};

#endif

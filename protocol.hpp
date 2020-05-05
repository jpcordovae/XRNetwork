#ifndef PROTOCOL_H__
#define PROTOCOL_H__

#include "nr_base.hpp"

enum class EN_RAW_MESSAGE_HEAD : uint16_t { HANDSHAKE,
					    RUNTIME,
					    PARTICIPANT_INFO,
					    AVAR_MESSAGE,
					    CHAT_MESSAGE,
					    EVENT,
					    CONTROL };

struct ST_RAW_MESSAGE
{
  EN_RAW_MESSAGE_HEAD head;
  uint32_t buffersize;
  std::byte buffer[1024*64-sizeof(EN_RAW_MESSAGE_HEAD)-sizeof(uint32_t)];
};

typedef std::shared_ptr<ST_RAW_MESSAGE> raw_network_message_ptr;

raw_network_message_ptr build_raw_network_message(uint16_t header, std::byte *buffer, uint32_t buffersize)
{
  raw_network_message_ptr rmp(new ST_RAW_MESSAGE);
  rmp->head = EN_RAW_MESSAGE_HEAD(header);
  rmp->buffersize = buffersize;
  memcpy(rmp->buffer,buffer,buffersize);
  return rmp;
}

enum class EN_HANDSHAKE : int16_t { HANDSHAKE_SERVER_HELLO,
				    HANDSHAKE_SERVER_HELLO_ACK,
				    HANDSHAKE_CREDENTIALS_REQUEST,
				    HANDSHAKE_CREDENTIALS_REQUEST_ACK,
				    HANDSHAKE_STATISTICS_REQUEST,
				    HANDSHAKE_STATISTICS_REQUEST_ACK };

struct ST_HANDSHAKE_HELLO
{
  EN_HANDSHAKE head;
  uint64_t service_id; // id of the server
  uint64_t participant_id; // id asigned by the server
  uint64_t server_timestamp; // timestamp of the server in UTC0
  uint64_t servername_buffersize;
  std::byte servername_buffer[1024]; // 1kb for the name, check this in code
};

struct ST_HANDSHAKE_HELLO_ACK
{
  EN_HANDSHAKE head;
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

struct ST_CREDENTIALS_REQUEST
{
  EN_SECURITY en_security;
  uint32_t server_certificate_buffersize;
  std::byte server_certificate_buffer[20*1024]; // 20kb reserved for certificates
  ST_CREDENTIALS_REQUEST() : en_security(EN_SECURITY::LOGIN_PASSWORD),
			     server_certificate_buffersize(0)
  {
    memset(server_certificate_buffer,0x00,20*1024); // clean the buffer
  }
};

struct ST_CREDENTIALS_REQUEST_ACK
{
  uint32_t login_buffersize;
  std::byte login_buffer[1024];
  uint32_t password_buffersize;
  std::byte password_buffer[1014];
  uint32_t server_certificate_buffersize;
  std::byte server_certificate_buffer[20*1024]; // 20kb reserved for certificates

  ST_CREDENTIALS_REQUEST_ACK() : en_security(EN_SECURITY::LOGIN_PASSWORD),
				 login_buffersize(0),
				 password_buffersize(0),
				 server_certificate_buffersize(0)
  {
    memset(login_buffer,0x00,1024);
    memset(password_buffer,0x00,1024);
    memset(server_certificate_buffer,0x00,20*1024); // clean the buffer
  }
};

#endif

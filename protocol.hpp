#ifndef PROTOCOL_H__
#define PROTOCOL_H__

#include "nr_base.hpp"

struct ST_MESSAGE {
  uint16_t m_head;
  uint32_t m_buffersize;
  std::byte *m_buffer;
}__attribute__((__packed__));

enum class EN_RAW_MESSAGE_HEAD : uint16_t { NONE,
  HANDSHAKE_HELLO,
  HANDSHAKE_HELLO_ACK,
  HANDSHAKE_CREDENTIALS,
  HANDSHAKE_CREDENTIALS_ACK,
  HANDSHAKE_STATISTICS_REQUEST,
  HANDSHAKE_STATISTICS_REQUEST_ACK,
  HANDSHAKE_PARTICIPANT_UPDATE,
  HANDSHAKE_PARTICIPANT_UPDATE_ACK,
  PARTICIPANT_JOIN, // inform to participant about join or to the service room
  PARTICIPANT_JOIN_ACK, //
  MESSAGE,
  MESSAGE_ACK,
  PARTICIPANT_NEW,
  PARTICIPANT_NEW_ACK,
  PARTICIPANT_LEAVE,
  PARTICIPANT_LEAVE_ACK,
  PARTICIPANT_INFO_REQUEST,
  PARTICIPANT_INFO_REQUEST_ACK,
  PARTICIPANT_UPDATE,
  PARTICIPANT_UPDATE_ACK,
  PARTICIPANT_EVENT,
  PARTICIPANT_EVENT_ACK,
  AVAR_MESSAGE,
  CHAT_MESSAGE,
  CONTROL,
  OBJECT,
  OBJECT_ACK };

enum class EN_OBJECT_MESSAGE_HEAD : uint16_t {
  NONE,
  OBJECT_NEW,
  OBJECT_DELETE,
  OBJECT_UPDATE
};

// ST_OBJECT_ACK

constexpr size_t ST_OBJECT_ACK_PAYLOAD_SIZE = 20*1024;

struct ST_OBJECT_ACK
{
  uint16_t object_head;
  uint64_t participant_id; // owner of the object, to be emplaced as parent transform
  uint64_t object_id;
  uint32_t json_buffersize;
  std::byte json_buffer[ST_OBJECT_ACK_PAYLOAD_SIZE];
  ST_OBJECT_ACK() : object_head((uint16_t)EN_OBJECT_MESSAGE_HEAD::NONE),
		    json_buffersize(ST_OBJECT_ACK_PAYLOAD_SIZE)
  {
    memset(json_buffer,0x00,ST_OBJECT_ACK_PAYLOAD_SIZE);
  }
}__attribute__((__packed__));

// ST_OBJECT

constexpr size_t ST_OBJECT_PAYLOAD_SIZE = 20*1024;

struct ST_OBJECT
{
  uint16_t object_head;
  uint64_t participant_id; // owner of the object
  uint64_t object_id;
  uint32_t json_buffersize;
  std::byte json_buffer[ST_OBJECT_PAYLOAD_SIZE];
  ST_OBJECT() : json_buffersize(ST_OBJECT_PAYLOAD_SIZE)
  {
    memset(json_buffer,0x00,ST_OBJECT_PAYLOAD_SIZE);
  }
}__attribute__((__packed__));

// ST_PARTICIPANT_DELETE
struct ST_PARTICIPANT_LEAVE
{
  uint64_t participant_id;
}__attribute__((__packed__));

std::ostream &operator<<(std::ostream& os, const ST_PARTICIPANT_LEAVE del)
{
  os << "participant_id: " << std::hex << del.participant_id;
  return os;
}

// ST_PARTCIPANT_LEAVE_ACK

struct ST_PARTICIPANT_LEAVE_ACK
{
  uint64_t participant_id;
}__attribute__((__packed__));

std::ostream &operator<<(std::ostream& os, const ST_PARTICIPANT_LEAVE_ACK del)
{
  os << "participant_id: " << del.participant_id;
  return os;
}

// ST_PARTICIPANT_NEW

struct ST_PARTICIPANT_NEW
{
  uint64_t participant_id;
  uint32_t descriptor_buffersize;
  std::byte descriptor_buffer[1024*20];
  ST_PARTICIPANT_NEW() : descriptor_buffersize(0)
  {
    memset(descriptor_buffer,0x00,1024);
  }
}__attribute__((__packed__));

std::ostream &operator<<(std::ostream &os, const ST_PARTICIPANT_NEW part_new)
{
  os << "participant_id: "  << std::hex << part_new.participant_id
     << "descriptor_buffersize: " << std::hex << part_new.descriptor_buffersize
     << "descriptor_buffer: "     << (char*) part_new.descriptor_buffer;
  return os;
}

// ST_PARTICIPANT_NEW_ACK
struct ST_PARTICIPANT_NEW_ACK
{
  uint64_t participant_id;
}__attribute__((__packed__));

std::ostream &operator<<(std::ostream &os, const ST_PARTICIPANT_NEW_ACK part_new_ack)
{
  os << "new_participant_id: " << std::hex << part_new_ack.participant_id;
  return os;
}

// ST_PARTICIPANT_JOIN
struct ST_PARTICIPANT_JOIN
{
  uint64_t participant_id;
  uint16_t max_data_rate;
  uint16_t allow_asynchronous_messages; // this determines if the server wors by polling or asynchronic messages
  uint32_t message_buffersize;
  std::byte message_buffer[1024*10];
  ST_PARTICIPANT_JOIN():participant_id(0),
                        max_data_rate(10),
                        allow_asynchronous_messages(0xFFFF),
                        message_buffersize(0)
  {
    memset(message_buffer,0x00,1024*10);
  }
}__attribute__((__packed__));

std::ostream &operator<<(std::ostream &os, const ST_PARTICIPANT_JOIN join)
{
  os << "participant_id: " << std::hex << join.participant_id << std::endl
     << "max_data_rate: " << join.max_data_rate << "[messages/s]\n"
     << "allow_asynchronous_messages: " << std::hex << join.allow_asynchronous_messages << std::endl
     << "message_buffersize: " << join.message_buffersize << " bytes" << std::endl
     << "message_buffer: " << (char*)join.message_buffer;
  return os;
}

// ST_PARTICIPANT_JOIN_ACK
struct ST_PARTICIPANT_JOIN_ACK
{
  uint64_t participant_id;
  uint32_t json_buffersize;
  std::byte json_buffer[JSON_BUFFER_SIZE];
  ST_PARTICIPANT_JOIN_ACK() : json_buffersize(0)
  {
    memset(json_buffer,0x00,JSON_BUFFER_SIZE);
  }
}__attribute__((__packed__));


std::ostream &operator<<(std::ostream &os, ST_PARTICIPANT_JOIN_ACK msg)
{
  os << "participant_id: " << std::hex << msg.participant_id << "\n"
     << "JSON buffersize: " << msg.json_buffersize << "\n"
     << "JSON: " << (char*)msg.json_buffer << std::endl;
  return os;
}

// PARTICIPANT_INFO_REQUEST
struct ST_PARTICIPANT_INFO_REQUEST {
  uint64_t participant_id; // just to add a dummy load
}__attribute__((__packed__));

ST_PARTICIPANT_INFO_REQUEST build_participant_info_request(uint64_t participant_id)
{
  ST_PARTICIPANT_INFO_REQUEST pir;
  pir.participant_id = participant_id;
  return pir;
}

// PARTICIPANT_INFO_EQUEST_ACK
struct ST_PARTICIPANT_INFO_REQUEST_ACK {
  uint64_t participant_id;
  uint32_t namebuffer_size;
  std::byte namebuffer[1024];
  ST_PARTICIPANT_INFO_REQUEST_ACK(): participant_id(0),
                                     namebuffer_size(0)
  {
    memset(namebuffer, 0x00, 1024);
  }
}__attribute__((__packed__));

// ST_PARTICIPANT_UPDATE
struct ST_PARTICIPANT_UPDATE {
  uint64_t origin_timestamp;
  uint64_t reception_timestamp;
  uint64_t deliver_timestamp;
  uint32_t buffersize;
  std::byte buffer[1024*20]; // 20kb to send something user to user
};

std::ostream &operator<<(std::ostream &os, const ST_PARTICIPANT_UPDATE update)
{
  os << "origin_timestamp: "      << std::hex << update.origin_timestamp
     << "\nreception_timestamp: " << std::hex << update.reception_timestamp
     << "\ndeliver_timestamp: "   << std::hex << update.deliver_timestamp
     << "\nbuffersize: "          << std::hex << update.buffersize
     << "\nbuffer: "              << (char*)update.buffer;
  return os;
}

// ST_PARTICIPANT_UPDATE_ACK
struct ST_PARTICIPANT_UPDATE_ACK {
  uint64_t participant_id;
  uint64_t origin_timestamp;
  uint32_t buffersize;
  std::byte buffer[1024*20];
  ST_PARTICIPANT_UPDATE_ACK() : origin_timestamp(0),
                                buffersize(0)
  {
    memset(buffer,0x00,1024*20);
  }
}__attribute__((__packed__));

std::ostream &operator<<(std::ostream &os, const ST_PARTICIPANT_UPDATE_ACK msg)
{
  os << "participant_id: " << std::hex << msg.participant_id
     << "\norigin_timestamp: " << std::hex << msg.origin_timestamp
     << "\nbuffersize: 0x" <<  std::hex << msg.buffersize << "(" << std::dec << msg.buffersize << ") bytes"
     << "\nbuffer: " << (char*)msg.buffer;
  return os;
}

// ST_HANDSHAKE_HELLO
struct ST_HANDSHAKE_HELLO {
  //uint8_t  endianess;              // 1: little endian, 0: big endian
  uint64_t service_id;               // id of the server
  uint64_t participant_id;           // id asigned by the server
  uint64_t server_timestamp;         // timestamp of the server in UTC0
  uint32_t servername_buffersize;    // 
  std::byte servername_buffer[1024]; // 1kb for the name, check this in code
}__attribute__((__packed__));

ST_HANDSHAKE_HELLO build_handshake_hello(uint64_t service_id, uint64_t participant_id,
                                         uint64_t server_timestamp, std::string servername)
{
  ST_HANDSHAKE_HELLO hello;
  hello.service_id = service_id;
  hello.participant_id = participant_id;
  hello.server_timestamp = server_timestamp;
  hello.servername_buffersize = (uint32_t)servername.size();
  memset(hello.servername_buffer,0x00,1024);
  memcpy(hello.servername_buffer,servername.c_str(),servername.size());
  return hello;
}

std::ostream &operator<<(std::ostream &os, const ST_HANDSHAKE_HELLO &st_hello)
{
  os << "service_id            : " << std::hex << st_hello.service_id             << std::endl
     << "participant_id        : " << std::hex << st_hello.participant_id         << std::endl
     << "server_timestamp      : " << std::hex << st_hello.server_timestamp       << std::endl
     << "servername_buffersize : " << std::dec << st_hello.servername_buffersize << std::endl
     << "servername_buffer     : " << std::hex << std::string((char*)st_hello.servername_buffer);
  return os;
}

// ST_HANDSHAKE_HELLO_ACK
struct ST_HANDSHAKE_HELLO_ACK
{
  uint64_t participant_id;
  uint64_t client_timestamp;
  uint32_t configuration_buffersize;
  std::byte configuration_buffer[1024*20];

  ST_HANDSHAKE_HELLO_ACK(): participant_id(0),
                            client_timestamp(0),
                            configuration_buffersize(0)
  {
    memset(configuration_buffer,0x00,1024*20);
  }

}__attribute__((__packed__));

std::ostream &operator<<(std::ostream &os, const ST_HANDSHAKE_HELLO_ACK &st_hello_ack)
{
  os << "participant_id           : " << std::hex << st_hello_ack.participant_id              << std::endl
     << "client_timestamp         : " << std::hex << st_hello_ack.client_timestamp            << std::endl
     << "configuration_buffersize : " << std::dec << st_hello_ack.configuration_buffersize    << std::endl
     << "configuration_buffer     : " << std::string((char*)st_hello_ack.configuration_buffer);
  return os;
}

enum class EN_SECURITY : int16_t { LOGIN_PASSWORD,
                                   SSL,
                                   TLS };

// ST_HANDSHAKE_CREDENTIALS
struct ST_HANDSHAKE_CREDENTIALS
{
  uint32_t server_certificate_buffersize;
  std::byte server_certificate_buffer[20*1024]; // 20kb reserved for certificates
  ST_HANDSHAKE_CREDENTIALS() : server_certificate_buffersize(0)
  {
    memset(server_certificate_buffer,0x00,20*1024); // clean the buffer
  }
}__attribute__((__packed__));

std::ostream &operator<<(std::ostream &os, const ST_HANDSHAKE_CREDENTIALS &st_cred)
{
  os << "certificate buffersize: " << std::dec << st_cred.server_certificate_buffersize << std::endl
     << "certificate:            " << (char*)st_cred.server_certificate_buffer << std::endl; 
  return os;
}

typedef std::shared_ptr<ST_HANDSHAKE_CREDENTIALS> ST_HANDSHAKE_CREDENTIALS_PTR;

ST_HANDSHAKE_CREDENTIALS build_handshake_credentials()
{
  ST_HANDSHAKE_CREDENTIALS cred;
  return cred;
}

// ST_HANDSHAKE_CREDENTIALS_ACK
struct ST_HANDSHAKE_CREDENTIALS_ACK
{
  uint64_t participant_id;
  uint16_t login_buffersize;
  std::byte login_buffer[1024];
  uint16_t password_buffersize;
  std::byte password_buffer[1024];
  uint16_t participant_certificate_buffersize;
  std::byte participant_certificate_buffer[20*1024]; // 20kb reserved for certificates

  ST_HANDSHAKE_CREDENTIALS_ACK() : login_buffersize(0),
                                   password_buffersize(0),
                                   participant_certificate_buffersize(0)
  {
    memset(login_buffer,0x00,1024);
    memset(password_buffer,0x00,1024);
    memset(participant_certificate_buffer,0x00,20*1024); // clean the buffer
  }
}__attribute__((__packed__));

std::ostream &operator<<(std::ostream &os, const ST_HANDSHAKE_CREDENTIALS_ACK &ack)
{
  os << "participant_id: "                     << std::hex << ack.participant_id << std::endl
     << "login_buffersize: "                   << std::dec << ack.login_buffersize << std::endl
     << "login_buffer: "                       << (char*)ack.login_buffer << std::endl
     << "password_buffersize: "                << std::dec << ack.password_buffersize << std::endl
     << "password_buffer: "                    << (char*)ack.password_buffer << std::endl
     << "participant_certificate_buffersize: " << std::dec << ack.participant_certificate_buffersize << std::endl
     << "participant_certificate_buffer: "     << (char*)ack.participant_certificate_buffer;
  return os;
}

#endif

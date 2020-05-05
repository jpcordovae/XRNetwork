#ifndef PARTICIPANT_H__
#define PARTICIPANT_H__

#include "nr_base.hpp"

class nr_participant
{
public:
    
  typedef std::shared_ptr<nr_participant> nr_participant_ptr;

  nr_participant() : ID_(0L), deaf_(false)
  {}

  // for performance just copy the whole structure to some managed memory space
#pragma pack(1)
  struct nr_participant_info {
    char name[1024];
    char ipv4_ip[512];
    uint64_t id_;
    uint64_t server_id_;

    nr_participant_info() : id_(0)
    {
      memset(name, 0x00, 1024);
      memset(ipv4_ip, 0x00, 512);
    }

    nr_participant_info(uint64_t server_id, uint64_t id) : id_(id), server_id_(server_id)
    { 
      nr_participant_info();
    }
  };

  typedef std::shared_ptr<nr_participant_info> nr_participant_info_ptr;

  using is_transparent = void;
  friend bool operator<(nr_participant const &part, uint64_t id) 
  {
    return part.ID_ < id;
  }

  friend bool operator<(uint64_t id, nr_participant const &part) 
  {
    return id < part.ID_;
  }

  friend bool operator<(nr_participant const& part1, nr_participant const& part2) 
  {
    return part1.ID_ < part2.ID_;
  }

  friend bool operator==(nr_participant const& part1, nr_participant const& part2) 
  {
    return part1.ID_ == part2.ID_;
  }
    
  friend bool operator==(nr_participant const& part, uint64_t id) 
  {
    return part.ID_ == id;
  }

  friend bool operator==(uint64_t id, nr_participant const& part) 
  {
    return id == part.ID_;
  }

  friend bool operator<(const nr_participant_ptr &p1, const nr_participant_ptr &p2)
  {
    return p1->ID_ < p2->ID_;
  }

  friend bool operator<(const nr_participant_ptr &p1, uint64_t id)
  {
    return p1->ID_ < id;
  }

  friend bool operator<(uint64_t id, const nr_participant_ptr& p1)
  {
    return id < p1->ID_;
  }

  virtual ~nr_participant() {};
  virtual void deliver(const nr_message& msg) = 0;
  virtual void deliver_byte(std::byte *buffer, size_t buffer_size) = 0;
  virtual void disconnect()=0; // clean drop
  virtual int set_keep_alive(bool) {
    return false;
  };
  virtual bool get_keep_alive()=0;
  bool is_deaf() { return deaf_; }
  void set_deaf(bool deaf) { deaf_ = deaf; }
  nr_participant_info_ptr get_participant_info() { return info_ptr_; }
  std::string name;
  std::string IP;
  //std::deque<nr_message> messages_deque; // in case you want to inspect the messages
  uint64_t ID_; // random ID for the participant
  nr_participant_info_ptr info_ptr_;
  bool deaf_;
};

typedef nr_participant::nr_participant_ptr nr_participant_ptr;
typedef nr_participant::nr_participant_info nr_participant_info;
typedef nr_participant::nr_participant_info_ptr nr_participant_info_ptr;


#endif

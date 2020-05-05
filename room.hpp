#ifndef ROOM_H__
#define ROOM_H__

#include "participant.hpp"
#include "protocol.hpp"

class network_room
{
public:

  typedef boost::signals2::signal<void(uint64_t)>::slot_type new_participant_callback_slot_type;

  void join(nr_participant_ptr participant)
  {
    if (participants_.size() >= max_participants_) {
      participant->disconnect();
      return;
    }
    participant->ID_ = rng();
    participant->set_keep_alive(keep_alive_);
    participant->info_ptr_ = nr_participant_info_ptr(new nr_participant_info());
    participant->info_ptr_->id_ = participant->ID_;
    memcpy(participant->info_ptr_->ipv4_ip, participant->IP.c_str(), participant->IP.size());
    participants_.insert(participant);
    signal_new_participant(participant->ID_);
    // begin handhsake
    /*for (auto msg : recent_msgs_) {
      participant->deliver(msg);
      }*/
  }

  void disconnect_participant(nr_participant_ptr participant)
  {
    participants_.erase(participant);
  }

  void disconnect_all_participants()
  {
    std::for_each(participants_.begin(),
		  participants_.end(),
		  [&](nr_participant_ptr p) {
		    p->disconnect();
		  });
  }

  void leave(nr_participant_ptr participant)
  {
    try {
      std::cout << "participant " << participant->ID_ << " leaving room" << std::endl;
      signal_participant_leave(participant->ID_);
      participants_.erase(participant);
    }
    catch (std::exception & e)
      {
	std::cout << "leave exception catched" << e.what() << std::endl;
	// log exception
      }
  }

  void deliver_byte(std::byte *buffer, size_t buffersize)
  {
    if (broadcast_participants_messages)
      {
	for (auto participant : participants_)
	  {
	    participant->deliver_byte(buffer, buffersize);
	  }
      }
  }

  void deliver(const nr_message& msg)
  {
    /*
      recent_msgs_.push_back(msg);
      while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();
    */

    if (broadcast_participants_messages) {
      //signal_new_message();
      for (auto participant : participants_) {
	/*if(!participant->is_deaf())*/ participant->deliver(msg);
      }
    }
  }

  size_t participants_count()
  {
    return participants_.size(); 
  }
     
  int set_max_participants(uint16_t max_participants)
  {
    max_participants_ = max_participants;
    return 0;
  }

  void set_keep_alive(bool keep_alive)
  {
    keep_alive_ = keep_alive;
  }

  bool get_keep_alive()
  {
    return keep_alive_;
  }

  uint16_t get_max_participants()
  {
    return max_participants_;
  }

  void set_message_buffer_size(size_t buffer_size)
  {
    max_recent_msgs = buffer_size;
  }

  size_t get_message_buffer_size()
  {
    return max_recent_msgs;
  }
    
  void set_broadcast_messages(bool broadcast)
  {
    broadcast_participants_messages.store(broadcast);
  }
    
  bool get_broadcast_messages()
  {
    return broadcast_participants_messages;
  }

  void disconnect_participant(uint64_t participant_id)
  {
    nr_participant_ptr p = get_participant_ptr(participant_id);
    if (p != nullptr)
      participants_.erase(p);
  }

  void register_callback_new_participant(const new_participant_callback np_callback_slot)
  {
    signal_new_participant.connect(np_callback_slot);
  }

  void register_callback_new_message(const new_message_callback nm_callback_slot)
  {
    signal_new_message.connect(nm_callback_slot);
  }

  void register_callback_participant_leave(const participant_leave_callback pl_callback_slot)
  {
    signal_participant_leave.connect(pl_callback_slot);
  }

  void get_participant_info(uint64_t participant_id, nr_participant_info *nptr)
  {
    assert(nptr != nullptr);
    // TODO: make participants_ thread safe
    nr_participant_ptr nrpi_ptr = get_participant_ptr(participant_id);
    if ( nrpi_ptr != nullptr)
      {
	nptr->id_ = nrpi_ptr->info_ptr_->id_;
	nptr->server_id_ = nrpi_ptr->info_ptr_->server_id_;
	memset(nptr->ipv4_ip, 0x00, 512);
	memset(nptr->name,0x00,1024);
	memcpy(nptr->ipv4_ip, nrpi_ptr->info_ptr_->ipv4_ip, 512);
	memcpy(nptr->name, nrpi_ptr->info_ptr_->name,1024);
      }
  }

  void disconnect_all_callbacks()
  {
    signal_new_participant.disconnect_all_slots();
    signal_new_message.disconnect_all_slots();
    signal_message_broadcast_message.disconnect_all_slots();
    signal_participant_leave.disconnect_all_slots();
  }

  unsigned char* get_node_location()
  {
    return 0x00;
  }

  int set_max_participant_buffer_size(uint64_t size)
  {
    //TODO: check max min of the buffer size
    max_participant_buffer_size_ = size;
    return NR_OK;
  }

  uint64_t get_max_participant_buffer_size()
  {
    return max_participant_buffer_size_;
  }

  void new_message(uint64_t participant_id, std::byte* buffer, uint32_t buffer_size)
  {
    /*if (b_buffered_messages_)
      {
      new_messages_buffer_mutex.lock();
      messages_buffer_.push_back(message_node_ptr(new message_node(participant_id, buffer, buffer_size)));
      new_messages_buffer_mutex.unlock();
      }
      else
      {*/
    //TODO: this must be changed for a post in asio, if the programmer make a large funciton in the callback the performance will be screw
    new_messages_buffer_mutex.lock();
    signal_new_message(participant_id, (char*)buffer, buffer_size);
    new_messages_buffer_mutex.unlock();
    //}
  }

  int set_participant_deaf(uint64_t participant_id, bool deaf)
  {
    nr_participant_ptr nrpi_ptr = get_participant_ptr(participant_id);
    if ( nrpi_ptr != nullptr) {
      nrpi_ptr->set_deaf(deaf);
      return NR_OK;
    }
    return NR_FAIL;
  }

  bool get_participant_deaf(uint64_t participant_id)
  {
    nr_participant_ptr nrpi_ptr = get_participant_ptr(participant_id);
    if ( nrpi_ptr != nullptr) {
      return nrpi_ptr->is_deaf();
    }
    return false;
  }
  
  ~network_room()
  {
    try {
      disconnect_all_callbacks();
      disconnect_all_participants();
      recent_msgs_.clear();
    }
    catch (std::exception & e)
      {
	std::cerr << "~network_room exeption: " << e.what();
      }
  }
  
private:
    
  nr_participant_ptr get_participant_ptr(uint64_t id)
  {
    std::set<nr_participant_ptr>::iterator it = participants_.find(id);
    if (it == participants_.end())
      return nr_participant_ptr(nullptr);
    return *it;
  }

  boost::signals2::signal<void(uint64_t)> signal_new_participant;
  boost::signals2::signal<void(uint64_t,char*,uint32_t)> signal_message_broadcast_message;
  boost::signals2::signal<void(uint64_t,char*,uint32_t)> signal_new_message;
  boost::signals2::signal<void(uint64_t)> signal_participant_leave;
  std::mt19937_64 rng;
  std::mutex participants_mutex;
  std::set<nr_participant_ptr,std::less<>> participants_;
  uint16_t max_participants_;
  std::atomic_bool broadcast_participants_messages;
  std::atomic_bool keep_alive_ = false;
  size_t max_recent_msgs = 100;
  std::deque<nr_message> nr_message_buffer;
  nr_message_queue recent_msgs_;
    
  //message memory managment
  std::atomic_bool b_buffered_messages_;
  std::mutex new_messages_buffer_mutex;
  std::mutex superbuffer_mutex;
  //std::vector<message_node_ptr> messages_buffer_;
  std::vector<std::vector<std::byte>> superbuffer_;
  size_t max_participant_buffer_size_;
  size_t max_message_buffer_size_;
  size_t super_message_buffer_size_;
  std::atomic_bool b_prealocate_supper_message_buffer_;
}; // end network_room

typedef network_room::new_participant_callback_slot_type new_participant_callback_slot_type;


#endif

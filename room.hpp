#ifndef ROOM_H__
#define ROOM_H__

#include "participant.hpp"
#include "protocol.hpp"
#include "XRMessage.hpp"

class network_room
{
public:
  // SIGNALS
  typedef boost::signals2::signal<void(uint64_t)> signal_on_new_participant;
  signal_on_new_participant signal_new_participant;
  //boost::signals2::signal<void(uint64_t)> signal_new_participant;
  boost::signals2::signal<void(uint64_t,char*,uint32_t)> signal_message_broadcast_message;
  boost::signals2::signal<void(uint64_t,char*,uint32_t)> signal_new_message;
  boost::signals2::signal<void(uint64_t)> signal_participant_leave;

  //boost::signals2::signal<void(uint64_t)> signal_new_participant;
  typedef boost::signals2::signal<void(uint64_t)>::slot_type new_participant_callback_slot_type;

  boost::signals2::connection connect_on_new_participant(const signal_on_new_participant::slot_type &subscriber)
  {
    return signal_new_participant.connect(subscriber); 
  }

  network_room(const network_room&) = delete;
  
  network_room(boost::asio::io_context &_io_context) : m_io_context(_io_context),
						       m_broadcast_last_message(true)
  {
    //connect_on_new_participant(std::bind(&network_room::on_new_participant_room,this,std::placeholders::_1));
    //register_callback_new_participant(this->on_new_participant_room);
    //signal_new_participant.connect(boost::bind(&network_room::on_new_participant_room,this,_1));
    //signal_new_participant.connect(&this->on_new_participant_room);
  }

  void join(nr_participant_ptr participant)
  {
    if (participants_.size() >= max_participants_) {
      std::cout << "max participants reached" << std::endl;
      participant->disconnect();
      return;
    }
    participant->m_id = rng();
    participant->set_keep_alive(keep_alive_);
    participant->info_ptr_ = nr_participant_info_ptr(new nr_participant_info());
    participant->info_ptr_->m_id.store(participant->m_id);
    memcpy(participant->info_ptr_->ipv4_ip, participant->IP.c_str(), participant->IP.size());
    participants_.insert(participant);
    signal_new_participant(participant->m_id);
  }
  
  void on_new_participant_room(const uint64_t np)
  {
  }
  
  void init_new_participant(uint64_t participant_id)
  {
    // this should be on "on_new_participant_room" function
    nr_participant_ptr np = get_participant_ptr(participant_id);
    new_paticipant_to_all(participant_id);
    send_list_of_players(np);
  }
  
  /*warn to all about a new participant in the room*/
  void new_paticipant_to_all(uint64_t np)
  {
    nr_participant_ptr nptr = get_participant_ptr(np);
    //if(auto_update_participants){
    ST_PARTICIPANT_NEW newp;
    newp.participant_id = np;
    newp.descriptor_buffersize = nptr->m_descriptor.size();
    memcpy(newp.descriptor_buffer,nptr->m_descriptor.data(),newp.descriptor_buffersize);
    
    XRMessage msg((uint16_t)EN_RAW_MESSAGE_HEAD::PARTICIPANT_NEW,
		  (std::byte*)&newp,
		  (uint32_t)sizeof(ST_PARTICIPANT_NEW));
    std::cout << ">> PARTICIPANT_NEW " << std::hex << np << " to all." << std::endl;  
    deliver_to_all_except_to_one(msg.data(),msg.size(),np);
  }
  
  void send_list_of_players(nr_participant_ptr ptr) // send list of participants to a new player
  {
    std::vector<ST_PARTICIPANT_NEW> id_vector;
    //if(auto_update_participants){
    ST_PARTICIPANT_NEW newp; // temporal variable
    for(auto p : participants_) {
      if( ptr->m_id == p->m_id ) continue;
      newp.participant_id = p->m_id;
      newp.descriptor_buffersize = p->m_descriptor.size();
      memcpy(newp.descriptor_buffer,p->m_descriptor.data(),newp.descriptor_buffersize);
      id_vector.push_back(newp);
    }
    
    for(auto newp : id_vector){
      XRMessage msg((uint16_t)EN_RAW_MESSAGE_HEAD::PARTICIPANT_NEW,
		    (std::byte*)&newp,
		    (uint32_t)sizeof(ST_PARTICIPANT_NEW));
      
      ptr->deliver_byte(msg.data(),msg.size());
    }
    
  }

  void disconnect_participant(uint64_t participant_id)
  {
    nr_participant_ptr p = get_participant_ptr(participant_id);
    disconnect_participant(p);
  }
  
  void disconnect_participant(nr_participant_ptr participant)
  {
    if(participant != nullptr)
      participants_.erase(participant);
    ST_PARTICIPANT_LEAVE pleave;
    pleave.participant_id = participant->m_id;
    XRMessage msg((uint16_t)EN_RAW_MESSAGE_HEAD::PARTICIPANT_LEAVE,
		  (std::byte*)&pleave,
		  (uint32_t)sizeof(ST_PARTICIPANT_LEAVE));
    deliver_to_all(msg.data(),msg.size());
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
      uint64_t id = participant->m_id;
      signal_participant_leave(id);
      participants_.erase(participant);
      ST_PARTICIPANT_LEAVE pleave;
      pleave.participant_id = id;
      XRMessage msg((uint16_t)EN_RAW_MESSAGE_HEAD::PARTICIPANT_LEAVE,
		    (std::byte*)&pleave,
		    (uint32_t)sizeof(ST_PARTICIPANT_LEAVE));
      deliver_to_all(msg.data(),msg.size());
      std::cout << ">> ST_PARTICIPANT_LEAVE " << std::hex << participant->m_id << std::endl;
    }
    catch (std::exception & e){
      std::cout << "leave exception catched" << e.what() << std::endl;
      // log exception
    }
  }

  void deliver_to_all(std::byte *buffer, size_t buffersize)
  {
    for (auto participant : participants_){
      participant->deliver_byte(buffer, buffersize);
    }
  }

  void deliver_to_all_except_to_one(XRMessage_ptr ptr, uint64_t id_exception)
  {
    deliver_to_all_except_to_one(ptr->data(),ptr->size(),id_exception);
  }
  
  void deliver_to_all_except_to_one(std::byte *buffer, size_t buffersize, uint64_t exception_id)
  {
    for(auto participant: participants_) {
      if(participant->m_id == exception_id) continue;
      participant->deliver_byte(buffer,buffersize);
    }
  }
  
  /*
  boost::asio::post(m_io_context,
		      [&](){
			for(auto participant: participants_) {
			  if(participant->m_id == exception_id) continue;
			  participant->deliver_byte(buffer,buffersize);
			}
		      });  
  */
  
  //void deliver(const nr_message& msg)
  //{
    /*
      recent_msgs_.push_back(msg);
      while (recent_msgs_.size() > max_recent_msgs)
      recent_msgs_.pop_front();
    */

    //if (broadcast_participants_messages) {
  //for (auto participant : participants_) {
  /*if(!participant->is_deaf())*/ //participant->deliver(msg);
  //    }
  //  }
  // }

  void set_auto_update_participants(bool au)
  {
    auto_update_participants = au;
  }

  bool get_auto_update_participants()
  {
    return auto_update_participants;
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
    if ( nrpi_ptr != nullptr) {
      nptr->m_id.store(nrpi_ptr->info_ptr_->m_id);
      nptr->server_id_.store(nrpi_ptr->info_ptr_->server_id_.load());
      memset(nptr->ipv4_ip, 0x00, 512);
      memcpy(nptr->ipv4_ip, nrpi_ptr->info_ptr_->ipv4_ip, 512);
      //TODO: check that the string size < 1024
      memcpy(nptr->name, nrpi_ptr->m_name.c_str(),nrpi_ptr->m_name.size());
    }
  }

  void disconnect_all_callbacks()
  {
    signal_new_participant.disconnect_all_slots();
    signal_new_message.disconnect_all_slots();
    signal_message_broadcast_message.disconnect_all_slots();
    signal_participant_leave.disconnect_all_slots();
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

  // signal a new message to the callback
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
    //TODO: this must be changed for a post in asio, if the programmer make a large function in the callback
    //      the performance will be screw
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

  bool participant_exist(uint64_t participant_id)
  {
    return get_participant_ptr(participant_id).get()!=nullptr;
  }

  ~network_room()
  {
    try {
      disconnect_all_callbacks();
      disconnect_all_participants();
      recent_msgs_.clear();
    }
    catch (std::exception & e) {
      std::cerr << "~network_room exeption: " << e.what();
    }
  }

  void add_rx_message(uint64_t pid, XRMessage_ptr msg)
  {
    rx_queue_mutex.lock();
    rx_queue.insert(std::pair<uint64_t,XRMessage_ptr>(pid,msg));
    rx_queue_mutex.unlock();
  }

private:
  
  nr_participant_ptr get_participant_ptr(uint64_t id)
  {
    std::set<nr_participant_ptr>::iterator it = participants_.find(id);
    if (it == participants_.end())
      return nr_participant_ptr(nullptr);
    return *it;
  }

  std::mt19937_64 rng;
  std::mutex participants_mutex;
  std::set<nr_participant_ptr,std::less<>> participants_;
  uint16_t max_participants_;
  std::atomic_bool broadcast_participants_messages;
  std::atomic_bool m_broadcast_last_message; // 
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
  std::atomic_size_t max_participant_buffer_size_;
  std::atomic_size_t max_message_buffer_size_;
  std::atomic_size_t super_message_buffer_size_;
  std::atomic_bool b_prealocate_supper_message_buffer_;
  std::string m_server_name;
  std::atomic_bool auto_update_participants; // new participnats are updated with the others at them moment they join to the room
  boost::asio::io_context &m_io_context;
  // room queues
  std::mutex rx_queue_mutex;
  std::mutex tx_queue_mutex;
  std::mutex new_participant_list_mutex;
  std::mutex dtaeto_mutex; // deliver to all except to one (dtaeto) mutex
  std::multimap<uint64_t,XRMessage_ptr> rx_queue;
  std::multimap<uint64_t,XRMessage_ptr> tx_queue;
  std::list<uint64_t> new_participant_list;
  std::multimap<uint64_t,XRMessage_ptr> deliver_to_all_except_to_one_map;
};// end network_room

typedef network_room::new_participant_callback_slot_type new_participant_callback_slot_type;


#endif

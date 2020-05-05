// nr_service.cpp : This file contains the Network Relay codes. 
//
//
// nr_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2020 Juan Pablo Cordova E. (jpcordovae at gmail dot com)
//
// Distributed under MIT License
//

#include "nr_server.hpp"
#include "XRNetworkLobbyService.hpp"
#include <atomic>

#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

//----------------------------------------------------------------------

#define BUFFER_SIZE 1024

using namespace std;
xrnls_ptr xrnls;
std::atomic_bool b_instantiated;

void OnNewParticipant(uint64_t participant_id)
{
  nr_participant_info *info = new nr_participant_info();
  xrnls->get_participant_info(participant_id,info);
  if(!info) return;
  cout << "----- New Participant event ID: " << participant_id << ", IP: " << std::string(info->ipv4_ip) << endl;
}

void OnNewMessage(uint64_t participant_id, char *buffer, uint32_t buffer_size)
{
  //cout << "----- New Message from participant " << participant_id << ", bytes: " << buffer_size << endl;
}

void OnParticipantLeave(uint64_t participant_id)
{
  cout << "----- participant " << std::to_string(participant_id) << " leaving room." << endl;
}

void OnServiceStarted()
{
  cout << "----- service has stated." << endl;
  if (!b_instantiated) {
    xrnls->register_callback_new_participant(OnNewParticipant);
    xrnls->register_callback_new_message(OnNewMessage);
    xrnls->register_callback_participant_leave(OnParticipantLeave);
    b_instantiated = true;
  }
}

void clean_callbacks()
{
    
}

void cleaning(void)
{
  xrnls->stop();
}

int main(int argc, char* argv[])
{
  int port = 1080;
  char c;
    
  xrnls = xrnls_ptr(new XRNetworkLobbyService());
    
  xrnls->set_port(port);
  xrnls->set_socket_no_delay(true);
  bool broadcast_to_all = false;
  nr_participant_info *pinfo = new nr_participant_info();
  std::cout << "----------------------" << std::endl;
  std::cout << " Unit test nr_service " << std::endl;
  std::cout << "----------------------" << std::endl;
  std::cout << "press h for help" << std::endl;
  std::cout << std::endl;

  do {
    c = getchar();
    switch (c)
      {
      case 'b':
      case 'B':
	if (!xrnls) continue;
	broadcast_to_all = !xrnls->get_broadcasting_all_messages();
	std::cout << "setting 'broadcast to all' to: " << broadcast_to_all << " ";
	if (xrnls->set_broadcasting_all_messages(broadcast_to_all) != NR_OK)
	  std::cout << "[FAIL] service not started";
	std::cout << std::endl;
	break;
      case 'n':
      case 'N':
	cout << "starting service\r\n";
	if (xrnls) {
	  xrnls->start();
	  if(!b_instantiated) xrnls->register_callback_on_running(OnServiceStarted);
	}
	break;
      case 's':
      case 'S':
	cout << "stopping service\r\n";
	if (xrnls) {
	  xrnls->stop();
	}
	break;
      case 'd':
      case 'D':
	cout << "disconnect all participants\r\n";
	if (xrnls)
	  xrnls->disconnect_all_participants();
	break;
      case 'h':
      case 'H':
	std::cout << "n: start service" << std::endl
		  << "s: stop service" << std::endl
		  << "c: register callbacks" << std::endl
		  << "d: disconnect all participants" << std::endl
		  << "b: auto-broadcast to all participants (offline)" << std::endl
		  << "l: list all participants (online)" << std::endl;
	    
	break;
      case 'l':
      case 'L':
	std::cout << "--- " << xrnls->participants_count() << " Participants :" << std::endl;
	;
	/*for(int i=0; i < participants_list_copy.size(); i++) {
	  xrnls->get_participant_info(pinfo);
	  std::cout << "ID: " << pinfo.id_ << ", IP: " << 
	  }*/
	break;
      }
    //std::cout << std::endl;
  } while (c != '`');

  //atexit(cleaning);
  //exit(EXIT_SUCCESS);
}

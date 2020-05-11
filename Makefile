CPPFLAGS=-gdwarf -Wall -pedantic -std=c++17 -DBOOST_LOG_DYN_LINK
LIBS = -lpthread -lboost_log -lboost_log_setup -Wno-reorder

#TARGET = nr_service

all:
	g++ -o nr_service nr_service_console.cpp nr_base.hpp participant.hpp room.hpp session.hpp handshake.hpp protocol.hpp nr_server.hpp XRNetworkLobbyService.hpp error_messages.hpp message.hpp message_node.hpp $(CPPFLAGS) $(LIBS)

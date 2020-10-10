CPPFLAGS=-gdwarf -Wall -pedantic -std=c++17 -DBOOST_LOG_DYN_LINK
LIBS = -lpthread -lboost_log -lboost_log_setup -Wno-reorder -Wno-memset-elt-size

#TARGET = nr_service

all: nr_service client

nr_service:
	g++ -o nr_service nr_service_console.cpp nr_base.hpp participant.hpp room.hpp session.hpp handshake.hpp protocol.hpp nr_server.hpp XRNetworkLobbyService.hpp error_messages.hpp XRMessage.hpp $(CPPFLAGS) $(LIBS)

client:
	g++ -o nr_client nr_client.cpp nr_base.hpp participant.hpp protocol.hpp XRMessage.hpp $(CPPFLAGS) -I../ $(LIBS)

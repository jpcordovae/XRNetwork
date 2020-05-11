#ifndef MESSAGE_NODE_H__
#define MESSAGE_NODE_H__

#include "message.hpp"
#include <chrono>
#include <cstdlib>
#include <cstddef>
#include <memory>

struct message_node {
    std::chrono::time_point<std::chrono::high_resolution_clock> timestamp;
    uint64_t participant_id_;
    uint64_t buffersize_;
    std::byte* buffer_;
    message_node() : timestamp(std::chrono::high_resolution_clock::now()),
		     buffer_(NULL),
		     buffersize_(0),
		     participant_id_(0)
    { }

    message_node(uint64_t part_id, std::byte* buffer, const size_t buffersize) : timestamp(std::chrono::high_resolution_clock::now())
    {
        participant_id_ = part_id;
        buffersize_ = buffersize;
        buffer_ = new std::byte[buffersize_];
        memset(buffer_, 0x00, buffersize_);
        memcpy(buffer_, buffer, buffersize);
    }

    ~message_node()
    {
        delete[] buffer_;
    }
};

typedef std::shared_ptr<message_node> message_node_ptr;
//typedef message_node::message_node_ptr message_node_ptr;


#endif

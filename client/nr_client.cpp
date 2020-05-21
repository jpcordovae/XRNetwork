//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <deque>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include "message.hpp"

using boost::asio::ip::tcp;
using boost::asio::ip::udp;

typedef std::deque<nr_message> chat_message_queue;

class chat_client
{
public:
    chat_client(boost::asio::io_context& io_context, const tcp::resolver::results_type& endpoints) 
        : io_context_(io_context), socket_(io_context)
    {
        read_buffer_ = std::shared_ptr<std::vector<std::byte>>(new std::vector<std::byte>());
        write_buffer_ = std::shared_ptr<std::vector<std::byte>>(new std::vector<std::byte>());
        read_buffer_->resize(1024 * 64);
        write_buffer_->resize(1024 * 64);
        do_connect(endpoints);
    }

    void write_byte(std::vector<std::byte> &buffer)
    {
        boost::asio::post(  io_context_,
                            [this, buffer]()
                            {
                                bool write_in_progress = !deque_write_buffer_.empty();
                                std::shared_ptr<std::vector<std::byte>> buffer2(new std::vector<std::byte>());
                                buffer2->resize(1024 * 64);
                                memset(buffer2->data(), 0x00, 1024 * 64);
                                std::copy(buffer.begin(), buffer.end(), std::begin(*buffer2));
                                deque_write_buffer_.push_back(buffer2);
                                memset((void*)buffer.data(), 0x00, buffer.capacity());
                                if (!write_in_progress)
                                {
                                    do_write_byte();
                                }
                                
                            });
    }

    /*void write(const nr_message& msg)
    {
        boost::asio::post(io_context_,
                        [this, msg]()
                        {
                            bool write_in_progress = !write_msgs_.empty();
                            write_msgs_.push_back(msg);
                            if (!write_in_progress)
                            {
                                do_write();
                            }
                        });
    }*/

    void close()
    {
        boost::asio::post(io_context_, [&]() { socket_.close(); });
        std::cout << "closing" << std::endl;
    }

private:
    void do_connect(const tcp::resolver::results_type& endpoints)
    {
        //std::cout << "connecting to " << socket_.remote_endpoint().address().to_string() << " at port " << socket_.remote_endpoint().port() << std::endl;
        boost::asio::async_connect( socket_, endpoints,
                                    [this](boost::system::error_code ec, tcp::endpoint)
                                    {
                                        if (!ec)
                                        {
                                            std::cout << "connected !!!\r\n";
                                            //do_read_header();
                                            do_read_byte();
                                        }
                                        else
                                        {
                                            std::cout << "failed to connect" << std::endl;
                                            this->close();
                                        }
                                    });
    }

    void do_read_byte()
    {
        size_t cap = read_buffer_->capacity();
        //std::cout << "Capacity: " << std::to_string(cap) << " bytes\r\n";
        boost::asio::async_read(
            socket_,
            boost::asio::buffer(read_buffer_->data(),read_buffer_->capacity()),
            [this](boost::system::error_code ec, size_t writesize) {
            if (!ec)
            {

                std::cout << "----- read " << std::to_string(writesize) << " bytes\r\n";
                read_buffer_->clear();
                do_read_byte();
            }
            else {
                std::cerr << "do_read error";
                this->close();
            }
        });
    }

    /*void do_read_header()
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.data(), nr_message::header_length),
                                [this](boost::system::error_code ec, std::size_t lenght)
                                {
                                    if (!ec && read_msg_.decode_header())
                                    {
                                        do_read_body();
                                    }
                                    else
                                    {
                                        std::cout << "disconnected" << std::endl;
                                        //socket_.close();
                                        this->close();
                                    }
                                });
    }

    void do_read_body()
    {
        boost::asio::async_read(socket_,
                                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                                [this](boost::system::error_code ec, std::size_t length)
                                {
                                    if (!ec)
                                    {
                                        std::cout.write(read_msg_.body(), read_msg_.body_length());
                                        std::cout << "\n";
                                        do_read_header();
                                    }
                                    else
                                    {
                                        std::cout << "async_read error" << std::endl;
                                        //socket_.close();
                                        this->close();
                                    }
                                });
    }*/

    void do_write_byte()
    {
        //std::cout << "capacity: " << deque_write_buffer_.front()->capacity() << std::endl;
        boost::asio::async_write(socket_,
                                boost::asio::buffer(deque_write_buffer_.front()->data(), deque_write_buffer_.front()->capacity()),
                                [this](boost::system::error_code ec, std::size_t sentbytes)
                                {
                                    if (!ec)
                                    {
                                        deque_write_buffer_.pop_front();
                                        if (!deque_write_buffer_.empty())
                                        {
                                            do_write_byte();
                                        }
                                        std::cout << " sent " << std::to_string(sentbytes) << " bytes\r\n";
                                    }
                                    else
                                    {
                                        std::cout << "do_write error" << std::endl;
                                        this->close();
                                    }
                                });
    }

   /* void do_write()
    {
        boost::asio::async_write(socket_,
                                boost::asio::buffer(write_msgs_.front().data(),write_msgs_.front().length()),
                                [this](boost::system::error_code ec, std::size_t length)
                                {
                                    if (!ec)
                                    {
                                        write_msgs_.pop_front();
                                        if (!write_msgs_.empty())
                                        {
                                            do_write();
                                        }
                                        std::cout << " sent " << std::to_string(lenght) << " bytes\r\n";
                                        write_buffer_->clear();;
                                    }
                                    else
                                    {
                                        std::cout << "do_write error" << std::endl;
                                        this->close();
                                    }
                                });
    }*/

private:
    boost::asio::io_context& io_context_;
    tcp::socket socket_;
    //udp::socket udp_socket_;
    std::shared_ptr<std::vector<std::byte>> write_buffer_;
    std::shared_ptr<std::vector<std::byte>> read_buffer_;
    
    std::deque<std::shared_ptr<std::vector<std::byte>>> deque_write_buffer_;
    std::deque<std::shared_ptr<std::vector<std::byte>>> deque_read_buffer_;

    nr_message read_msg_;
    chat_message_queue write_msgs_;
}; // end chat_client

int main(int argc, char* argv[])
{
    try
    {
        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        //auto endpoints = resolver.resolve(argv[1], argv[2]);
        //auto endpoints = resolver.resolve("localhost", "1080");
        auto endpoints = resolver.resolve("139.162.56.62", "1080");
        chat_client c(io_context, endpoints);
        
        std::thread t([&io_context]() { io_context.run(); });

        //char line[nr_message::max_body_length + 1];
        char line[64 * 1024];
        std::vector<std::byte> ptr;
        while (std::cin.getline(line, 1024) )
        {
            /*nr_message msg;
            msg.body_length(std::strlen(line));
            std::memcpy(msg.body(), line, msg.body_length());
            msg.encode_header();
            c.write(msg);*/
            ptr.insert(std::begin(ptr),(std::byte*)line, (std::byte*)line + strlen(line));
            c.write_byte(ptr);
            ptr.clear();
            memset(line, 0x00, 64 * 1024);
            //407 273 9676
        }

        c.close();
        t.join();
    }
    catch (std::exception & e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
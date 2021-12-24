#pragma once
#if !defined(_SERVER_HPP_)
#define _SERVER_HPP_

#include <boost/asio.hpp>
#include <memory>
#include "clientUI.hpp"

#if BOOST_VERSION < 106600
using IoContext = boost::asio::io_service;
#else
using IoContext = boost::asio::io_context;
#endif

class ClientUI;
class ConnectHandler;
class Client : boost::asio::noncopyable {
public:
   Client(std::string address, unsigned int port)
      : iocontext_(), address_(address), port_(port),
        ui_(*this), connect_(std::make_shared<ConnectHandler>(*this)) {}
   void run();
   void connect(std::string param);
   void connect();
   void disconnect();
   bool is_connected() { return connected_; }
   void set_port(int port);
   void send_msg(std::vector<std::string> data);
   void set_connected(bool connected) { connected_ = connected;}
   bool get_connected() { return connected_;}
   bool set_address(std::string address);
   int get_port() { return port_; }
   std::string get_address() { return address_; }
   IoContext &getIOcontext(){return iocontext_;}
   std::string print_prompt();               // trick for print cli name
private:
   boost::asio::ip::tcp::acceptor *acceptor_ = nullptr;
   IoContext iocontext_;
   ClientUI ui_;
   std::shared_ptr<ConnectHandler> connect_;
   unsigned int port_;
   std::string address_;
   bool connected_ = false;
};



class ConnectHandler : public std::enable_shared_from_this<ConnectHandler>, boost::asio::noncopyable {
public:
   ConnectHandler(Client &client) 
         : client_(client), sock_(client.getIOcontext()), timer_(client.getIOcontext()) {
   }
   boost::asio::ip::tcp::socket &socket() {return sock_;}
   void read();
   void write();
   void send(boost::asio::ip::tcp::endpoint ep, std::string msg);
   void connect(boost::asio::ip::tcp::endpoint ep);
   void handle_read(const boost::system::error_code &err, size_t bytes_transferred);
   void handle_write(const boost::system::error_code &err, size_t bytes_transferred);
   void on_connect(const boost::system::error_code &err);
   void ping(const boost::system::error_code &err);
   void postpone_ping();
   void close();
private:
   Client &client_;
   boost::asio::ip::tcp::socket sock_;
   std::string msg_;
   std::string received_;
   boost::asio::ip::tcp::endpoint ep_;
   boost::asio::deadline_timer timer_;
   bool started_;
   enum {max_length = 1024};
   char data_[max_length];
};

class TimeStamp {
   public:
      static const std::string getTimeStamp();
};

#endif // _SERVER_HPP_
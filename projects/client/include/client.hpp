#pragma once
#if !defined(_SERVER_HPP_)
#define _SERVER_HPP_

#include <boost/asio.hpp>

#if BOOST_VERSION < 106600
using IoContext = boost::asio::io_service;
#else
using IoContext = boost::asio::io_context;
#endif

class ConectHandler;
class Client : std::enable_shared_from_this<Client>, boost::asio::noncopyable {
public:
   Client(IoContext &iocontext, std::string address, unsigned int port)
      : iocontext(iocontext), address_(address), port_(port) {}
   void handle_accept(std::shared_ptr<ConectHandler> con, const boost::system::error_code &err);

   void start();
   void stop();
   void connect(std::string param);
   bool isStarted() { return started_; }
   void set_port(int port);
   int getPort() { return port_; }
   std::string get_address() { return address_; }
   bool set_address(std::string address);
   void send_msg(std::vector<std::string> msg);
private:
   boost::asio::ip::tcp::acceptor *acceptor_ = nullptr;
   IoContext &iocontext;
   int port_ = 80;                // TODO: change
   std::string address_ = "127.0.0.1"; // TODO: change
   bool started_ = false;
   void start_accept();
};

class ConectHandler : public std::enable_shared_from_this<ConectHandler>, boost::asio::noncopyable {
public:
   ConectHandler(IoContext &iocontext) : sock(iocontext) {}
   ConectHandler(IoContext &iocontext, std::string msg) : sock(iocontext), msg(msg) {
   }
   boost::asio::ip::tcp::socket &socket() {return sock;}
   void read();
   void write();
   void send(boost::asio::ip::tcp::endpoint ep);
   void handle_read(const boost::system::error_code &err, size_t bytes_transferred);
   void handle_write(const boost::system::error_code &err, size_t bytes_transferred);
   void on_connect(const boost::system::error_code &err);
private:
   boost::asio::ip::tcp::socket sock;
   std::string msg = "";
   enum {max_length = 1024};
   char data[max_length];
};

class TimeStamp {
   public:
      static const std::string getTimeStamp();
};

#endif // _SERVER_HPP_
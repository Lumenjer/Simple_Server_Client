#pragma once
#if !defined(_SERVER_HPP_)
#define _SERVER_HPP_

#include <boost/asio.hpp>

#if BOOST_VERSION < 106600
using IoContext = boost::asio::io_service;
#else
using IoContext = boost::asio::io_context;
#endif

class ConnectHandler;
class ServerUI;
class Server : std::enable_shared_from_this<Server>, boost::asio::noncopyable
{
public:
   void printPromt();
   // constructor for accepting connection from client
   Server(IoContext &iocontext, std::string address, unsigned int port)
       : iocontext(iocontext), address_(address), port_(port), ui(nullptr) {}
   void handle_accept(std::shared_ptr<ConnectHandler> con, const boost::system::error_code &err);

   bool isStarted() { return started_; }
   int getPort() { return port_; }
   std::string get_address() { return address_; }

   void start();
   void stop();
   void set_port(unsigned int port);
   void set_UI(ServerUI *ui) {this->ui = ui;}
private:
   boost::asio::ip::tcp::acceptor *acceptor_ = nullptr;
   IoContext &iocontext;
   ServerUI *ui;
   unsigned int port_ = 80;                // TODO: change
   std::string address_ = "127.0.0.1"; // TODO: change
   bool started_ = false;
   void start_accept();
};

class ConnectHandler : public std::enable_shared_from_this<ConnectHandler>, boost::asio::noncopyable {
private:
   boost::asio::ip::tcp::socket sock;
   std::string msg;
   enum {max_length = 1024};
   char data[max_length];
   Server &server;

public:
   ConnectHandler(IoContext &iocontext, Server &server) : sock(iocontext), server(server) {}
   boost::asio::ip::tcp::socket &socket() {return sock;}

   void read();
   void write();
   void handle_read(const boost::system::error_code &err, size_t bytes_transferred);
   void handle_write(const boost::system::error_code &err, size_t bytes_transferred);
};

class TimeStamp {
   public:
      static const std::string getTimeStamp();
};

#endif // _SERVER_HPP_
#pragma once
#if !defined(_SERVER_HPP_)
#define _SERVER_HPP_

#include <boost/asio.hpp>
#include "serverUI.hpp"

using IoContext = boost::asio::io_context;

class ConnectHandler;
class ServerUI;
class Server : boost::asio::noncopyable
{
public:

   Server(std::string address, unsigned int port)
      : iocontext_(), address_(address), port_(port), ui_(*this) {
      }
   void handle_accept(std::shared_ptr<ConnectHandler> con, const boost::system::error_code &err);
   void run();
   bool is_started() { return started_; }
   void set_started(bool started) { started_ = started;}
   int get_port() { return port_; }
   std::string get_address() { return address_; }
   IoContext &getIOcontext(){return iocontext_;}
   std::string print_prompt();
   void start();
   void stop();
   void set_port(unsigned int port);
private:
   boost::asio::ip::tcp::acceptor *acceptor_ = nullptr;
   IoContext iocontext_;
   ServerUI ui_;
   std::string remote_ep;
   unsigned int port_;                // TODO: change
   std::string address_; // TODO: change
   bool started_ = false;
   void start_accept();
};

class ConnectHandler : public std::enable_shared_from_this<ConnectHandler>, boost::asio::noncopyable {


public:
   ConnectHandler(Server &server) 
         : server_(server), sock_(server.getIOcontext()), timer_(server.getIOcontext()) {}
   boost::asio::ip::tcp::socket &socket() {return sock_;}
   void read();
   void write(std::string msg);
   void handle_read(const boost::system::error_code &err, size_t bytes_transferred);
   void handle_write(const boost::system::error_code &err, size_t bytes_transferred);
   void close();
   void close(const boost::system::error_code &err);
   void postpone_close();

private:
   Server &server_;
   boost::asio::ip::tcp::socket sock_;
   std::string msg_;
   boost::asio::deadline_timer timer_;
   enum {max_length = 1024};
   char data_[max_length];
};

class TimeStamp {
   public:
      static const std::string getTimeStamp();
};

#endif // _SERVER_HPP_
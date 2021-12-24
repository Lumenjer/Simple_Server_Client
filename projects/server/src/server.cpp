#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <memory>
#include <chrono>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include "server.hpp"
#include "serverUI.hpp"

using namespace std;
using namespace boost::asio::placeholders;
using err_code = boost::system::error_code;
using end_point = boost::asio::ip::tcp::endpoint;


string Server::print_prompt() {
   ui_.Prompt();
   return "";
}

void Server::start_accept() {
   shared_ptr<ConnectHandler> connect = make_shared<ConnectHandler>(*this);
   acceptor_->async_accept(connect->socket(),
                           boost::bind(&Server::handle_accept, this, connect,
                                       error));
}

void Server::handle_accept(shared_ptr<ConnectHandler> connect, const err_code &err) {
   if (!started_)
      return;
   if (!err) {
      connect->read();
   }
   start_accept();
}

void Server::run(){
   auto work = boost::asio::make_work_guard(iocontext_);
   iocontext_.run();
}

void Server::start() {
   if (started_) {
      cout << "Server already started! \n";
      return;
   }
   else {
      err_code ec;
      boost::asio::ip::address addr = boost::asio::ip::make_address(address_, ec);
      if (ec){
         cerr << "Can't start, wrong address, error " << ec << endl;
      }
      acceptor_ = new boost::asio::ip::tcp::acceptor(iocontext_, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(address_), port_));
      start_accept();
      started_ = true;
   }
   cout << "Server started! \n";
   cout << "Address: " << address_.c_str() << endl;
   cout << "Port: " << port_ << endl;
}

void Server::stop() {
   if (!started_)
      cout << "Server already stopped!\n";
   else {
      err_code ec;
      acceptor_->close(ec);
      if (ec) {
         cerr << "Acceptor close error: " << ec << endl;
         return;
      }
      delete acceptor_;
      started_ = false;
      cout << "Server stopped!\n";
   }
}

void Server::set_port(unsigned int port) {
   cout << "Old port: " << port_ << ", new port: " << port << endl;
   if (port_ == port){
      cout << "Same port, no changes";
      return;
   }
   if (started_) {
      cout << "Server running now, restarting with new port\n";
      stop();
      port_ = port;
      start();
   }
   else
      port_ = port;
   cout << "Port changed \n";
}

void ConnectHandler::read() {
   if (!server_.is_started())
      return;
   sock_.async_read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(&ConnectHandler::handle_read,
                  shared_from_this(),
                  error,
                  bytes_transferred));
   postpone_close();
}

void ConnectHandler::write(string msg) {
   if (!server_.is_started())
      return;
   msg_ = msg;
   sock_.async_write_some(
      boost::asio::buffer(msg_, max_length),
      boost::bind(&ConnectHandler::handle_write,
                  shared_from_this(),
                  error,
                  bytes_transferred));
   postpone_close();
}

void ConnectHandler::handle_read(const err_code &err, size_t bytes_transferred) {
   if (!server_.is_started())
      return;
   if (err) {
      cerr << "Read error: " << err.message() << endl << server_.print_prompt();
      close();
      return;
   }
   string tmp(data_, bytes_transferred);
   if (tmp == "connect") {
      write("connected");
      cout << endl << TimeStamp::getTimeStamp() << " Client from "
           << sock_.remote_endpoint() << " connected"
           << endl << server_.print_prompt();
   }
   else if (tmp == "ping"){
      write("ping");
   }
   else{
      write("OK");
      cout << endl << TimeStamp::getTimeStamp() << " Message from " 
           << sock_.remote_endpoint() << " with data:" << endl
           << tmp << endl << server_.print_prompt();
   }
}

void ConnectHandler::handle_write(const err_code &err, size_t bytes_transferred) {
   if (!server_.is_started())
      return;
   if (err) {
      cerr << "\nWrite error: " << err.message() << endl << server_.print_prompt();
      close();
      return;
   }
   if (server_.is_started())
      read();
   else
      close();
}


void ConnectHandler::close(const err_code &err){
   if (err)
      return;
   close();
}
void ConnectHandler::close(){
   cout << endl << TimeStamp::getTimeStamp() << " Client from "
      << sock_.remote_endpoint() << " disconnected"
      << endl << server_.print_prompt();
   err_code err;
   timer_.cancel(err);
   shared_from_this().reset();
}

void ConnectHandler::postpone_close(){
   timer_.expires_from_now(boost::posix_time::millisec(10000));
	timer_.async_wait(boost::bind(&ConnectHandler::close, shared_from_this(), error));
}

const string TimeStamp::getTimeStamp() {
   time_t now = time(0);
   struct tm tstruct;
   char buf[80];
   tstruct = *localtime(&now);
   strftime(buf, sizeof(buf), "[%Y-%m-%d.%X]", &tstruct);
   return buf;
}

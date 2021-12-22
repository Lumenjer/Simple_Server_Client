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



void Server::printPromt() {
   if (ui)
      ui->Prompt();
}

void Server::start_accept() {
   shared_ptr<ConnectHandler> connect = make_shared<ConnectHandler>(iocontext, *this);
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

void Server::start() { // TODO: change
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
      acceptor_ = new boost::asio::ip::tcp::acceptor(iocontext, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(address_), port_));
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
      if (ec)
      {
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
   sock.async_read_some(
      boost::asio::buffer(data, max_length),
      boost::bind(&ConnectHandler::handle_read,
                  shared_from_this(),
                  error,
                  bytes_transferred));
}

void ConnectHandler::write() {
   msg = data;
   sock.async_write_some(
      boost::asio::buffer(msg, max_length),
      boost::bind(&ConnectHandler::handle_write,
                  shared_from_this(),
                  error,
                  bytes_transferred));
}

void ConnectHandler::handle_read(const err_code &err, size_t bytes_transferred) {
   if (!err) {
      cout << endl << TimeStamp::getTimeStamp().c_str() << " Message from " 
         << sock.remote_endpoint().address().to_string().c_str() << " with data:" << endl
         <<  string(data, bytes_transferred) << endl;
      server.printPromt();
   }
   else {
      cerr << "error: " << err.message() << endl;
      sock.close();
   }
   write();
}

void ConnectHandler::handle_write(const err_code &err, size_t bytes_transferred) {
   if (err) {
      cerr << "error: " << err.message() << endl;
      sock.close();
   }
}

const string TimeStamp::getTimeStamp() {
   time_t now = time(0);
   struct tm tstruct;
   char buf[80];
   tstruct = *localtime(&now);
   strftime(buf, sizeof(buf), "[%Y-%m-%d.%X]", &tstruct);
   return buf;
}

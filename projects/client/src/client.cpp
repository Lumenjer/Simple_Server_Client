#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <memory>
#include <chrono>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <vector>
#include "client.hpp"

using namespace std;
using namespace boost::asio::placeholders;
using err_code = boost::system::error_code;
using end_point = boost::asio::ip::tcp::endpoint;

void Client::start_accept() {
    // socket
    shared_ptr<ConectHandler> con = make_shared<ConectHandler>(iocontext);

    // asynchronous accept operation and wait for a new connection.
    acceptor_->async_accept(con->socket(),
                            boost::bind(&Client::handle_accept, this, con,
                                        error));
}

void Client::send_msg(vector<string> data){
   string msg;
   for (auto str : data){
      msg += str + " ";
   }
   shared_ptr<ConectHandler> con = make_shared<ConectHandler>(iocontext, msg);
   con->send(end_point(boost::asio::ip::make_address(address_), port_));

}

void Client::handle_accept(shared_ptr<ConectHandler> con, const err_code &err) {
   if (!started_) 
      return;
   if (!err) {
      con->read();
   }
   start_accept();
}


void Client::start() { // TODO: change
   if (started_) {
      cout << "Client already started! \n";
      return;
   }
   else {
      started_ = true;
      acceptor_ = new boost::asio::ip::tcp::acceptor(iocontext, end_point(boost::asio::ip::make_address(address_), port_));
      start_accept();
   }
   cout << "Client started! \n";
}

void Client::stop() {
   if (!started_)
      cout << "Client already stopped!\n";
   else {
      err_code ec;
      acceptor_->close(ec);
      if (ec) {
      cerr << "Acceptor close error: " << ec << endl;
      return;
      }
      delete acceptor_;
      started_ = false;
      cout << "Client stopped!\n";
   }
}

void Client::set_port(int port) {
   cout << "Port " << port << endl;
   port_ = port;
}

bool Client::set_address(string address) {
   err_code ec;
   boost::asio::ip::make_address(address, ec);
   if (ec){
      cout << "Wrong address, error " << ec << endl;
      return false;
   }
      cout << "New address " << address << endl;
      address_ = address;
   return true;
}

void Client::connect(string param){
   auto idx = param.find(':');
   auto dot_qt = count(param.begin(), param.end(), '.');
   if (idx < 6 || idx > 14 || dot_qt != 3) {
      cout << "Wrong parametr or format, try again!" << endl
            << "Example: 127.0.0.1:9999\n";
      return;
   }
   string address = param.substr(0, idx);
   if (!set_address(address))
      return;
   unsigned long port = stoul(param.substr(idx+1, param.length()-1));
   if (port > 65535 ) {
      cout << "Port oversize or negative numbers, try again!\n";
      return;
   }
   set_port(port);
}


void ConectHandler::on_connect(const err_code &err){
   if (!err){
      sock.async_write_some(
         boost::asio::buffer(msg, max_length),
         boost::bind(&ConectHandler::handle_write,
                     shared_from_this(),
                     error,
                     bytes_transferred));
   }
}

void ConectHandler::read() {
   sock.async_read_some(
      boost::asio::buffer(data, max_length),
      boost::bind(&ConectHandler::handle_read,
                  shared_from_this(),
                  error,
                  bytes_transferred));
}

void ConectHandler::send(end_point ep) {
   sock.async_connect(ep,
            boost::bind(&ConectHandler::on_connect,
                     shared_from_this(),
                     error));
}
void ConectHandler::write() {
   msg = data;
   sock.async_write_some(
      boost::asio::buffer(msg, max_length),
      boost::bind(&ConectHandler::handle_write,
                  shared_from_this(),
                  error,
                  bytes_transferred));
}

void ConectHandler::handle_read(const err_code &err, size_t bytes_transferred) {
   if (!err) {
      cout << TimeStamp::getTimeStamp().c_str() 
         << " Message from " 
         << sock.remote_endpoint().address().to_string().c_str() << endl 
         << data << endl;
   }
   else {
      cerr << "error: " << err.message() << endl;
      sock.close();
   }
   write();
}

void ConectHandler::handle_write(const err_code &err, size_t bytes_transferred) {
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

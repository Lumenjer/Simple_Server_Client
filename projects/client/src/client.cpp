#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
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

// struct Prompt {
//    Prompt(ClientUI &ui) : ui_(ui){}
//    ClientUI &ui_;
//    std::string operator() () const{
//       ui_.Prompt();
//       return "";   
//    }
// };

// std::ostream& operator<<(std::ostream& out, Prompt &prompt){
//    out << endl;
//    prompt();
//    return out;
// }


// void Client::start_accept() {
//     // socket
//     shared_ptr<ConnectHandler> con = make_shared<ConnectHandler>(iocontext_);

//     // asynchronous accept operation and wait for a new connection.
//     acceptor_->async_accept(connect_->socket(),
//                             boost::bind(&Client::handle_accept, this, con,
//                                         error));
// }


// void Client::handle_accept(shared_ptr<ConnectHandler> con, const err_code &err) {
//    if (!connected_) 
//       return;
//    if (!err) {
//       connect_->read();
//    }
//    start_accept();
// }


// void Client::start() { // TODO: change
//    if (connected_) {
//       cout << "Client already started! \n";
//       return;
//    }
//    else {
//       connected_ = true;
//       acceptor_ = new boost::asio::ip::tcp::acceptor(iocontext_, end_point(boost::asio::ip::make_address(address_), port_));
//       start_accept();
//    }
//    cout << "Client started! \n";
// }

// void Client::stop() {
//    if (!connected_)
//       cout << "Client already stopped!\n";
//    else {
//       err_code ec;
//       acceptor_->close(ec);
//       if (ec) {
//       cerr << "Acceptor close error: " << ec << endl;
//       return;
//       }
//       delete acceptor_;
//       connected_ = false;
//       cout << "Client stopped!\n";
//    }
// }

void Client::run(){
   cout << "\rDefault address: " << address_ << ":" << port_ << endl << print_prompt();
#if BOOST_VERSION < 106600
   boost::asio::io_service::work work(iocontext_);
#else
   auto work = boost::asio::make_work_guard(iocontext_);
#endif
   iocontext_.run();
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
   cout << "Address " << address << endl;
   address_ = address;
   return true;
}
void Client::connect(){
   connect_->connect(end_point(boost::asio::ip::make_address(address_), port_));
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
      cout << "Port oversize or negative number, try again!\n";
      return;
   }
   set_port(port);
   // shared_ptr<ConnectHandler> con = make_shared<ConnectHandler>(*this);
   connect_->connect(end_point(boost::asio::ip::make_address(address_), port_));
}

void Client::send_msg(vector<string> data){
   string msg;
   for (auto str : data){
      msg += str + " ";
   }
   // shared_ptr<ConnectHandler> con = make_shared<ConnectHandler>(*this);
   if (!connected_) {
      connect_->connect(end_point(boost::asio::ip::make_address(address_), port_));
   }
   connect_->send(end_point(boost::asio::ip::make_address(address_), port_), msg);
   cout << "Sent to " << address_ << ":" << port_ << " with data: " << msg << endl;

}

string Client::print_prompt() {
   ui_.Prompt();
   return "";
}

void ConnectHandler::ping(const err_code &err){
   // if (err){
   //    cerr << "\rPing error: " << err.message() << endl << client_.print_prompt();
   //    sock_.close();
   //    client_.set_connected(false);
   //    return;
   // }
   if (started_ || err)
      return;
   msg_= "ping";
   sock_.async_connect(ep_,
            boost::bind(&ConnectHandler::on_connect,
                     shared_from_this(),
                     error));
   cout << "\rping    \n";
}
void ConnectHandler::postpone_ping(){
   timer_.expires_from_now(boost::posix_time::millisec(5000));
	timer_.async_wait(boost::bind(&ConnectHandler::ping, shared_from_this(), error));
}


void ConnectHandler::send(end_point ep, string msg) {
   if (started_)
      return;
   ep_ = ep;
   msg_ = msg;
   sock_.async_connect(ep,
            boost::bind(&ConnectHandler::on_connect,
                     shared_from_this(),
                     error));
}
void ConnectHandler::on_connect(const err_code &err){
   if (err){
      cerr << (client_.get_connected() ? "Connection lost" : "Can't connect, check server address and port") 
           << endl << client_.print_prompt();
      sock_.close();
      client_.set_connected(false);
      started_ = false;
      return;
   }
   write();
}
void ConnectHandler::connect(end_point ep) {
   if (started_)
      return;
   ep_ = ep;
   msg_ = "connect";
   sock_.async_connect(ep,
            boost::bind(&ConnectHandler::on_connect,
                     shared_from_this(),
                     error));
}
void ConnectHandler::write() {
   sock_.async_write_some(
      boost::asio::buffer(msg_, max_length),
      boost::bind(&ConnectHandler::handle_write,
                  shared_from_this(),
                  error,
                  bytes_transferred));
}

void ConnectHandler::read() {
   sock_.async_read_some(
      boost::asio::buffer(data_, max_length),
      boost::bind(&ConnectHandler::handle_read,
                  shared_from_this(),
                  error,
                  bytes_transferred));
}

void ConnectHandler::handle_read(const err_code &err, size_t bytes_transferred) {
   if (err) {
      cerr << "\rRead error: " << err.message() << endl << client_.print_prompt();
      sock_.close();
      client_.set_connected(false);
      return;
   }
   received_ = string(data_, bytes_transferred);
   if (!received_.empty() && !client_.is_connected()){
      client_.set_connected(true);
      cout << "\rSuccessfully connected!\n" << client_.print_prompt();
   }
   if(!received_.empty()) {
      postpone_ping();
      cout << "\rPing    " << received_ << "\n" << client_.print_prompt();
      received_.clear();
   }
   sock_.close();
   started_ = false;
}

void ConnectHandler::handle_write(const err_code &err, size_t bytes_transferred) {
   if (err) {
      cerr << "\rWrite error: " << err.message() << endl << client_.print_prompt();
      sock_.close();
      return;
   }
   read();
}

const string TimeStamp::getTimeStamp() {
   time_t now = time(0);
   struct tm tstruct;
   char buf[80];
   tstruct = *localtime(&now);
   strftime(buf, sizeof(buf), "[%Y-%m-%d.%X]", &tstruct);
   return buf;
}

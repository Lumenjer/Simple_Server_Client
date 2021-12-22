#pragma once
#if !defined(_CLIUI_HPP_)
#define _CLIUI_HPP_

#include <iostream>
#include <cli/boostasioscheduler.h>
#include <cli/cli.h>
#include <cli/clilocalsession.h>

#if BOOST_VERSION < 106600
using IoContext = boost::asio::io_service;
#else
using IoContext = boost::asio::io_context;
#endif
class Server;

class ServerUI {
public:
   ServerUI(IoContext &iocontext, Server &server);
   void Prompt(){
      localSession->Prompt();
   }
   void Prompt(std::string msg){
      localSession->OutStream() << msg;
   }
private:
   void printTwist(int qt = 20);
   boost::asio::deadline_timer timer;
   cli::BoostAsioScheduler scheduler;
   std::unique_ptr<cli::Cli> cli;
   std::unique_ptr<cli::CliLocalTerminalSession> localSession;
   cli::CmdHandler colorCmd;
   cli::CmdHandler nocolorCmd;
   Server &server;
};

#endif // _CLIUI_HPP_
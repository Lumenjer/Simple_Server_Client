#pragma once
#if !defined(_CLIUI_HPP_)
#define _CLIUI_HPP_

#include <cli/boostasioscheduler.h>
#include <cli/cli.h>
#include <cli/clilocalsession.h>

using IoContext = boost::asio::io_context;

class Server;

class ServerUI {
public:
   ServerUI(Server &server);
   void Prompt(){ localSession->Prompt(); }
private:
   void printTwist(int qt = 20);
   Server &server;
   boost::asio::deadline_timer timer_;
   cli::BoostAsioScheduler scheduler;
   std::unique_ptr<cli::Cli> cli;
   std::unique_ptr<cli::CliLocalTerminalSession> localSession;
   cli::CmdHandler colorCmd;
   cli::CmdHandler nocolorCmd;
};

#endif // _CLIUI_HPP_
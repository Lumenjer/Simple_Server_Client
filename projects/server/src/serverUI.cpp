#include "serverUI.hpp"
#include "server.hpp"

using namespace cli;
using namespace std;

ServerUI::ServerUI(Server &server)
   : server(server), scheduler(server.getIOcontext()), timer_(server.getIOcontext()) {
   auto rootMenu = make_unique<Menu>("server");
   rootMenu->Insert(
      "start",
      [&](ostream &out) {
         server.start();
      },
      "Start server");
   rootMenu->Insert(
      "setPort", {"unsigned int"},
      [&](ostream &out, unsigned int port) {
         server.set_port(port);
      },
      "Set port for server");
   rootMenu->Insert(
      "status",
      [&](ostream &out) {
         out << "Server: " << (server.is_started() ? "active\n" : "down\n")
               << "Address: " << server.get_address().c_str() << endl
               << "Port: " << server.get_port() << endl;
      },
      "Show server status");
   rootMenu->Insert(
      "stop",
      [&](ostream &out) {
         server.stop();
      },
      "Stop server");
   rootMenu->Insert(
      "tryMe",
      [&](ostream &out) {
         printTwist();
      },
      "Fun;)");
   colorCmd = rootMenu->Insert(
      "color",
      [&](ostream &out) {
         out << "Colors ON\n";
         SetColor();
         colorCmd.Disable();
         nocolorCmd.Enable();
      },
      "Enable colors in the cli");
   nocolorCmd = rootMenu->Insert(
      "nocolor",
      [&](ostream &out) {
         out << "Colors OFF\n";
         SetNoColor();
         colorCmd.Enable();
         nocolorCmd.Disable();
      },
      "Disable colors in the cli");

   cli = make_unique<Cli>(move(rootMenu));
   // global exit action
   cli->ExitAction([](auto &out)
                  { out << "Goodbye and thanks for all the fish.\n"; });
   // std exception custom handler
   cli->StdExceptionHandler(
      [](ostream &out, const string &cmd, const exception &e) {
         out << "Exception caught in cli handler: "
               << e.what()
               << " handling command: "
               << cmd
               << ".\n";
      });

   localSession = make_unique<CliLocalTerminalSession>(*cli, scheduler, cout, 200);
   localSession->ExitAction(
      [this](auto &out) {
         out << "Closing App...\n";
         scheduler.Stop();
      });
}

void ServerUI::printTwist(int qt) {
   timer_.expires_from_now(boost::posix_time::millisec(100));
   timer_.async_wait([qt, this](const boost::system::error_code &) {
               int lQt = qt;
               static uint8_t stage = 0;
               if (qt == 20)
                  cout << "\n";
               cout << "\r";
               if (qt < 20 && qt >= 10) {
                  for (int i = 0; i < 20-qt; i++)
                     cout << " ";
               }
               else if (qt < 10) {
                  for (int i = qt; i > 0; i--)
                     cout << " ";
               }
               cout << (stage == 3 ? "/ " : stage == 2 ? "- " : stage == 1 ? "\\ " : "| ");
               ++stage &= 3;
               if (qt == 0) {
                  cout << "\rFun!\n";
                  this->localSession->Prompt();
                  return;
               }
               return printTwist(--lQt); 
         }
      );
}
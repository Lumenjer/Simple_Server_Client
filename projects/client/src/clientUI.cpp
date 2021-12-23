#include "clientUI.hpp"
#include "client.hpp"

using namespace cli;
using namespace std;

// ClientUI::ClientUI(IoContext &iocontext, Client &client)
ClientUI::ClientUI(Client &client)
   : client_(client), scheduler(client.getIOcontext()), timer_(client.getIOcontext()) {
   auto rootMenu = make_unique<Menu>("client");
   rootMenu->Insert(
      "send", {"string"},
      [&](ostream &out, std::vector<std::string> data) {
         client.send_msg(data);
      },
      "Send message");
   rootMenu->Insert(
      "connect", {"string"},
      [&](ostream &out, string str) {
         client.connect(str);
      },
      "Connect via new address and port");
   rootMenu->Insert(
      "connect",
      [&](ostream &out) {
         client.connect();
      },
      "Connect via setted address and port");
   rootMenu->Insert(
      "status",
      [&](ostream &out) {
         out << "Server: " << (client.is_connected() ? "connected\n" : "disconnected\n")
             << "Address: " << client.get_address().c_str() << endl
             << "Port: " << client.get_port() << endl;
      },
      "Show connection status");
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
         // client_.stop(); TODO:
         out << "Closing App...\n";
         scheduler.Stop();
      });
}

void ClientUI::printTwist(int qt) {
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
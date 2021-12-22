#include <iostream>
#include "client.hpp"
#include "clientUI.hpp"
using namespace std;

int main(int argc, char *argv[]) {
   try {
      cout << "Hello on Lumenjer's client! You can try help to see available commands, good luck!\n";
      IoContext iocontext;
      Client client(iocontext, "127.0.0.1", 8000);
      ClientUI interface(iocontext, client);

#if BOOST_VERSION < 106600
      boost::asio::io_service::work work(iocontext);
#else
      auto work = boost::asio::make_work_guard(iocontext);
#endif
      iocontext.run();
      return 0;
   }
   catch (const std::exception& e) {
      cerr << "Exception caugth in main: " << e.what() << endl;
   }
   catch (...) {
      cerr << "Unknown exception caugth in main." << endl;
   }
   return -1;
}
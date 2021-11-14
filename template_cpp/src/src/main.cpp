#include <chrono>
#include <iostream>
#include <thread>

#include "parser.hpp"
#include "hello.h"
#include <signal.h>
#include "process_controller.hpp"
#include <assert.h>

ProcessController * PROCESS_CONTROLLER;


static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  std::cout << "Terminate command received. Closing files\n";
  std::cout.flush();

  PROCESS_CONTROLLER -> stopProcess();

  // exit directly from signal handler
  exit(0);
}


int main(int argc, char **argv) {
  // specify that function stop handles signals SIGTERM, SIGINT
  signal(SIGTERM, stop);
  signal(SIGINT, stop);

  // `true` means that a config file is required.
  // Call with `false` if no config file is necessary.
  bool requireConfig = true;

  Parser parser(argc, argv);
  parser.parse();

  hello();
  std::cout << std::endl;

  std::cout << "My PID: " << getpid() << "\n";
  std::cout << "From a new terminal type `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
            << getpid() << "` to stop processing packets\n\n";

  std::cout << "My ID: " << parser.id() << "\n\n";

  std::cout << "List of resolved hosts is:\n";
  std::cout << "==========================\n";
  auto hosts = parser.hosts();
  std::cout << "\n";

  std::cout << "Path to output:\n";
  std::cout << "===============\n";
  std::cout << parser.outputPath() << "\n\n";

  std::cout << "Path to config:\n";
  std::cout << "===============\n";
  std::cout << parser.configPath() << "\n\n";

  std::cout << "Initializing Process Controller\n";
  PROCESS_CONTROLLER = new ProcessController(parser.id(), parser);
  std::cout << "Begin sending/receiving messages\n";
  PROCESS_CONTROLLER -> start();

  return 0;
}
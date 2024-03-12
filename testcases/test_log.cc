#include "netrpc/common/log.h"
#include "netrpc/common/config.h"
#include <pthread.h>
#include <thread>

void* fun(void*) {
  int i = 20;
  while (i --) {
    printf("1111111\n");
    DEBUGLOG("debug this is thread in %s", "fun");
    // INFOLOG("info this is thread in %s", "fun");
  }

  return NULL;
}


int main() {

  netrpc::Config::GetInst().Init(NULL);

  netrpc::Logger::GetInst().Init(0);


  // pthread_t thread;
  // pthread_create(&thread, NULL, &fun, NULL);
  std::thread t1(&fun, nullptr);


  int i = 20;
  while (i--) {
    DEBUGLOG("test debug log %s", "11");
    INFOLOG("test info log %s", "11");
  }

  // pthread_join(thread, NULL);
  t1.join();
  return 0;
}
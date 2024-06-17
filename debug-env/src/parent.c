#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdint.h>

#include "common_log.h"
#include "redis_app.h"

unsigned int NET_PORT = 1234;

// ———————————————————————— Declare the RB functions ———————————————————————— //

RB_DECLARE_FUNCS(char);


// The mempool for the child.
static const uint64_t MEMPOOL_ADDR = 0x700000;

// The app structures address
static const uint64_t APP_ADDR = 0x300000;

// The brk segment for the app.
static const uint64_t BRK_ADDR = 0xa20000;

void* create_shared(const char* name, void* addr, size_t size)
{
  int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
  ftruncate(shm_fd, size);
  void* ptr = mmap(addr, size, PROT_WRITE, MAP_SHARED, shm_fd, 0);
  memset(ptr, 0, size);
  return ptr;
}

void* run_stdin(void* arg) {
  redis_app_t* app = (redis_app_t*) arg;
  if (stdin_start_server(app) != SUCCESS) {
    ERROR("Error while executing the app");
    exit(-1);
  }
  return NULL;
}

void* run_tcp(void* arg) {
  redis_app_t* app = (redis_app_t*) arg;
  if (tcp_start_server(arg) != SUCCESS) {
    ERROR("Error while executing the app");
    exit(-1);
  }
  return NULL;
}

int main() {
    pthread_t server;
    redis_app_t* app = (redis_app_t*) create_shared("/app_shared", (void*)APP_ADDR, 0x2000); 
    rb_char_init(&(app->to_redis), MSG_BUFFER_SIZE, app->to_buffer);
    rb_char_init(&(app->from_redis), MSG_BUFFER_SIZE, app->from_buffer);
    void* pool = create_shared("/mempool_shared", (void*) MEMPOOL_ADDR, 0x640000);

    // Forking
    int pid = fork();
    // Execute the child process
    if (pid == 0) { 
      execl("../redis-server-dbg", "../redis-server-dbg", NULL);
    } else {
      int status;
      printf("The child was successfully created!\n");
#ifdef RUN_STDIN
      if (pthread_create(&server, NULL, run_stdin, (void*) app) < 0) {
#elif defined(RUN_TCP)
      if (pthread_create(&server, NULL, run_tcp, (void*) app) < 0) {
#else 
        if (1) {
#endif
        ERROR("Error starting server");
        exit(-1);
      }
      waitpid(pid, &status, 0);
      printf("The child exited.\n");
      pthread_join(server, NULL);
    }

    return 0;
}

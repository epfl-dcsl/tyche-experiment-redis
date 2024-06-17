#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common_log.h"
#include "redis_app.h"
#include "ringbuf_generic.h"

// —————————————————————————————— Local types ——————————————————————————————— //

typedef struct thread_arg_t {
  /// The shared info with the redis enclave.
  redis_app_t *comm;
} thread_arg_t;

// ——————————————————————————————— Functions ———————————————————————————————— //

void *run_output(void *arg) {
  thread_arg_t *thread_arg = (thread_arg_t *)arg;
  char buffer[MSG_BUFFER_SIZE] = {0};
  // Keep reading the channel.
  while (1) {
    int res = rb_char_read_n(&(thread_arg->comm->from_redis), MSG_BUFFER_SIZE,
                             buffer);
    if (res == FAILURE) {
      ERROR("Failure reading the channel.");
      goto failure;
    }
    // Print to the output.
    if (res > 0) {
      printf("stdout: ");
      fwrite(buffer, res, sizeof(char), stdout);
      printf("\n");
    }
  }
  return NULL;
failure:
  return NULL;
}

//TODO: maybe write a size or something.
static int process_input(char *input, redis_app_t *app) {
  // Send the message to the redis enclave.
  int to_write = strlen(input);
  int written = 0;
  printf("Sending `%s` to redis....", input);
  while(written < to_write) {
    int res = rb_char_write_n(&(app->to_redis), to_write - written, &input[written]);
    if (res == FAILURE) {
      // This should not happen with the stdin_server.
      ERROR("Failed to write to the channel");
      goto failure;
    }
    written += res;
  }
  printf("DONE!\n");
  return SUCCESS;
failure:
  return FAILURE;
}

int stdin_start_server(redis_app_t *comm) {
  // Untrusted thread for prints.
  pthread_t out_thread;
  char input[NET_BUFFER_SIZE];
  // Arguments for the output thread.
  thread_arg_t output_arg = {comm};

  // Create the thread for the output.
  if (pthread_create(&out_thread, NULL, run_output, (void *)&output_arg) < 0) {
    ERROR("Failed to create the output thread");
    goto failure;
  }
  while (1) {
    size_t len = 0;
    if (fgets(input, sizeof(input), stdin) == NULL) {
      // Handle error or end of file
      ERROR("Error reading input or end of file\n");
      goto failure;
    }

    // Formatting for redis.
    int backslash = strcspn(input, "\n");
    input[backslash] = '\r';
    input[backslash+1] = '\n';
    input[backslash+2] = '\0';
    //input[strcspn(input, "\n")] = '\0';


    // Check for exit condition
    if (strcmp(input, "exit") == 0) {
      printf("Exiting...\n");
      break;
    }
    // Process the input
    if (process_input(input, comm) != SUCCESS) {
      ERROR("Failure to process the input.");
      goto failure;
    }
  }

  /// Join on the out_thread.
  /// TODO: figure out how to signal that.
  pthread_join(out_thread, NULL);

  return SUCCESS;
failure:
  return FAILURE;
}

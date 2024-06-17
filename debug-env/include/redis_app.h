#pragma once

#include "common.h"
#include "ringbuf_generic.h"

// ——————————————————— Type for the shared memory region ———————————————————— //

RB_DECLARE_TYPE(char);
RB_DECLARE_PROTOS(char);

#define MSG_BUFFER_SIZE 1048

/// The redis enclave shared memory gets typecasted to this.
typedef struct redis_app_t {
  // Sending things to redis.
  rb_char_t to_redis;
  // Receiving messages from redis.
  rb_char_t from_redis;
  // Buffer for the to_redis.
  char to_buffer[MSG_BUFFER_SIZE];
  // Buffer for the from_redis.
  char from_buffer[MSG_BUFFER_SIZE];
} redis_app_t;

// —————————————————————————— Server configuration —————————————————————————— //
/// Port for the server.
extern unsigned int NET_PORT;
/// Size of tcp buffer.
#define NET_BUFFER_SIZE 1048

// ——————————————————————————————— Functions ———————————————————————————————— //

int stdin_start_server(redis_app_t* comm);
int tcp_start_server(redis_app_t* comm);

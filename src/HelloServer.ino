#include "comm.h"

void setup() {
  Serial.begin(115200);
}

void loop() {
  Comm* comm_instance = Comm::get_instance();
  if (!(comm_instance->is_connected())) {
    comm_instance->try_connect();
  }
  // save the password and the network name
  // connect to the network
}
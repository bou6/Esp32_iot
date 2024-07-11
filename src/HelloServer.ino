#include "comm.h"

void setup() {
  Serial.begin(115200);
}

void loop() {
  Comm* comm_instance = Comm::get_instance();
  if (comm_instance->get_state()!=Comm::CONNECTED) {
    comm_instance->comm_state_machine();
  }
  // save the password and the network name
  // connect to the network
}
#include "comm.h"
#include "mqtt_handler.h"


void setup() {
  Serial.begin(115200);
  
  //### ToDo: Initialize create a class for Hw initialization
  // Set up the pump pin
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  Comm* comm_instance = Comm::get_instance();
  Mqtt_handler* mqtt_instance = Mqtt_handler::get_instance();
  
  comm_instance->comm_state_machine();

  if (comm_instance->get_state()==Comm::CONNECTED) {
    mqtt_instance->mqtt_handler_state_machine();
  }
  // connect to the network
}
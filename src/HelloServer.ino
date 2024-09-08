#include "comm.h"
#include "mqtt_handler.h"
#include "config.h"
void setup() {
  Serial.begin(115200);

  pinMode(WATER_CTRL_PIN, OUTPUT);

  // inititialize the pin to low
  digitalWrite(WATER_CTRL_PIN, LOW);
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
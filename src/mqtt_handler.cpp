#include "mqtt_handler.h"
#include "config.h"
#include <ESP8266WiFi.h>
#include <time.h>
#include <TZ.h>
#include <FS.h>
#include <LittleFS.h>

Mqtt_handler* Mqtt_handler::m_instance = nullptr;

const char* mqtt_server = "294fd52b749e4ed494a3c125261d9f0e.s1.eu.hivemq.cloud";
const char* mqtt_username = "achraf";
const char* mqtt_password = "Mqtt26081991";
const int mqtt_port =8883;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
#define COMMANDS_TOPIC "commands"
#define STATUS_TOPIC "status"
char msg[MSG_BUFFER_SIZE];
int value = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

   // Allocate memory for the payload plus null terminator
  char *data = (char *)malloc(length + 1);
  if (data == NULL) {
      fprintf(stderr, "Failed to allocate memory.\n");
      return;
  }
  // Copy the payload to the new buffer
  memcpy(data, payload, length);

  // Add the null terminator
  data[length] = '\0';

  Serial.println(data);

  if (0==strcmp(topic,COMMANDS_TOPIC))
  {

    if (0==strcmp(data,"water on"))
    {
      digitalWrite(WATER_CTRL_PIN, HIGH);
    }
    else if (0==strcmp(data,"water off"))
    {
      digitalWrite(WATER_CTRL_PIN, LOW);
    }
    else
    {
      Serial.println("Unknown command");
    }
  }
  else
  {
    Serial.println("Unknown topic");
  }
}

void setDateTime() {
  // You can use your own timezone, but the exact time is not used at all.
  // Only the date is needed for validating the certificates.
  configTime(TZ_Europe_Berlin, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(100);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
}

Mqtt_handler::Mqtt_handler()
{
    m_state = SETUP;
    bear = new BearSSL::WiFiClientSecure();
}

Mqtt_handler* Mqtt_handler::get_instance()
{
    if (m_instance == nullptr)
    {
        m_instance = new Mqtt_handler();
    }
    return m_instance;
}

String Mqtt_handler::mqtt_state_to_string(Mqtt_state state)
{
    switch (state)
    {
        case Mqtt_state::SETUP:
            return "SETUP";
        case Mqtt_state::CONNECTED:
            return "CONNECTED";
    }
    return "UNKNOWN";
}

void Mqtt_handler::mqtt_handler_state_transition(Mqtt_state new_state)
{
    m_state = new_state;
    Serial.println("Transition from " + mqtt_state_to_string(m_state) + " to " + mqtt_state_to_string(new_state));
}

void Mqtt_handler::reconnect() {

  //###ToDo handle the case when the connection is lost
  // Loop until we’re reconnected
  while (!client->connected()) {
    Serial.print("Attempting MQTT connection…");
    String clientId = "ESP8266Client - MyClient";
    // Insert your password
    if (client->connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement…
      client->publish(STATUS_TOPIC, "Reconnect !!!");
      // … and resubscribe
      client->subscribe(COMMANDS_TOPIC);
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client->state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void Mqtt_handler::mqtt_handler_state_machine()
{
  int numCerts;
  unsigned long now = 0;
  switch (m_state)
  {
    case SETUP:
        setDateTime();
        numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
        Serial.printf("Number of CA certs read: %d\n", numCerts);
        if (numCerts == 0) {
          Serial.printf("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
          break;
        }
        // Integrate the cert store with this connection
        bear->setCertStore(&certStore);
        client = new PubSubClient(*bear);
        client->setServer(mqtt_server, mqtt_port);
        client->setCallback(callback);
        mqtt_handler_state_transition(CONNECTED);
        break;
    
    case CONNECTED:
        if (!client->connected()) {
            reconnect();
        }
        client->loop();
        now = millis();
        if (now - lastMsg > 2000) {
          lastMsg = now;
          ++value;
          snprintf (msg, MSG_BUFFER_SIZE, "connected #%ld", value);
          Serial.print("Publish message: ");
          Serial.println(msg);
          client->publish(STATUS_TOPIC, msg);
        }
        break;
      
      default:
        break;   
    }   
}
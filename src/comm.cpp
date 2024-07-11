#include "comm.h"
#include <ESP8266WiFi.h>
#include <LittleFS.h>

//#### to do if the connection to the wifi is succesfull then kill the server
//#### store the user Id and password in the flash memory if first connection is Ok
//#### handle the case when the user is not connected to the wifi

// SSID and password for the ESP32 Access Point
const char* ap_ssid = "ESP32-AP";
const char* ap_password = "12345678";
Comm* Comm::instance = nullptr;

Comm::Comm() {
  // Set the initial state
  m_state = INIT;

  //Start LittleFS
  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    delay(1000);
    return;
  }
  #if 0
  String network, password; 
  if (!read_saved_credentials(&network, &password))
  {
    Serial.println("No saved credentials found, Access point will be started");
    ESP8266WebServer server(80);
    
    // Set up the Access Point
    WiFi.softAP(ap_ssid, ap_password);
    Serial.println();
    Serial.print("Access Point \"");
    Serial.print(ap_ssid);
    Serial.println("\" started");
    Serial.print("IP address:\t");
    Serial.println(WiFi.softAPIP());

    // Define the server routes
    instance->server.on("/", HTTP_GET, handleRoot);
    instance->server.on("/connect", HTTP_POST, handleConnect);

    // Start the server
    instance->server.begin();
    Serial.println("Server started");
  }
  else
  {
    Serial.println("Saved credentials found, connecting to the network");
    WiFi.begin(network.c_str(), password.c_str());
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(500);
      Serial.print(".");
    }
    // if the connection fail
  }
  #endif
}

// Function to handle the connection attempt
void handleConnect() {
  Comm* comm = Comm::get_instance();

  String network = comm->server.arg("network");
  String password = comm->server.arg("password");
  
  // Attempt to connect to the selected network
  WiFi.begin(network.c_str(), password.c_str());
  
  String response = "<html><body><h1>Connecting to ";
  response += network;
  response += "...</h1><p>";
  
  // Wait for connection (10 seconds timeout)
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    response += "Connected! IP address: ";
    response += WiFi.localIP().toString();
    // set the network and the password
    comm->set_network(network);
    comm->set_password(password);
  } else {
    response += "Failed to connect.";
  }
  
  response += "</p><a href='/'>Back</a></body></html>";
  comm->server.send(200, "text/html", response);
}

// Function to handle the root path "/"
void handleRoot() {
  Comm* instance = Comm::get_instance();
  String html = "<html><body><h1>Select a Network</h1>";
  html += "<form action='/connect' method='POST'>";
  html += "<label for='network'>Choose a network:</label><br><select name='network'>";
  
  int numNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numNetworks; ++i) {
    html += "<option value='" + String(WiFi.SSID(i)) + "'>" + String(WiFi.SSID(i)) + " (" + String(WiFi.RSSI(i)) + ")</option>";
  }
  
  html += "</select><br><br>";
  html += "<label for='password'>Password:</label><br><input type='password' name='password'><br><br>";
  html += "<input type='submit' value='Connect'>";
  html += "</form></body></html>";
  
  instance->server.send(200, "text/html", html);
}

void Comm::set_network(String network)
{
  m_network = network;
}

void Comm::set_password(String password)
{
  m_password = password;
}

Comm* Comm::get_instance() {
  if (instance == nullptr)
    instance = new Comm();
  return instance;
}

bool Comm::is_connected() {
  return WiFi.status() == WL_CONNECTED;
}

String Comm::comm_state_to_string(Comm::State state)
{
  switch (state)
  {
    case Comm::INIT:
      return "INIT";
    case Comm::AP_STARTED:
      return "AP_STARTED";
    case Comm::CONNECTING:
      return "CONNECTING";
    case Comm::CONNECTED:
      return "CONNECTED";   
  }
  return "UNKNOWN";
}

void Comm::comm_state_transition(State new_state)
{
  Serial.println("Transition from " + comm_state_to_string(m_state) + " to " + comm_state_to_string(new_state));
  m_state = new_state;
}

void Comm::comm_state_machine()
{
  unsigned long startTime;
  
  switch (m_state)
  {
    case INIT:
      if (read_saved_credentials(&m_network, &m_password))
      {
        // try to connect to the network
        WiFi.begin(m_network.c_str(), m_password.c_str());
        comm_state_transition(CONNECTING);
      }
      else
      {
        // Set up the Access Point
        WiFi.softAP(ap_ssid, ap_password);
        Serial.println();
        Serial.print("Access Point \"");
        Serial.print(ap_ssid);
        Serial.println("\" started");
        Serial.print("IP address:\t");
        Serial.println(WiFi.softAPIP());
    
        // Define the server routes
        instance->server.on("/", HTTP_GET, handleRoot);
        instance->server.on("/connect", HTTP_POST, handleConnect);
    
        // Start the server
        instance->server.begin();
        Serial.println("Server started");

        // Transition to the AP_STARTED state
        comm_state_transition(AP_STARTED);
      }
      break;
    
    case AP_STARTED:
      server.handleClient();
      if (WiFi.status() == WL_CONNECTED)
      {
        // store the password and the network name
        save_credentials(m_network, m_password);

        // Stop the server
        server.stop();
        comm_state_transition(CONNECTED);
      }
    break;
    
    case CONNECTING:
      // Wait for connection (10 seconds timeout)
      startTime = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) 
      {
        delay(500);
        Serial.print(".");
      }
      if (WiFi.status() == WL_CONNECTED)
      {
        // store the password and the network name
        save_credentials(m_network, m_password);
        comm_state_transition(CONNECTED);
      }
      else if (WiFi.status() == WL_DISCONNECTED)
      {
        //delete the saved credentials
        delete_credentials();
        comm_state_transition(INIT);
      }
      break;
    
    case CONNECTED:
    break;
  }
}

Comm::State Comm::get_state()
{
  return m_state;
}

bool Comm::read_saved_credentials(String* network, String* password)
{
  //Open the file
  File file = LittleFS.open("/credentials.txt", "r");
  if(!file){
    Serial.println("Error opening file");
    return false;
  }
  //Read the network and the password
  *network = file.readStringUntil('\n');
  *password = file.readStringUntil('\n');
  //Close the file
  file.close();
  return true;

}

bool Comm::save_credentials(String network, String password)
{
   //Open the file 
  File file = LittleFS.open("/credentials.txt", "w");
  //save the network and the password
  file.print(network);
  file.print("\n");
  file.print(password);
  file.print("\n");
  //Close the file
  file.close();
  // check if the file is saved
  if(!file){
    Serial.println("Error saving file");
    return false;
  }
  Serial.println("Write successful");
  return true;
}

bool Comm::delete_credentials()
{
  //Open the file
  File file = LittleFS.open("/credentials.txt", "r");
  if(!file){
    Serial.println("Error opening file");
    return false;
  }
  //Delete the file
  LittleFS.remove("/credentials.txt");
  //Close the file
  file.close();
  return true;
}

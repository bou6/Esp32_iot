#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// SSID and password for the ESP32 Access Point
const char* ap_ssid = "ESP32-AP";
const char* ap_password = "12345678";

// Create a web server on port 80
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  
  // Set up the Access Point
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println();
  Serial.print("Access Point \"");
  Serial.print(ap_ssid);
  Serial.println("\" started");
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
  
  // Define the server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/connect", HTTP_POST, handleConnect);

  // Start the server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}

// Function to handle the root path "/"
void handleRoot() {
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
  
  server.send(200, "text/html", html);
}

// Function to handle the connection attempt
void handleConnect() {
  String network = server.arg("network");
  String password = server.arg("password");
  
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
  } else {
    response += "Failed to connect.";
  }
  
  response += "</p><a href='/'>Back</a></body></html>";
  server.send(200, "text/html", response);
}
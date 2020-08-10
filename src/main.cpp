#include <Arduino.h>
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "WirelessBelgie-Internet";
const char* password = NULL;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
const int output5 = 5;
const int output4 = 4;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /turn/left") >= 0) {
              Serial.println("left");
            } else if (header.indexOf("GET /turn/right") >= 0) {
              Serial.println("right");
            } else if (header.indexOf("GET /turn/stop") >= 0) {
              Serial.println("stop");
            } else if (header.indexOf("GET /turn?") >= 0) {
              int elIndex = header.indexOf("el=")+3;
              Serial.println(header.substring(elIndex,elIndex+4));
              int azIndex = header.indexOf("az=")+3;
              Serial.println(header.substring(azIndex,azIndex+4));
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 15px 35px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Rotor controller</h1>");
            client.println("<p>");
            client.println("<a href=\"/turn/left\"><button class=\"button\">left</button></a>");
            client.println("<a href=\"/turn/stop\"><button class=\"button\">stop</button></a>");
            client.println("<a href=\"/turn/right\"><button class=\"button\">right</button></a>");
            client.println("</p>");

            client.println("<p>input in 10 of a degree, always 4 digits (prepend with 0 if needed)<br /><table><tr><td><form action='/turn' method='get'>");
            client.println("el:<input type='text' name='el' value=''><br />");
            client.println("az:<input type='text' name='az' value=''><br />");
            client.println("</td><td><input type='submit' class='button' value='turn'></td></tr></table></form>");
            client.println("</p>");

            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }
}
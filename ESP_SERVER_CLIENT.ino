/***********************EEPROM****************/
#include <EEPROM.h>

/***********************SERWER WIFI****************/
#include <WiFi.h>
#include <WebServer.h>
#include <ESPAsyncWebServer.h>
AsyncWebServer server1(80);
#define eeprom_reset_switch 15


/***********************CLIENT WIFI****************/
#include <HTTPClient.h>

/* Put your SSID & Password */
const char* ssid = "ESP32";         // Enter SSID here
const char* password = "12345678";  //Enter Password here
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="pl">
<head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
  <body>
    <form action="/get">
      WIFI SSID: <input type="text" name="input1"><br>
      WIFI PASS: <input type="text" name="input2"><br>
      DEVICE ID: <input type="text" name="input3"><br>
      USER ID: <input type="text" name="input4"><br>
      <input type="submit" value="Submit">
    </form><br>
  </body>
</html>
)rawliteral";

void (*resetFunc)(void) = 0;


void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(eeprom_reset_switch, INPUT_PULLUP);

  if (EEPROM.readString(1) != "" && EEPROM.readString(30) != "" && EEPROM.readString(90) != "") {
    /***********************CLIENT WIFI***************/
    String ssid_read_1 = EEPROM.readString(1);
    String pass_read_1 = EEPROM.readString(30);
    int str_1_len = ssid_read_1.length() + 1;
    int str_2_len = pass_read_1.length() + 1;
    char ssid_eeprom_read[str_1_len];
    char pass_eeprom_read[str_2_len];
    ssid_read_1.toCharArray(ssid_eeprom_read, str_1_len);
    pass_read_1.toCharArray(pass_eeprom_read, str_2_len);

    Serial.print("SSID:");
    Serial.print(ssid_eeprom_read);
    Serial.print("_PASSWORD:");
    Serial.print(pass_eeprom_read);
    Serial.print("_Device ID:");
    Serial.println(EEPROM.readString(60));
    Serial.print("_User ID:");
    Serial.println(EEPROM.readString(90));
    
    Serial.println("HTTP client started");

    const char* ssid_client = ssid_eeprom_read;
    const char* password_client = pass_eeprom_read;
    WiFi.begin(ssid_client, password_client);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.print("IP:");
    Serial.println(WiFi.localIP());
    Serial.print("Connected with WIFI:");
    Serial.println(ssid_eeprom_read);
    /***********************END CLIENT WIFI***************/
  } else {
    Serial.println("No WIFI data in EEPROM");

    /***********************SERWER WIFI****************/
    //Turning on wifi server
    //define wifiserver
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    delay(100);


    // Send web page with input fields to client
    server1.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      request->send_P(200, "text/html", index_html);
    });
    // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
    server1.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
      String inputMessage1;
      String inputMessage2;
      String inputMessage3;
      String inputMessage4;
      String inputParam1;
      String inputParam2;
      String inputParam3;
      String inputParam4;
      // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
      if (request->hasParam("input1")) {
        inputMessage1 = request->getParam("input1")->value();
        inputMessage2 = request->getParam("input2")->value();
        inputMessage3 = request->getParam("input3")->value();
        inputMessage4 = request->getParam("input4")->value();
        inputParam1 = "input1";
        inputParam2 = "input2";
        inputParam3 = "input3";
        inputParam3 = "input4";
        EEPROM.writeString(1, inputMessage1);
        EEPROM.writeString(30, inputMessage2);
        EEPROM.writeString(60, inputMessage3);
        EEPROM.writeString(90, inputMessage4);
        EEPROM.commit();
        resetFunc();
      }
      // // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
      // else if (request->hasParam("input2")) {
      //   inputMessage = request->getParam("input2")->value();
      //   inputParam = "input2";
      // }
      else {
        inputMessage1 = "No message sent";
        inputParam1 = "none";
      }


      request->send(200, "text/html", index_html);
      // request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("
      //                                  + inputParam + ") with value: " + inputMessage +
      //                                  "<br><a href=\"/\">Return to Home Page</a>");
    });
    server1.begin();  //asynchwebserver
    server.begin();
    Serial.println("HTTP server started");
  /***********************END SERWER WIFI****************/
  }

}
void loop() {
  resetEeprom();  //clear wifi data
}

void resetEeprom(){
  static int resetTime = 0;
  resetTime++;
  if(digitalRead(eeprom_reset_switch)==LOW && resetTime>=10000){
        EEPROM.writeString(1, "");
        EEPROM.writeString(30, "");
        EEPROM.writeString(60, "");
        EEPROM.writeString(90, "");
        EEPROM.commit();
        Serial.println("EEPROM RESET");
        resetTime=0;
        resetFunc();
  }
}
//  Import required libraries
#include <Arduino.h>
// ArduinoJson - Version: 6.13.0
#include <ArduinoJson.h>

#include <ArduinoOTA.h>
// #include <ArduinoJson.hpp>
#define tests
#define reals

// Variables to store the extracted values
String phVal;
float ph1;
long ph2;
long water;
long water2, timer1, timer2, timer3, pump1, pump2, pump3 = 0;
// Define variables:
long duration;
float distance;
String inputString = "phVal,6.851097,ph1,1234,ph2,244,water,344";
// Static IP address configuration
IPAddress staticIP(192, 168, 1, 100); // Desired static IP address
IPAddress gateway(192, 168, 1, 1);    // Your network gateway
IPAddress subnet(255, 255, 255, 0);   // Subnet mask
IPAddress dns(192, 168, 1, 1);        // DNS server (usually the same as gateway)

#if defined(ESP32)
#include <WiFi.h>
#include <AsyncTCP.h>
#include <WebServer.h> //#include <WebServer.h>
#include <Hash.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
WebServer server(80);
int workingLed = 4;
int wifiLed = 15;
int dbLed = 14;
int relay1 = 2;
int relay2 = 4;
int relay3 = 19;
int relay4 = 18;
int MUXPinS0 = 7;
int MUXPinS1 = 6;
int MUXPinS2 = 5;
int MUXPinS3 = 4;
// Define Trig and Echo pin:
#define trigPin 14
#define echoPin 12

#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiClientSecure.h>
// #include "certs.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti WiFiMulti;
// X509List cert(cert_DigiCert_Global_Root_CA);
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Updater.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
ESP8266WebServer server(80);
int workingLed = D7;
int wifiLed = D7;
int dbLed = D7;
int MUXPinS0 = D7;
int MUXPinS1 = D7;
int MUXPinS2 = D7;
int MUXPinS3 = D7;

int relay1 = D3; // relay1,int 1
int relay2 = D1; // relay2, int 3
int relay3 = D0; // pump4
int relay4 = D2; // ph1 relay, int 1
                 // Define Trig and Echo pin:
const char *fingerprint = "0A:C5:5E:61:CD:83:C4:B1:12:16:5D:61:41:6D:C9:C8:CA:7A:F9:D8";

#define trigPin D4
#define echoPin D5
// wire (multiplexor)EN to (ESP)GND, SIG to A0, VCC to 3v3 and GND to GND
#endif
#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF
#include <WiFiManager.h>
// #include <Adafruit_Sensor.h>
// #include <DHT.h>
unsigned long relayStartTimes[3] = {0}; // Array to store the start times of relay operations
bool relayActive[3] = {false};          // Flags to indicate if relays are currently active
String json;
int phval = 0;
float values[5];
int phRaw = values[0];
int analog2 = values[1];
int maxRead = 44;
int minRead = 22;
int waterLevel = 100;
int waterRead = 10;
float tdsValue = 0;
float ppmValue = 0;
int pump, height = 0;
// Conversion factor to calculate ppm value
float ppmConversionFactor = 0.5;
const char *host = "esp32";
#include "secrets.h"
// #include <ArduinoJson.h>
//  Variable as required in source
bool signupOK = false;
int buffer_arr[20], temp = 0;
float calibration_value = 21.34 - 0.7;

unsigned long int avgval;
String macAdd = "";

int bootCount = 0;
char name[15] = CLIENT;
// int LED_BUILTIN = 4;
StaticJsonDocument<5000> doc;

#define DHTPIN 2 // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
bool toggle = HIGH;
// DHT object
// DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;
char apNames[30];
// Updates DHT readings every 20 seconds
const long interval = 8000;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time DHT was updated
#define OTA_HOSTNAME ""           // Leave empty for esp8266-[ChipID]

#define WIFI_MANAGER_STATION_NAME "" // Leave e mpty for auto generated name ESP + ChipID
#define TdsSensorPin A0
#define VREF 5.0  // analog reference voltage(Volt) of the ADC
#define SCOUNT 30 // sum of sample point

int analogBuffer[SCOUNT]; // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;

float averageVoltage = 0;
float temperature = 16; // current temperature for compensation

// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
  {
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else
  {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}
void setup_wifi_manager()
{

  WiFiManager wifiManager;
  // wifiManager.resetSettings();
  if (WIFI_MANAGER_STATION_NAME == "")
  { // Configure the static IP address if WiFi is not connected
    // WiFi.config(staticIP, gateway, subnet, dns);

    // use this for auto generated name ESP + ChipID
    macAdd = WiFi.macAddress();
    // WIFI_MANAGER_STATION_NAME = macAdd;
    // char  * apNames = macAdd.c_str();
    // char apNames[30];
    String newMac = "8C:AA:B5:7F:62:2F";
    newMac.toCharArray(apNames, 30);
    // macAdd.toCharArray(apNames, 30);
    Serial.println(apNames);
    wifiManager.autoConnect(apNames);

    Serial.print("the mother ap name is ");
    Serial.println(apNames);
    // doc["device"]["APname"] = apNames;
  }

  else
  {
    // Configure the static IP address if WiFi is not connected
    //  WiFi.config(staticIP, gateway, subnet, dns);

    macAdd = WiFi.macAddress();
    // WIFI_MANAGER_STATION_NAME = macAdd;
    // char  * apNames = macAdd.c_str();
    // char apNames[30];
    String newMac = "8C:AA:B5:7F:62:2F"; // enter the mac address here
    newMac.toCharArray(apNames, 30);
    // macAdd.toCharArray(apNames, 30);
    Serial.println(apNames);
    wifiManager.autoConnect(apNames);

    Serial.print("the mother ap name is ");
    Serial.println(apNames);
  }
}

void processString(String input)
{
  char inputCharArray[input.length() + 1];
  input.toCharArray(inputCharArray, sizeof(inputCharArray));

  char *token;
  char *key;
  char *value;

  token = strtok(inputCharArray, ",");
  while (token != NULL)
  {
    yield();
    key = token;
    token = strtok(NULL, ",");
    if (token != NULL)
    {
      value = token;

      // Process the key-value pair and assign the value to the appropriate global variable
      if (strcmp(key, "phVal") == 0)
      {
        phVal = value;
        Serial.println(phVal);
      }
      else if (strcmp(key, "ph1") == 0)
      {
        ph1 = atof(value); // Convert to float
      }
      else if (strcmp(key, "ph2") == 0)
      {
        ph2 = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "water") == 0)
      {
        water = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "water2") == 0)
      {
        water2 = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "pump1") == 0)
      {
        pump1 = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "pump2") == 0)
      {
        pump2 = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "pump3") == 0)
      {
        pump3 = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "timer1") == 0)
      {
        timer1 = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "timer2") == 0)
      {
        timer2 = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "timer3") == 0)
      {
        timer3 = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "height") == 0)
      {
        height = atol(value); // Convert to long integer
      }
      else if (strcmp(key, "pump") == 0)
      {
        pump = atol(value); // Convert to long integer
      }
    }
    token = strtok(NULL, ",");
  }
  relayStartTimes[0] = timer1;
  relayStartTimes[1] = timer2;
  relayStartTimes[2] = timer3;
}
void checkPhRelays()
{ // iF WaterLevel falls below point A activate pump 3
  //   IF PHLevel falls below point A activate pump 1
  //  IF PHLevel rise above point B activate pump 2
  Serial.print("ph value: ");
  Serial.println(phVal.toFloat());
  if (pump1 < 7)
  {
    if (phVal.toFloat() < ph1)
    {

      digitalWrite(relay2, HIGH);
      Serial.println("ph value is low");
    }
    else
    {
      digitalWrite(relay2, LOW);
    }
  }
  if (pump2 < 7)
  {

    if (phVal.toFloat() > ph2)
    {
      digitalWrite(relay3, HIGH);
      Serial.println("ph value is high");
    }
    else
    {
      digitalWrite(relay3, LOW);
    }
  }
}
void checkAndControlRelays()
{
  unsigned long currentTime = millis();

  for (int i = 0; i < 3; i++)
  {
    if (i == 0 && pump1 > 5 && currentTime - relayStartTimes[i] >= timer1 * 1000)
    {
      // Serial.println("relay 1 deactivated at " + String(currentTime));
      //  relayActive[i] = false;
      digitalWrite(relay1, LOW); // Deactivate relay1
    }
    else if (i == 1 && pump2 > 5 && currentTime - relayStartTimes[i] >= timer2 * 1000)
    {
      // Serial.println("relay 2 deactivated at " + String(currentTime));
      // relayActive[i] = false;
      digitalWrite(relay2, LOW); // Deactivate relay2
    }
    else if (i == 2 && pump3 > 5 && currentTime - relayStartTimes[i] >= timer3 * 1000)
    {
      // Serial.println("relay 3 deactivated at " + String(currentTime));
      // relayActive[i] = false;
      digitalWrite(relay3, LOW); // Deactivate relay3
    }
  }

  if (pump1 > 5 && !relayActive[0])
  {
    // Serial.println("relay 1 activated at " + String(currentTime));
    relayActive[0] = true;
    relayStartTimes[0] = currentTime;
    digitalWrite(relay1, HIGH); // Activate relay1
  }
  else if (pump1 < 3)
  {
    relayActive[0] = false;
  }

  if (pump2 > 5 && !relayActive[1])
  {
    // Serial.println("relay 2 activated at " + String(currentTime));
    relayActive[1] = true;
    relayStartTimes[1] = currentTime;
    digitalWrite(relay2, HIGH); // Activate relay2
  }
  else if (pump2 < 3)
  {
    relayActive[1] = false;
  }

  if (pump3 > 5 && !relayActive[2])
  {
    // Serial.println("relay 3 activated at " + String(currentTime));
    relayActive[2] = true;
    relayStartTimes[2] = currentTime;
    digitalWrite(relay3, HIGH); // Activate relay3
  }
  else if (pump3 < 3)
  {
    relayActive[2] = false;
  }
}
void setUpOTA()
{
  ArduinoOTA.onStart([]()
                     {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type); });

  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });

  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    } });

  ArduinoOTA.begin();

  server.on("/", HTTP_GET, []()
            { server.send(200, "text/html", "<h1>OTA Update</h1><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>"); });

  server.on("/update", HTTP_POST, []()
            {
    server.send(200, "text/plain", (Update.hasError()) ? "Update Failed" : "Update Success! Rebooting...");
    ESP.restart(); }, []()
            {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      //WiFiUDP::stopAll();
      if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %u bytes\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield(); });

  server.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void POSTData()
{
  yield();
  Serial.println("posting data");
#if defined(ESP8266)

  // Configure BearSSL
  // BearSSL::WiFiClientSecure *client = new BearSSL::WiFiClientSecure;
  // CertStoreBearSSL certStore;
  WiFiClientSecure client;

  // client.setCACert(test_root_ca);
  client.setFingerprint(fingerprint);
  BearSSL::X509List trustAnchors((const uint8_t *)test_root_ca, strlen(test_root_ca));
  client.setTrustAnchors(&trustAnchors);

  // Ignore SSL certificate validation
  client.setInsecure();

  // HTTPClient object
  HTTPClient http;

  // Start HTTP client
  http.begin(client, serverName);
  Serial.println("HTTP client started!");

#elif defined(ESP32)
  WiFiClientSecure client;

  client.setCACert(test_root_ca);
  // client.setTrustAnchors(&cert);
  //  if (!client.connect("4D:A1:38:30:EF:83:AA:42:9D:28:C1:0A:0D:BC:C0:EF:BA:39:E3:BC")) {
  if (!client.connect(serverName, 443))
  {
    Serial.println("connected!");
    // return;
  }
  HTTPClient http;
  // if (http.begin(*client, serverName, 443))
  // { // HTTPS

  http.begin(client, serverName);
#endif

  http.addHeader("Content-Type", "application/json");

  serializeJson(doc, json);
  // ESP.wdtEnable(2000);
  // Serial.println(json);
  // http.setTimeout(5500);  // 10-second timeout
  yield();
  int httpResponseCode = http.POST(json);
  yield();
  Serial.print("response :");
  Serial.println(httpResponseCode);
  // delay(1000);

  String response = http.getString();
  Serial.println(response.substring(1, response.length() - 1));
  // phval = (response.substring(1, response.length() - 1).toInt());
  processString(response.substring(1, response.length() - 1));

  // Now you can use the extracted values in your further code logic
  Serial.print("phVal: ");
  Serial.println(phVal.toFloat());
  Serial.print("ph1: ");
  Serial.println(ph1);
  Serial.print("ph2: ");
  Serial.println(ph2);
  Serial.print("water: ");
  Serial.println(water);
  if (pump1 > 7)
  {
    digitalWrite(relay1, HIGH); // relay is to be on here
    Serial.println("pump 1 on ph value is higher");
  }
  else
  {
    digitalWrite(relay1, LOW); // relay off pump also off
  }

  if (pump2 > 7)
  {
    digitalWrite(relay2, HIGH);
    Serial.println("pump 2 on ph value is lower");
  }
  else
  {
    // digitalWrite(relay2, LOW);
  }
  if (pump3 > 7)
  {
    digitalWrite(relay3, HIGH);
    Serial.println("pump 3 on ph value is lower");
  }
  else
  {
    // digitalWrite(relay3, LOW);
  }
}

void getDevice()
{

  // esp_sleep_wakeup_cause_t wakeup_reason;
  String wakeup_reason = "1"; // esp_sleep_get_wakeup_cause();

  doc[apNames]["IP"] = String(WiFi.localIP().toString());
  // doc[apNames]["RSSI"] = String(WiFi.RSSI());
  // doc[apNames]["type"] = TYPE;
  // doc[apNames]["name"] = name;
  // doc[apNames]["chipid"] = buffer;
  //  doc[apNames]["bootCount"] = bootCount;
  // doc[apNames]["wakeup_reason"] = String(wakeup_reason);
  // doc[apNames]["vbatt_raw"] = vbatt_raw;
  // doc[apNames]["vbatt"] = map(vbatt_raw, 0, 4096, 0, 4200);
}

void controlLevel()
{
  /* iF WaterLevel falls below point A activate pump 3
      IF PHLevel falls below point A activate pump 1
      IF PHLevel rise above point B activate pump 2
    */
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U)
  { // every 40 milliseconds,read the analog value from the ADC
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); // read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
    {
      analogBufferIndex = 0;
    }
  }

  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
    {
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

      // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;

      // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
      // temperature compensation
      float compensationVoltage = averageVoltage / compensationCoefficient;

      // convert voltage value to tds value
      tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
    }
  }
  Serial.print("voltage:");
  Serial.print(analogRead(TdsSensorPin), 2);
  Serial.print("V   ");
  Serial.print("TDS Value:");
  Serial.print(tdsValue, 0);
  Serial.println("ppm");
}

float getUltra()
{
  // Clear the trigPin by setting it LOW:
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  // Trigger the sensor by setting the trigPin high for 10 microseconds:
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
  duration = pulseIn(echoPin, HIGH);
  // Calculate the distance:
  distance = duration * 0.034 / 2;
  // Print the distance on the Serial Monitor (Ctrl+Shift+M):
  Serial.print("Distance = ");
  Serial.print(distance);
  Serial.println(" cm");
  delay(100);
  return distance;
}

void setup()
{
  // ESP.wdtEnable(15000);
  // WiFi.config(staticIP, gateway, subnet, dns);
  // Serial port for debugging purposes
  Serial.begin(115200);
  // dht.begin();
  setup_wifi_manager();
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
  setUpOTA();
  //  Serial.begin(115200);

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(TdsSensorPin, INPUT);

  // delay(2000);
  Serial.println("relay settings done");
  // digitalWrite(relay1, LOW);
  // digitalWrite(relay2, LOW);
  //  digitalWrite(relay3, LOW);
  // digitalWrite(relay4, LOW);
}

void loop()
{
  ArduinoOTA.handle();
  server.handleClient();
  doc.clear();
  json = "";
#ifdef test
  // checkAndControlRelays();

#endif
#ifdef reals
  unsigned long currentMillis = millis();
  macAdd = WiFi.macAddress();
  server.handleClient();
  if (currentMillis - previousMillis >= interval)
  {
    getDevice();
    controlLevel();
    phRaw = values[0];
    analog2 = tdsValue;
    delay(1000);
    checkPhRelays();
    float getwater = 0.3937008 * getUltra(); // cm to inches
    getwater = height - getwater;            // the leftover is the real water height
    if (getwater > height)
      getwater = height;
    Serial.print("water filled  level value: ");
    Serial.println(getwater);
    if (pump3 < 7)
    {
      if (getwater > pump) // stat pumping at a lower height set from ui
      {
        Serial.println("pump on");
      }
      // digitalWrite(relay3, LOW);
      if (getwater < 5) // so as not to touch the sensor 5 inches minimum 5 inches is the 20 cm to stop pumping
      {
        // digitalWrite(relay3, HIGH);
      }
      digitalWrite(workingLed, HIGH);
    }
    getwater = getwater + 7;
    getwater = getwater / height; // the tank wont be 100 percent full on ui if i dont add the 7  inches cut off value to the water height
    getwater = getwater * 100;
    Serial.print("water filled  level percentage ");
    Serial.println(getwater);

    Serial.print("water tank height ");
    Serial.println(height);

    Serial.print("water level pump percentage ");
    Serial.println(pump);
    doc[apNames]["waterlevel"] = getwater;
    doc[apNames]["ecSensor"] = analog2;

    // phval = 33;
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    Serial.println("debugs");
    POSTData();
  }
#endif
}

/*
coreypatt
Aug 18, 2023, 11:49 PM


Int1 = D3 = pump1 water pump
Int2 = D2 = signal
Int3 = D1 = pump2 Ph up
Int4 = D0 = pump3 ph down

coreypatt
Aug 03, 2023, 11:39 AM
Com 3 = int2
Com 2 = int3
Com1 = int4

Com 3 = pump 1 water pump
Com 2 = pump 2 PH up pump
Com 1 = pump 3 PH down pump
*/
// Import required libraries
#include <Arduino.h>
#include <ArduinoOTA.h>
#if defined(ESP32)
#include <WiFi.h>
#include <AsyncTCP.h>
#include <WebServer.h> //#include <WebServer.h>
#include <Hash.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
WebServer server(80);
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
ESP8266WebServer server(80);
#endif
#include <WiFiManager.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#include "secrets.h"
#include <ArduinoJson.h>
// Variable as required in source
bool signupOK = false;
int buffer_arr[20], temp = 0;
float calibration_value = 0;
int phval = 0;
unsigned long int avgval;
String macAdd = "";
unsigned long int avgValue; // Store the average value of the sensor feedback
int bootCount = 0;
char name[15] = CLIENT;
// int LED_BUILTIN = 4;
StaticJsonDocument<500> doc;
float b;
int temporarData[10];
#define DHTPIN 2 // Digital pin connected to the DHT sensor
#define phSensorPin A0
// Uncomment the type of sensor in use:
#define DHTTYPE DHT11 // DHT 11
// #define DHTTYPE    DHT22     // DHT 22 (AM2302)
// #define DHTTYPE    DHT21     // DHT 21 (AM2301)

// DHT object
DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;
char apNames[30];
// Updates DHT readings every 10 seconds
const long interval = 5000;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0; // will store last time DHT was updated
#define OTA_HOSTNAME ""           // Leave empty for esp8266-[ChipID]

// Assuming WIFI_MANAGER_STATION_NAME is defined as a string
std::string WIFI_MANAGER_STATION_NAME = "";

void setup_wifi_manager()
{

  WiFiManager wifiManager;

  if (WIFI_MANAGER_STATION_NAME == "")
  {
    // use this for auto generated name ESP + ChipID
    macAdd = WiFi.macAddress();
    // WIFI_MANAGER_STATION_NAME = macAdd;
    // char  * apNames = macAdd.c_str();
    // char apNames[30];
    macAdd.toCharArray(apNames, 30);
    Serial.println(apNames);
    wifiManager.autoConnect(apNames);
    // doc["device"]["APname"] = apNames;
  }

  else
  {
    macAdd = WiFi.macAddress();
    // WIFI_MANAGER_STATION_NAME = macAdd;
    // char  * apNames = macAdd.c_str();
    // char apNames[30];
    macAdd.toCharArray(apNames, 30);
    Serial.println(apNames);
    wifiManager.autoConnect(apNames);
  }
}
void POSTData()
{

  if (WiFi.status() == WL_CONNECTED)
  {
#if defined(ESP8266)
    // X509List cert("0A:C5:5E:61:CD:83:C4:B1:12:16:5D:61:41:6D:C9:C8:CA:7A:F9:D8");
    // BearSSL::WiFiClientSecure client;
    std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
    client->setFingerprint("0A:C5:5E:61:CD:83:C4:B1:12:16:5D:61:41:6D:C9:C8:CA:7A:F9:D8");
    client->setTrustAnchors(new BearSSL::X509List(test_root_ca));
    // Ignore SSL certificate validation
    client->setInsecure();
    if (!client->connect(serverName, 443))
    {
      Serial.println(" not connected!");
      // return;
    }
    else
    {
      Serial.println("  connected!  to mongo db");
    }
    HTTPClient http;

    http.begin(*client, serverName);

#elif defined(ESP32)
    WiFiClientSecure client;
    client.setCACert(test_root_ca);
    if (!client.connect(serverName, 443))
    {
      Serial.println(" not connected!");
      // return;
    }
    HTTPClient http;

    http.begin(client, serverName);
#endif

    http.addHeader("Content-Type", "application/json");

    String json;
    serializeJson(doc, json);

    Serial.println(json);
    int httpResponseCode = http.POST(json);
    Serial.println(httpResponseCode);
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

float readPH()
{
  for (int i = 0; i < 10; i++) // Get 10 sample value from the sensor for smooth the value
  {
    temporarData[i] = analogRead(phSensorPin);
    delay(10);
  }
  for (int i = 0; i < 9; i++) // sort the analog from small to large
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (temporarData[i] > temporarData[j])
      {
        temp = temporarData[i];
        temporarData[i] = temporarData[j];
        temporarData[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++) // take the average value of 6 center sample
    avgValue += temporarData[i];
  float collectedValue = (float)avgValue * 5.0 / 1023 / 6; // convert the analog into millivolt
  float voltage = collectedValue;
  collectedValue = 14 - (1.9 * collectedValue) + 1.5; // convert the millivolt into pH value
  Serial.print("pH:");
  Serial.print(collectedValue, 2);
  Serial.print(" ");
  Serial.print("    raw reading pH:");
  Serial.print(analogRead(phSensorPin));
  Serial.print(" ");
  Serial.print(" voltage mV:");
  Serial.print(voltage);
  /* float pH_Value = analogRead(A0);
   float Voltage = pH_Value * (5.0 / 1023.0);
   float ph_act = -5.60 * voltage + calibration_value;
   Serial.print(" voltage mV raw:");
   Serial.print(Voltage);
   Serial.print("    voltage ph calib: ");
   Serial.println(ph_act);*/
  int analogReading = analogRead(A0);                                                                                       // Read the analog value from the sensor
  float pHValue = 1.1139606788501757e-05 * analogReading * analogReading +  0.013737803115719713 * analogReading -0.6248975748906497; // Quadratic conversion
  return collectedValue;
}
void getDevice()
{

  // esp_sleep_wakeup_cause_t wakeup_reason;
  // wakeup_reason = esp_sleep_get_wakeup_cause();

  // uint64_t chipid = ESP.getEfuseMac();                                                        // The chip ID is essentially its MAC address(length: 6 bytes).
  // Serial.printf("***ESP32 Chip ID = %04X%08X\n", (uint16_t)(chipid >> 32), (uint32_t)chipid); // print High 2 bytes
  // char buffer[200];
  // sprintf(buffer, "%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);
  // sprintf(buffer, "esp32%" PRIu64, ESP.getEfuseMac());

  // int vbatt_raw = 0;
  // for (int i=0;i<SAMPLES;i++)
  // {
  //    vbatt_raw += analogRead(PIN_POWER);
  //    delay(100);
  // }
  // vbatt_raw = vbatt_raw/SAMPLES;
  // float vbatt = map(vbatt_raw, 0, 4096, 0, 4200);

  doc[apNames]["IP"] = WiFi.localIP().toString();
  doc[apNames]["RSSI"] = String(WiFi.RSSI());
  doc[apNames]["type"] = TYPE;
  doc[apNames]["name"] = name;
  // doc[apNames]["chipid"] = buffer;
  doc[apNames]["bootCount"] = bootCount;
  // doc[apNames]["wakeup_reason"] = String(wakeup_reason);
  //  doc[apNames]["vbatt_raw"] = vbatt_raw;
  //  doc[apNames]["vbatt"] = map(vbatt_raw, 0, 4096, 0, 4200);
}

void setup()
{

  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.begin();
  setup_wifi_manager();
  setUpOTA();
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop()
{
  ArduinoOTA.handle();
  server.handleClient();
  unsigned long currentMillis = millis();
  macAdd = WiFi.macAddress();
  if (currentMillis - previousMillis >= interval)
  {
    getDevice();

    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    // float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT))
    {
      Serial.println("Failed to read from DHT sensor!");
      doc[apNames]["temperature"] = random(10);
    }
    else
    {
      t = newT;
      // Serial.println(Firebase.setInt(firebaseData, "/dht/temperature", t));
      // Serial.println(Firebase.RTDB.setFloat(&fbdo, macAdd + "/temprature", t));
      // Serial.println("status: " + fbdo.errorReason());
      doc[apNames]["temperature"] = t;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value
    if (isnan(newH))
    {
      Serial.println("Failed to read humidity from DHT sensor!");
      doc[apNames]["humidity"] = random(100);
    }
    else
    {
      h = newH;
      // Serial.println(Firebase.setInt(firebaseData, "/dht/humidity", h));
      // Serial.println(firebaseData.errorReason());
      // Serial.println(Firebase.RTDB.setFloat(&fbdo, macAdd + "/humidity", h));
      // Serial.println("status: " + fbdo.errorReason());
      doc[apNames]["humidity"] = h;
      Serial.println(h);
    }

    Serial.print("pH Val:");
    Serial.println(readPH());
    delay(1000);
    // Serial.println(Firebase.RTDB.setFloat(&fbdo, macAdd + "/phVal", ph_act));
    float ph = readPH();
    doc[apNames]["phVal"] = (ph);
    // Serial.println("status: " + fbdo.errorReason());

    POSTData();
  }
}
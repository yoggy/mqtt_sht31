//
// mqtt_sht31.ino - sample sketch for WioNode & SHT31
//
// Akiduki SHT31 module
//   http://akizukidenshi.com/catalog/g/gK-12125/
//
// pubsubclient
//   https://github.com/knolleary/pubsubclient/
//
// Adafruit_SHT31
//   https://github.com/adafruit/Adafruit_SHT31
//
// How to use:
//
//     $ git clone https://github.com/yoggy/mqtt_sht31
//     $ cd mqtt_sht31
//     $ cp config.ino.sample config.ino
//     $ vi config.ino
//       - edit wifi_ssid, wifi_password, mqtt_server, mqtt_publish_topic, ... etc
//     $ open mqtt_sht31.ino
//
// license:
//     Copyright (c) 2017 yoggy <yoggy0@gmail.com>
//     Released under the MIT license
//     http://opensource.org/licenses/mit-license.php;
//

////////////////////////////////////////////////////////////////////////////////////
// for WioNode Pin Assign
#define PORT0_D0    3
#define PORT0_D1    1
#define PORT1_D0    5
#define PORT1_D1    4

#define I2C_SCL     PORT1_D0
#define I2C_SDA     PORT1_D1

#define BUTTON_FUNC 0

#define LED_BLUE    2
#define GROVE_PWR   15

// initialize functions for WioNode.
#define LED_ON()  {digitalWrite(LED_BLUE, LOW);}
#define LED_OFF() {digitalWrite(LED_BLUE, HIGH);}
#define ENABLE_GROVE_CONNECTOR_PWR() {pinMode(GROVE_PWR, OUTPUT);digitalWrite(GROVE_PWR, HIGH);}
#define INIT_WIO_NODE() {pinMode(BUTTON_FUNC, INPUT);pinMode(LED_BLUE, OUTPUT);ENABLE_GROVE_CONNECTOR_PWR();LED_OFF();}

////////////////////////////////////////////////////////////////////////////////////
// for Wifi settings
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/

// Wif config
extern char *wifi_ssid;
extern char *wifi_password;
extern char *mqtt_server;
extern int  mqtt_port;

extern char *mqtt_client_id; 
extern bool mqtt_use_auth;
extern char *mqtt_username;
extern char *mqtt_password;

extern char *mqtt_publish_topic;

WiFiClient wifi_client;
void mqtt_sub_callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqtt_client(mqtt_server, mqtt_port, mqtt_sub_callback, wifi_client);

void setup_mqtt() {
  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.mode(WIFI_STA);
  int wifi_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (wifi_count % 2 == 0) {
      LED_ON();
    }
    else {
      LED_OFF();
    }
    wifi_count ++;
    delay(500);
  }

  bool rv = false;
  if (mqtt_use_auth == true) {
    rv = mqtt_client.connect(mqtt_client_id, mqtt_username, mqtt_password);
  }
  else {
    rv = mqtt_client.connect(mqtt_client_id);
  }
  if (rv == false) {
    reboot();
  }

  LED_ON();
}

void reboot() {
  for (int i = 0; i < 10; ++i) {
    LED_ON()
    delay(100);
    LED_OFF()
    delay(100);
  };

  ESP.restart();

  while (true) {
    LED_ON()
    delay(100);
    LED_OFF()
    delay(100);
  };
}

////////////////////////////////////////////////////////////////////////////////////
// Adafruit_SHT31
#include <Adafruit_SHT31.h> //https://github.com/adafruit/Adafruit_SHT31

Adafruit_SHT31 sht31 = Adafruit_SHT31();

////////////////////////////////////////////////////////////////////////////////////
void setup() {
  INIT_WIO_NODE();
  setup_mqtt();

  Serial.begin(115200);

  //Wire.begin(PORT1_D1, PORT1_D0); // SDA(PORT1_D1,white), SCL(PORT1_D0,yellow)
 
  if (!sht31.begin(0x45)) {
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
}

void loop() {
  if (!mqtt_client.connected()) {
    reboot();
  }
  mqtt_client.loop();

  // measurement temperature & humidity
  float t = sht31.readTemperature();
  char  t_str[8];
  dtostrf(t, 4, 2, t_str); // see also...https://stackoverflow.com/questions/27651012/arduino-sprintf-float-not-formatting
  float h = sht31.readHumidity();
  char  h_str[8];
  dtostrf(h, 4, 2, h_str);

  // build message
  char buf[256];
  memset(buf, 0, 256);
  snprintf(buf, 256, "{\"temperature\":%s, \"humidity\":%s}", t_str, h_str);

  // publish message
  Serial.print("publish:message=");
  Serial.println(buf);
  mqtt_client.publish(mqtt_publish_topic, buf, true);

  delay(1000);  
}

void mqtt_sub_callback(char* topic, byte* payload, unsigned int length) {
  LED_OFF()
  delay(50);
  LED_ON()
}



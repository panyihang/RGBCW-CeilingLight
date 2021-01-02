#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define BLINKER_WIFI
#include <Blinker.h>

/* 
    本项目灯板的IO口 
        C（冷光）   -->    GPIO14
        W（暖光）   -->    GPIO13
        R（红光）   -->    GPIO12
        G（绿光）   -->    GPIO5
        B（蓝光）   -->    GPIO4
        注意，在v1.02的灯板中使用G和B是反的！！！ 在v1.03已修复。
*/

#ifndef STASSID
#define STASSID ""
#define STAPSK ""
#endif

char auth[] = ""; // 密钥

#define Slider_1 "light-w" // 暖光
#define Slider_2 "light-c" // 冷光
#define RGB_1 "color" // RGB

const char *ssid = STASSID;
const char *password = STAPSK;

int ColdIO = 14;
int WarmIO = 13;
int RedIO = 12;
int GreenIO = 4;
int BlueIO = 5;

int red = 0;
int green = 0;
int blue = 0;
int bright = 0;

bool switch_state = false;
bool flag = false;

BlinkerRGB RGB1(RGB_1);
BlinkerSlider Slider1(Slider_1);
BlinkerSlider Slider2(Slider_2);

void slider1_callback(int32_t value)
{
  //设置冷光亮度
  analogWrite(ColdIO, value);
  BLINKER_LOG("C value: ", value);
}

void slider2_callback(int32_t value)
{
  //设置暖光亮度
  analogWrite(WarmIO, value);
  BLINKER_LOG("W value: ", value);
}

void rgb1_callback(uint8_t r_value, uint8_t g_value, uint8_t b_value, uint8_t bright_value)
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  red = r_value;
  green = g_value;
  blue = b_value;
  bright = bright_value;
  changeLight();

  //输出log
  BLINKER_LOG("R value: ", r_value);
  BLINKER_LOG("G value: ", g_value);
  BLINKER_LOG("B value: ", b_value);
  BLINKER_LOG("Rrightness value: ", bright_value);

  RGB1.brightness(bright);
  RGB1.print(red, green, blue);
}

void switch_callback(const String &state)
{

  BLINKER_LOG("get switch state: ", state);

  if (state == BLINKER_CMD_ON)
  {
    switch_state = true;
    bright = 255;
    changeLight();
    BUILTIN_SWITCH.print("on");
  }
  else if (state == BLINKER_CMD_OFF)
  {
    switch_state = false;
    bright = 0;
    changeLight();
    digitalWrite(LED_BUILTIN, LOW);

    BUILTIN_SWITCH.print("off");
  }
}

void heartbeat()
{
  if (switch_state)
    BUILTIN_SWITCH.print("on");
  else
    BUILTIN_SWITCH.print("off");
  RGB1.brightness(bright);
  RGB1.print(red, green, blue);
}

void dataRead(const String &data)
{
  BLINKER_LOG("Blinker readString: ", data);
  uint32_t BlinkerTime = millis();
  Blinker.print("millis", BlinkerTime);
}

void changeLight()
{ //将blinker的rgb数据从0-255映射到0-1023
  int red_temp = map(red * bright / 255, 0, 255, 0, 1023);
  int green_temp = map(green * bright / 255, 0, 255, 0, 1023);
  int blue_temp = map(blue * bright / 255, 0, 255, 0, 1023);
  //输入log
  BLINKER_LOG("red_temp: ", red_temp);
  BLINKER_LOG("green_temp: ", green_temp);
  BLINKER_LOG("blue_temp: ", blue_temp);
  //调整PWM
  analogWrite(RedIO, red_temp);
  analogWrite(GreenIO, green_temp);
  analogWrite(BlueIO, blue_temp);
}

void setup()
{
  Serial.begin(115200);
  BLINKER_DEBUG.stream(Serial);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  // 初始化IO口
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ColdIO, OUTPUT);
  pinMode(WarmIO, OUTPUT);
  pinMode(RedIO, OUTPUT);
  pinMode(GreenIO, OUTPUT);
  pinMode(BlueIO, OUTPUT);

  // 为保证使用体验，全部io口直接拉低
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(ColdIO, LOW);
  digitalWrite(WarmIO, LOW);
  digitalWrite(RedIO, LOW);
  digitalWrite(GreenIO, LOW);
  digitalWrite(BlueIO, LOW);

  Blinker.begin(auth, ssid, password);
  Blinker.attachData(dataRead);
  Blinker.attachHeartbeat(heartbeat);
  BUILTIN_SWITCH.attach(switch_callback);

  RGB1.attach(rgb1_callback);

  Slider1.attach(slider1_callback);
  Slider2.attach(slider2_callback);

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR)
    {
      Serial.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      Serial.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      Serial.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      Serial.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  //检测OTA远程升级
  if (flag)
  {
    ArduinoOTA.handle();
  }
  else
  {
    Blinker.run();
  }
}

//目前只能想到用连mqtt来获取OTA升级的flag，但不可控因素过多，如果有好的建议请提issus

const char *mqttServer = "mqtt.panyihang.top";
int mqttPort = 3303;
const char *mqttConnectUserName = "";
const char *mqttConnectUserPassword = "";
const char *mqttConnectTopic = "rgbcwLight-testing";

void getArduinoOTAFlag()
{

  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    delay(1000);
    ESP.restart();
  }
}

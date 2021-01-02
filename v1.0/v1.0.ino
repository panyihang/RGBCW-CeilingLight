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
#define STASSID "TP-LINK_5AFF"
#define STAPSK "5t4r3e2w1q"
#endif

char auth[] = "ece4c6d4ec0e"; // 密钥

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
}

void loop()
{
    Blinker.run();
}

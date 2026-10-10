#include <Arduino.h>

//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// можете включить вывод отладочной информации в Serial на 115200
//#define REMOTEXY__DEBUGLOG    

// определение режима соединения и подключение библиотеки RemoteXY 
#define REMOTEXY_MODE__WIFI_POINT

#include <ESP8266WiFi.h>

// настройки соединения 
#define REMOTEXY_WIFI_SSID "RemoteXY"
#define REMOTEXY_WIFI_PASSWORD "12345678"
#define REMOTEXY_SERVER_PORT 6377


#include <RemoteXY.h>

// конфигурация интерфейса RemoteXY  
#pragma pack(push, 1)  
uint8_t const PROGMEM RemoteXY_CONF_PROGMEM[] =   // 42 bytes V19 
  { 255,2,0,0,0,35,0,19,0,0,0,69,83,80,56,50,54,54,0,82,
  2,106,200,200,84,1,1,1,0,5,44,125,59,59,154,38,43,43,32,65,
  26,31 };
  
// структура определяет все переменные и события вашего интерфейса управления 
struct {
    // input variables
  int8_t joystick_01_x; // oт -100 до 100
  int8_t joystick_01_y; // oт -100 до 100

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

uint32_t lastPrint = 0;

class MotorsControl {
private:
  static constexpr uint8_t AIN1 = D1;
  static constexpr uint8_t AIN2 = D5;
  static constexpr uint8_t BIN1 = D6;
  static constexpr uint8_t BIN2 = D7;

  /// Устанавливает значение пинам мотора
  void setVals(uint8_t ain1_val, uint8_t ain2_val, uint8_t ain3_val, uint8_t ain4_val){
    analogWrite(AIN1, ain1_val);
    analogWrite(AIN2, ain2_val);
    analogWrite(BIN1, ain3_val);
    analogWrite(BIN2, ain4_val);
  }  

public:
  void init(){
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    analogWriteRange(255); 
    stop();
  }

  void stop()           { setVals(0, 0, 0, 0); }
  void forward(int s)   { setVals(s, 0, s, 0); }
  void backward(int s)  { setVals(0, s, 0, s); }
  void right(int s)     { setVals(0, 0, s, 0); }
  void left(int s)      { setVals(s, 0, 0, 0); }
};

MotorsControl Motors;

constexpr uint8_t JOYSTICK_DEADZONE = 40;
constexpr uint8_t MIN_PWM = 200;

void setup() 
{
  RemoteXY_Init ();  // initialization by macros 
   
  // TODO you setup code
  Serial.begin(115200);
  Serial.setTimeout(50);

  Motors.init();
}

void loop() 
{ 
  RemoteXYEngine.handler ();   
    
  // TODO you loop code
  // используйте структуру RemoteXY для передачи данных
  // не используйте функцию delay(), вместо нее используйте RemoteXYEngine.delay() 

  if(millis() - lastPrint >= 10){
    lastPrint = millis();
/*
    Serial.print("X: ");
    Serial.print(RemoteXY.joystick_01_x);
    Serial.print(" Y: ");
    Serial.println(RemoteXY.joystick_01_y);
*/

    int8_t x = RemoteXY.joystick_01_x;
    int8_t y = RemoteXY.joystick_01_y;

    if(!RemoteXY.connect_flag || (abs(y) < JOYSTICK_DEADZONE && abs(x) < JOYSTICK_DEADZONE)){
      Motors.stop();
      return;
    }

    int8_t dir = max(abs(x), abs(y));
    uint8_t speed = map(dir, JOYSTICK_DEADZONE, 100, MIN_PWM, 255);

    if(abs(y) >= abs(x)){
      if(y > 0) Motors.forward(speed);
      else Motors.backward(speed);
    }
    else{
      if(x > 0) Motors.right(speed);
      else Motors.left(speed);     
    }
  }
}


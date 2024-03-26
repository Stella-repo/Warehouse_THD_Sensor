int count = 0;
long mil = 0;


//릴레이
#include <NTPClient.h>
#include <WiFiUdp.h>
int stop = 50, start = 55;
int setstatus = 0;
int Relaypin = 16;
int dehumi = 0;
//릴레이


//디스플레이
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED 가로 넓이, 픽셀 사이즈
#define SCREEN_HEIGHT 64 // OLED 세로 넓이, 픽셀 사이즈
 
#define OLED_RESET  -1 // 리셋핀 #(또는 -1 아두이노와 리셋핀을 연결하는 경우)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
//디스플레이


//온습도센서
#include <DHT11.h>
int pin = 0;  // 핀설정
DHT11 dht11(pin);
float humi, temp;
float humi_log[] = {0,0,0,0,0,0,0,0,0,0};
float temp_log[] = {0,0,0,0,0,0,0,0,0,0};
float humi_avg;
float temp_avg;
float voMeasured_avg;
float humi_sum;
float temp_sum;
float voMeasured_sum;
String humi_status;
String temp_status;
//온습도센서


//먼지센서
int measurePin = 0; //Connect dust sensor to Arduino A0 pin
int ledPower = 2;   //Connect 3 led driver pins of dust sensor to Arduino D2
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
float voMeasured_log[] = {0,0,0,0,0,0,0,0,0,0};
String voMeasured_status;
//먼지센서


//텔레그램
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Initialize Wifi connection to the router
char ssid[] = "your network SSID (name)";     // your network SSID (name)
char password[] = "your network key"; // your network key
String mychatid = "mychatid";
int senddata = 0;

// Initialize Telegram BOT
#define BOTtoken "YOURTOKEN"  // your Bot Token (Get from Botfather)

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

bool Start = false;

const int ledPin = D5;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    String text = bot.messages[i].text;

    if (chat_id = mychatid) {
      String from_name = bot.messages[i].from_name;
      
      if (text == "/status") {
        bot.sendMessage(chat_id, "기온 : " + String(temp_avg) + "  습도 : " + String(humi_avg) + "  먼지 : " + String(voMeasured_avg) + "\n정지습도 : " + stop + "  동작습도 : " + start + "\n수신상태 : " + senddata + "  제습기동작상태 : " + dehumi);
      }

      else if (text == "/set") { //얘가 위에 있으면 setstatus = 1상태로 내려가서 오작동함
        bot.sendMessage(chat_id, "설정할 습도를 <정지습도-동작습도>로 적어주세요.\n상태정보 수신을 설정하려면 /on /off 를 보내주세요.\n설정을 취소하려면 /cancel 이라고 보내주세요.");
        setstatus = 1;
      }
      else {
        if (setstatus == 1) {
          if (text == "/cancel") {
            setstatus = 0;
            bot.sendMessage(chat_id, "동작습도 설정이 취소되었습니다.");
          }
          else if (text == "/on") {
            senddata = 1;
            bot.sendMessage(chat_id, "상태정보를 수신을 켭니다.");            
          }
          else if (text == "/off") {
            senddata = 0;
            bot.sendMessage(chat_id, "상태정보를 수신을 끕니다.");            
          }
          else {
            stop = (text.substring(0, 2)).toInt();
            start = (text.substring(3, 5)).toInt();
            Serial.println(stop, start);
            bot.sendMessage(chat_id, "설정완료!\n정지습도 : " + String(stop) + "  동작습도 : " + String(start));
            setstatus = 0;
          }
        }
      }



      if (text.substring(0,3) == "set") {
        stop = (text.substring(4, 6)).toInt();
        start = (text.substring(7, 9)).toInt();  
        bot.sendMessage(chat_id, "설정완료!\n정지습도 : " + String(stop) + "  동작습도 : " + String(start));
      }
    }
  }
}
//텔레그램


//NTP서버 시간
WiFiUDP udp;
NTPClient timeClient(udp, "kr.pool.ntp.org", 32400);
int status = 1;
//NTP서버 시간




void setup() {
  client.setInsecure();  
  Serial.begin(9600);

  //릴레이
  pinMode(Relaypin,OUTPUT);
  digitalWrite(Relaypin, HIGH);
  //릴레이

  //디스플레이
  // SSD1306_SWITCHCAPVCC = 내부 3.3V 차지 펌프 회로를 켜둔다.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
  Serial.println(F("SSD1306 Not Connected"));
  for(;;); // SSD1306에 주소할당이 되지 않으면 무한루프
  }
  display.setRotation(2);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0,13);
  display.print("Now\nStarting..");
  display.display();
  //디스플레이
  
  pinMode(ledPower,OUTPUT); //먼지센서 센서led

  //텔레그램
    // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //텔레그램
  
  //NTP서버 시간
  timeClient.begin();
  timeClient.update();
  Serial.println(timeClient.getFormattedTime());
  //NTP서버 시간
}
 
void loop() {
  //온습도센서
  dht11.read(humi, temp);
  for (int i = 0; i < 10; i++)
  {
    humi_log[i] = humi_log[i+1];
    temp_log[i] = temp_log[i+1];
  }
  humi = humi + 6;
  temp = temp;
  humi_log[9] = humi;
  temp_log[9] = temp;
  //온습도센서

  
  //먼지센서
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(samplingTime);
 
  voMeasured = analogRead(measurePin); // read the dust value
 
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(sleepTime);
 
  // 0 - 5V mapped to 0 - 1023 integer values
  // recover voltage
  calcVoltage = voMeasured * (3.3 / 1024.0);
 
  // linear eqaution taken from http://www.howmuchsnow.com/arduino/airquality/
  // Chris Nafis (c) 2012
  dustDensity = 0.17 * calcVoltage - 0;

  for (int i = 0; i < 10; i++)
  {
    voMeasured_log[i] = voMeasured_log[i+1];
  }
  voMeasured_log[9] = voMeasured;
  //먼지센서


  //텔레그램
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while(numNewMessages) {
    Serial.println("got response");
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
  //텔레그램  

  
  if (count > 9) {
    float humi_sum = 0;
    float temp_sum = 0;
    float voMeasured_sum = 0;
    for (int i = 0; i < 10; i++)
    {
      humi_sum = humi_sum + humi_log[i];
      temp_sum = temp_sum + temp_log[i];
      voMeasured_sum = voMeasured_sum + voMeasured_log[i];
    }
    humi_avg = humi_sum/10;
    temp_avg = temp_sum/10;
    voMeasured_avg = voMeasured_sum/10;

    if (humi_avg < 40)
      {humi_status = "G";}
    else if (humi_avg < 60)
      {humi_status = "M";}
    else
      {humi_status = "B";}
    if (temp_avg < 5)
      {temp_status = "C";}
    else if (temp_avg < 35)
      {temp_status = "G";}
    else
      {temp_status = "B";}
    if (voMeasured_avg < 100)
      {voMeasured_status = "G";}
    else if (voMeasured_avg <400)
      {voMeasured_status = "B";}
    else
      {voMeasured_status = "VB";}
    
    //디스플레이
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,0);
    display.println("temp " + String(temp) + "\nhumi " + String(humi));
    display.println("dust " + String(voMeasured));
    display.println(humi_status + " " + temp_status + " " + voMeasured_status);
    display.display();
    //디스플레이

    //릴레이
    if (dehumi == 1) {
      if (humi <= float(stop)) {
        digitalWrite(Relaypin, HIGH);
        dehumi = 0;
        if (senddata == 1) {
          bot.sendMessage("1698174738", "제습기 작동 정지");
        }
        Serial.println("제습기 작동 정지");
      }
    }
    if (dehumi == 0) {
      if (humi >= float(start)) {
        digitalWrite(Relaypin, LOW);
        dehumi = 1;
        if (senddata == 1) {
          bot.sendMessage("1698174738", "제습기 작동 시작");
        }
        Serial.println("제습기 작동 시작");
      }
    }
    //릴레이
   }

  else {
    int countdown = 10 - count;
    Serial.println("Plase wait " + String(countdown) + " loop");

    //디스플레이
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0,13);
    display.print("Plase wait " + String(countdown) + " loop");
    display.display();
    //디스플레이
  }


  //1시간마다 현재상태 전송
  if (status == 1) { //상태가 1이고 00분이 되면 전송하고 시간 받아오고 상태를 0으로 바꾸기
    if (String(timeClient.getFormattedTime()).substring(3, 5) == "00") {
      if (senddata == 1) {
        bot.sendMessage("1698174738", "기온 : " + String(temp_avg) + "  습도 : " + String(humi_avg) + "  먼지 : " + String(voMeasured_avg));
      }
      timeClient.update();
      status = 0;
    }
  }
  if (status == 0) { //상태가 0일때(한번 전송했을때) 01분이 되면 상태 1로 바꾸기
    if (String(timeClient.getFormattedTime()).substring(3, 5) == "01") {
      status = 1;
    }
  }
  //1시간마다 현재상태 전송
     
  Serial.println("humidity : " + String(humi) + " - temperature : " + String(temp));
  Serial.println("Raw Signal Value (0-1023) : " + String(voMeasured));  
  
  Serial.println("humidity_avg : " + String(humi_avg) + " - temperature_avg : " + String(temp_avg)); 
  Serial.println("Raw Signal Value_avg (0-1023) : " + String(voMeasured_avg) + "\n");


  count = count + 1;
      
} 

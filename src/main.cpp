#include <Arduino.h>
#include <device.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <WiFi.h>
#include <Servo.h>  
#include <firebase.h>
#include <WiFiManager.h>
#include "NTP.h"
#include <TRIGGER_GOOGLESHEETS.h>

// #include <LiquidCrystal_I2C.h>
// #include <Wire.h>

// LiquidCrystal_I2C lcd(0x27,20,4);
//  ntp NTP;
// int ir1_value;
// int ir2_value;
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;
Servo servo5;
Servo servo6;
Servo servo7;
Servo servo8;
// Servo servo9;
// Servo servo10;
WiFiUDP wifiUdp;
NTP ntp(wifiUdp);

SemaphoreHandle_t xSemaphore;

// const char *ssid = "Lekker coffee";
// const char *password = "monyetada1";

volatile bool _irState1 = false;
volatile bool _irState2 = false;
volatile bool _irState3 = false;
volatile bool _irState4 = false;
volatile bool _irState5 = false;
volatile bool _irState6 = false;
volatile bool _irState7 = false;
volatile bool _irState8 = false;
volatile bool sendData = false;

// void WifiConnect();
void onSendSensor(void *parameter);
void sendDataToFirebase(void *parameter);
void sendDataToGoogleSheets(void *parameter);

// void bacasensor(void *pvParameters)
// {
//   while (1)
//   {
//     ir1_value = digitalRead(IR1_PIN);
//     ir2_value = digitalRead(IR2_PIN);
//     xSemaphoreGive(xSemaphore);
//     vTaskDelay(pdMS_TO_TICKS(100));
//     if (ir1_value == LOW)
//     {
//       // Serial.println("Gate 1: Ada objek terdeteksi! ");
//     }
//     delay(2000);

//     if (ir2_value == LOW)
//     {
//       // Serial.println("Gate 2: Ada objek terdeteksi! ");
//     }
//     vTaskDelay(100 / portTICK_PERIOD_MS);
//   }
// }

// void kontrolservo(void *pvParameters)
// {

//   int pos1 = 0;
//   int pos2 = 0;
//   while (1)
//   {
//     if (ir1_value == HIGH)
//     {

//       if (pos1 < 90)
//       {
//         pos1++;
//         servo1.write(pos1);
//       }
//     }
//     else
//     {
//       if (pos1 > 0)
//       {
//         pos1--;
//         servo1.write(pos1);
//       }
//     }
//     if (ir2_value == HIGH)
//     {
//       if (pos2 < 90)
//       {
//         pos2++;
//         servo2.write(pos2);
//       }
//     }
//     else
//     {
//       if (pos2 > 0)
//       {
//         pos2--;
//         servo2.write(pos2);
//       }
//     }
//     vTaskDelay(pdMS_TO_TICKS(2)); // delay non-blocking
//   }
// }
char column_name_in_sheets[][20] = {"value1", "value2", "value3", "value4", "value5", "value6", "value7", "value8"}; /*1. The Total no of column depends on how many value you have created in Script of Sheets;2. It has to be in order as per the rows decided in google sheets*/
String Sheets_GAS_ID = "AKfycbz-vKy-jBDgvi8coeU4O3L7gP-DQteLYjRnx6RvQSvVUGSU2yc3yhGC6zRbLeKTWRHQiQ";                  /*This is the Sheets GAS ID, you need to look for your sheets id*/
int No_of_Parameters = 8;

void setup()
{
  Serial.begin(115200);
  pinMode(PUSH_BUTTON, INPUT_PULLUP);
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);
  servo3.attach(SERVO3_PIN);
  servo4.attach(SERVO4_PIN);
  servo5.attach(SERVO5_PIN);
  servo6.attach(SERVO6_PIN);
  servo7.attach(SERVO7_PIN);
  servo8.attach(SERVO8_PIN);
  // servo9.attach(SERVO9_PIN);
  // servo10.attach(SERVO10_PIN);
  // pinMode(IR1_PIN, INPUT_PULLUP);
  // pinMode(IR2_PIN, INPUT_PULLUP);
  pinMode(IR_SENSOR_PIN_1, INPUT);
  pinMode(IR_SENSOR_PIN_2, INPUT);
  pinMode(IR_SENSOR_PIN_3, INPUT);
  pinMode(IR_SENSOR_PIN_4, INPUT);
  pinMode(IR_SENSOR_PIN_5, INPUT);
  pinMode(IR_SENSOR_PIN_6, INPUT);
  pinMode(IR_SENSOR_PIN_7, INPUT);
  pinMode(IR_SENSOR_PIN_8, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  // WifiConnect();
  WiFiManager wifiManager;

  if (digitalRead(PUSH_BUTTON) == LOW)
  {
    Serial.println("Button pressed, starting configuration portal");
    wifiManager.startConfigPortal("ESP32-AP");
    wifiManager.resetSettings();
    digitalWrite(LED_BUILTIN, LOW);
  }
  wifiManager.autoConnect("AdminValet", "halloadmin"); // buat password wifi
  Serial.println("connected...yeey :)");
  digitalWrite(LED_BUILTIN, HIGH);

  Google_Sheets_Init(column_name_in_sheets, Sheets_GAS_ID, No_of_Parameters);
  /*This function is used to initialize the google sheets*/
  Firebase_Init("parkingArea");
  delay(2000);
  ntp.ruleSTD("UTC", Third, Mon, Jun, 19, 420); // last sunday in october 3:00, timezone +60min (+1 GMT)
  ntp.begin();
  xSemaphore = xSemaphoreCreateBinary();
  if (xSemaphore == NULL)
  {
    Serial.println("Semaphore creation failed!");
  }
  xSemaphoreGive(xSemaphore);
  // xTaskCreate(bacasensor, "membaca sensor", 4096, NULL, 2, NULL);
  // xTaskCreate(kontrolservo, "Servo Controller", 4096, NULL, 1, NULL);
  xTaskCreatePinnedToCore(onSendSensor, "onSendSensor", 10000, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(sendDataToFirebase, "sendDataToFirebase", 10000, NULL, 1, NULL, 0);
  // xTaskCreatePinnedToCore(sendDataToGoogleSheets, "sendDataToGoogleSheets", 10000, NULL, 1, NULL, 0);
  // Data_to_Sheets(No_of_Parameters, "message");


  Serial.println("System ready.");
}

void loop()
{
  ntp.update();
  
}

// void WifiConnect()
// {
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(1000);
//     Serial.println("Connecting to WiFi...");
//   }
//   Serial.println("WiFi connected.");
// }

void onSendSensor(void *parameter)
{
  while (true)
  {
    if (xSemaphoreTake(xSemaphore, (TickType_t)10) == pdTRUE)
    {
      _irState1 = digitalRead(IR_SENSOR_PIN_1) == LOW ? true : false;
      _irState2 = digitalRead(IR_SENSOR_PIN_2) == LOW ? true : false;
      _irState3 = digitalRead(IR_SENSOR_PIN_3) == LOW ? true : false;
      _irState4 = digitalRead(IR_SENSOR_PIN_4) == LOW ? true : false;
      _irState5 = digitalRead(IR_SENSOR_PIN_5) == LOW ? true : false;
      _irState6 = digitalRead(IR_SENSOR_PIN_6) == LOW ? true : false;
      _irState7 = digitalRead(IR_SENSOR_PIN_7) == LOW ? true : false;
      _irState8 = digitalRead(IR_SENSOR_PIN_8) == LOW ? true : false;
      sendData = true;
      xSemaphoreGive(xSemaphore);
    }
    vTaskDelay(pdMS_TO_TICKS(1000)); // Menunda eksekusi task selama 1 detik
  }
}
void sendDataToFirebase(void *pvParameters)
{
  bool prevIrState1 = _irState1;
  bool prevIrState2 = _irState2;
  bool prevIrState3 = _irState3;
  bool prevIrState4 = _irState4;
  bool prevIrState5 = _irState5;
  bool prevIrState6 = _irState6;
  bool prevIrState7 = _irState7;
  bool prevIrState8 = _irState8;
  while (1)
  {
    if (xSemaphoreTake(xSemaphore, (TickType_t)10) == pdTRUE)
    {
      Serial.println("Semaphore taken by sendDataToFirebase");

      if (_irState1 != prevIrState1)
      {
        String message1 = _irState1 ? "TERISI MOBIL" : "KOSONG";
        Serial.print("Infrared 1: ");
        Serial.println(message1);
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_1/Status", message1);
        String formattedTime1 = ntp.formattedTime(" %d %B  %Y - %T");
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_1/Tanggal-Jam", formattedTime1);
        prevIrState1 = _irState1;
      }

      if (_irState2 != prevIrState2)
      {
        String message2 = _irState2 ? "TERISI MOBIL" : "KOSONG";
        Serial.print("Infrared 2: ");
        Serial.println(message2);
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_2/Status", message2);
        String formattedTime2 = ntp.formattedTime("%d %B  %Y - %T");
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_2/Tanggal-Jam", formattedTime2);
        prevIrState2 = _irState2;
      }

      if (_irState3 != prevIrState3)
      {
        String message3 = _irState3 ? "TERISI MOBIL" : "KOSONG";
        Serial.print("Infrared 3: ");
        Serial.println(message3);
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_3/Status", message3);
        String formattedTime3 = ntp.formattedTime("%d %B  %Y - %T");
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_3/Tanggal-Jam", formattedTime3);
        prevIrState3 = _irState3;
      }

      if (_irState4 != prevIrState4)
      {
        String message4 = _irState4 ? "TERISI MOBIL" : "KOSONG";
        Serial.print("Infrared 4: ");
        Serial.println(message4);
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_4/Status", message4);
        String formattedTime4 = ntp.formattedTime("%d %B  %Y - %T");
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_4/Tanggal-Jam", formattedTime4);
        prevIrState4 = _irState4;
      }

      if (_irState5 != prevIrState5)
      {
        String message5 = _irState5 ? "TERISI MOBIL" : "KOSONG";
        Serial.print("Infrared 5: ");
        Serial.println(message5);
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_5/Status", message5);
        String formattedTime5 = ntp.formattedTime(" %d %B  %Y - %T");
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_5/Tanggal-Jam", formattedTime5);
        prevIrState5 = _irState5;
      }

      if (_irState6 != prevIrState6)
      {
        String message6 = _irState6 ? "TERISI MOBIL" : "KOSONG";
        Serial.print("Infrared 6: ");
        Serial.println(message6);
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_6/Status", message6);
        String formattedTime6 = ntp.formattedTime(" %d %B  %Y - %T");
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_6/Tanggal-Jam", formattedTime6);
        prevIrState6 = _irState6;
      }
      if (_irState7 != prevIrState7)
      {
        String message7 = _irState7 ? "TERISI MOBIL" : "KOSONG";
        Serial.print("Infrared 7: ");
        Serial.println(message7);
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_7/Status", message7);
        String formattedTime7 = ntp.formattedTime("%d %B  %Y - %T");
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_7/Tanggal-Jam", formattedTime7);
        prevIrState7 = _irState7;
      }
      if (_irState8 != prevIrState8)
      {
        String message8 = _irState8 ? "TERISI MOBIL" : "KOSONG";
        Serial.print("Infrared 8: ");
        Serial.println(message8);
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_8/Status", message8);
        String formattedTime8 = ntp.formattedTime(" %d %B  %Y - %T");
        Firebase.RTDB.setString(&fbdo, "/parkingArea/SLOT_8/Tanggal-Jam", formattedTime8);
        prevIrState8 = _irState8;
      }

      Serial.println("Data sent to Firebase.");
      sendData = true;

      xSemaphoreGive(xSemaphore);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void sendDataToGoogleSheets(void *pvParameters)
{

  while (1)
  {
    if (xSemaphoreTake(xSemaphore, (TickType_t)10) == pdTRUE)
    {
      Serial.println("Semaphore taken by sendDataToGoogleSheets task");

      if (_irState1 == HIGH )
      {
        String message1 = "TERISI" ;
        Data_to_Sheets(No_of_Parameters, &message1);
         
   
      }
      else if (_irState1 == LOW){
        String message1 = "KOSONG";
        Data_to_Sheets(No_of_Parameters, &message1);
      }


      Serial.println("Data sent to Google.");
      sendData = true;

      xSemaphoreGive(xSemaphore);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void onFirebaseStream(FirebaseStream data)
{
  // onFirebaseStream: /dataparking/test string true/false
  Serial.printf("onFirebaseStream: %s %s %s %s\n", data.streamPath().c_str(),
                data.dataPath().c_str(), data.dataType().c_str(),
                data.stringData().c_str());
  Serial.println(data.dataPath());
  Serial.println(data.dataPath() == "/SLOT_1/Gate");
  if (data.dataType() == "string")
  {
    String value = data.stringData();
    if (data.dataPath() == "/SLOT_1/Gate")
    {
      if (value == "FALSE") // Periksa nilai data
      {
        servo1.write(90);

        Serial.println("Servo berputar 90 derajat");
      }
      else if (value == "TRUE") // Periksa nilai data
      {
        servo1.write(0);

        Serial.println("Servo berputar 0 derajat");
      }
    }

    if (data.dataPath() == "/SLOT_2/Gate")
    {
      if (value == "FALSE") // Periksa nilai data
      {
        servo2.write(90);

        Serial.println("Servo berputar 90 derajat");
      }
      else if (value == "TRUE") // Periksa nilai data
      {
        servo2.write(0);

        Serial.println("Servo berputar 0 derajat");
      }
    }
    if (data.dataPath() == "/SLOT_3/Gate")
    {
      if (value == "FALSE") // Periksa nilai data
      {
        servo3.write(180);

        Serial.println("Servo berputar 90 derajat");
      }
      else if (value == "TRUE") // Periksa nilai data
      {
        servo3.write(90);

        Serial.println("Servo berputar 0 derajat");
      }
    }
    if (data.dataPath() == "/SLOT_4/Gate")
    {
      if (value == "FALSE") // Periksa nilai data
      {
        servo4.write(90);

        Serial.println("Servo berputar 90 derajat");
      }
      else if (value == "TRUE") // Periksa nilai data
      {
        servo4.write(0);

        Serial.println("Servo berputar 0 derajat");
      }
    }
    if (data.dataPath() == "/SLOT_5/Gate")
    {
      if (value == "FALSE") // Periksa nilai data
      {
        servo5.write(180);

        Serial.println("Servo berputar 90 derajat");
      }
      else if (value == "TRUE") // Periksa nilai data
      {
        servo5.write(90);

        Serial.println("Servo berputar 0 derajat");
      }
    }
    if (data.dataPath() == "/SLOT_6/Gate")
    {
      if (value == "FALSE") // Periksa nilai data
      {
        servo6.write(180);

        Serial.println("Servo berputar 90 derajat");
      }
      else if (value == "TRUE") // Periksa nilai data
      {
        servo6.write(90);

        Serial.println("Servo berputar 0 derajat");
      }
    }
    if (data.dataPath() == "/SLOT_7/Gate")
    {
      if (value == "FALSE") // Periksa nilai data
      {
        servo7.write(180);

        Serial.println("Servo berputar 90 derajat");
      }
      else if (value == "TRUE") // Periksa nilai data
      {
        servo7.write(90);

        Serial.println("Servo berputar 0 derajat");
      }
    }
    if (data.dataPath() == "/SLOT_8/Gate")
    {
      if (value == "FALSE") // Periksa nilai data
      {
        servo8.write(180);

        Serial.println("Servo berputar 90 derajat");
      }
      else if (value == "TRUE") // Periksa nilai data
      {
        servo8.write(90);

        Serial.println("Servo berputar 0 derajat");
      }
    }
  }
}

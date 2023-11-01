#ifndef __TRIGGER_GOOGLESHEETS_H__
#define __TRIGGER_GOOGLESHEETS_H__

#include <WiFiManager.h>
#include <HTTPClient.h>

void sheets_initialization();
void Send_Data();
void Google_Sheets_Init(char array_2d[][20], String sheets_gas_id, int param_size);

String url;

char column_name[][100] = {"SLOT1", "SLOT2", "SLOT3", "SLOT4", "SLOT5", "SLOT6", "SLOT7", "SLOT8"};

String random_values[100];

void float_to_string();
const char* host = "script.google.com";
const int httpsPort = 443;

char Sensor_Values[10][10];
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String GAS_ID;
int Count;

void Google_Sheets_Init(char test[][20], String sheets_gas_id, int param_size)
{
  GAS_ID = sheets_gas_id;
  Count = param_size;

  for (int i = 0; i < Count; i++)
  {
    for (int j = 0; j < 20; j++)
    {
      column_name[i][j] = test[i][j];
    }
  }

#ifdef Debug_Serial_Mon
  for (int i = 0; i < Count; i++)
  {
    Serial.print("column_name= ");
    Serial.println(column_name[i]);
  }
#endif
}

void Data_to_Sheets(int num, String message[])
{
  for (int i = 0; i < num; i++)
  {
    Serial.println(message[i]);
    random_values[i] = message[i];
  }

  float_to_string();
  Send_Data();
}


void float_to_string()
{
  for (int j = 0; j < Count; j++)
  {
   
    Serial.println(random_values[j]);
    sprintf(Sensor_Values[j], "%s", random_values[j]);
#ifdef Debug_Serial_Mon
    Serial.print("Sensor Values : ");
    Serial.println(Sensor_Values[j]);
#endif
  }
}

void Send_Data()
{
  sheets_initialization();

  String url = "https://script.google.com/macros/s/" + GAS_ID + "/exec?";
  
  int i = 0;
  while (i != Count)
  {
    if (i == 0)
    {
      url = url + column_name[i] + "=" + Sensor_Values[i];
      i++;
    }
    if (i == Count)
      break;
    url = url + "&" + column_name[i] + "=" + Sensor_Values[i];
    i++;
  }

  Serial.print("requesting URL: ");
  Serial.println(url);

  HTTPClient http;

  if (http.begin(url))
  {
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();
#ifdef Debug_Serial_Mon
        Serial.println(payload);
#endif
      }
    }
    else
    {
#ifdef Debug_Serial_Mon
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
#endif
    }
    http.end();
  }
  else
  {
#ifdef Debug_Serial_Mon
    Serial.printf("[HTTP] Unable to connect\n");
#endif
  }
}

void sheets_initialization()
{
  WiFiManager wifiManager;
//   // wifiManager.autoConnect("WiFiAP");  // Atur SSID WiFi AP di sini
//   // Serial.println("Connected to WiFi");
}

#endif

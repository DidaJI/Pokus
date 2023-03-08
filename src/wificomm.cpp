//**********************************************************************************************************************
//******                    M Y   W i F i   A N D   C O M M U N I C A T I O N   L I B R A R Y                     ******
//******                      Moje knihovna pro připojení na WiFi a komunikaci se serverem                        ******
//**********************************************************************************************************************
#include "wificomm.h"
#include <Preferences.h>
#include <ArduinoJson.h>

Preferences preferencesWiFi;
Preferences preferencesCheckPin;


struct Record
{
 String Command;
 String Pin;
 String TimeFrom;
 String TimeTo;
};
Record CommandTable[100];


//======================================================================================================================
//===== Pripojeni k WiFi                                                                                           =====
//======================================================================================================================
void WiFiConnect()
{
 //unsigned int TimeOut = 2; // 2s 
 WiFi.begin(Myssid, Mypassword);
 Serial.println("Connecting to WiFi..");
 
 /*
 while (WiFi.status() != WL_CONNECTED) {
    unsigned char i;
    for (i = 0; i < 5; i++)
    {
     digitalWrite(LiveLED, HIGH);
     delay(100);
     digitalWrite(LiveLED, LOW);
     delay(100);  
     
     // Reset WD timer
     // esp_task_wdt_reset();
    }
    TimeOut--;
    if (TimeOut == 0) break;
 }
 
 Serial.println("Connected to the WiFi network");
 Serial.print("Connected, IP address: ");
 Serial.println(WiFi.localIP());
 */
}  


//======================================================================================================================
//===== Zadost o cas ze serveru                                                                                    =====
//======================================================================================================================
void WiFiGetTime()
{
 
 if (WiFi.status() == WL_CONNECTED)
 {
  HTTPClient http;
  
  String htmlStr;
  
  //strcpy(htmlStr, ServerAdr);
  
  // Time request
  htmlStr = ServerAdr; 
  htmlStr += "c_action.php?readerID=";
  htmlStr += readerID;
  htmlStr += "&actionID=FD";

  // Osoby request
  //http://192.168.0.1/dochazka/webapi.php?uid=zw&password=morava&command=ACCESSLIST&data=<xml><readerid>1</readerid></xml>
  //htmlStr = ServerAdr; 
  //htmlStr += "webapi.php?uid=zw&password=morava&command=ACCESSLIST&data=<xml><readerid>1</readerid></xml>";
  

  // Your Domain name with URL path or IP address with path
  http.begin(htmlStr.c_str());
      
  // Send HTTP GET request
  int httpResponseCode = http.GET();
      
  if (httpResponseCode > 0) {
   Serial.print("HTTP Response code: ");
   Serial.println(httpResponseCode);
   String payload = http.getString();
   Serial.println(payload);

   // Nastaveni datumu a casu
   unsigned int year, month, day, hour, min, sec, dts;
   year = 2000 + (payload[6] - 48) * 10 + payload[7] - 48;
   month = (payload[8] - 48) * 10 + payload[9] - 48;
   day = (payload[10] - 48) * 10 + payload[11] - 48;
   hour = (payload[14] - 48) * 10 + payload[15] - 48;
   min = (payload[16] - 48) * 10 + payload[17] - 48;
   sec = (payload[18] - 48) * 10 + payload[19] - 48;
   dts = 1;   
   SetTime(year, month, day, hour, min, sec, dts);

   OnLine = true;
  }
  else 
  {
   Serial.print("Error code: ");
   Serial.println(httpResponseCode);
   OnLine = false;
  }
  // Free resources
  http.end();
 }
 else 
 {
  Serial.println("WiFi Disconnected");
  OnLine = false;
 }
  //  lastTime = millis();

}


//======================================================================================================================
//===== Zadost o status ze serveru                                                                                 =====
//======================================================================================================================
void WiFiGetState()
{
 
 if (WiFi.status() == WL_CONNECTED)
 {
  HTTPClient http;
  
  String htmlStr;
  
  //strcpy(htmlStr, ServerAdr);
  
  // Time request
  htmlStr = ServerAdr; 
  htmlStr += "c_action.php?readerID=";
  htmlStr += readerID;
  htmlStr += "&init=1";

  // Your Domain name with URL path or IP address with path
  http.begin(htmlStr.c_str());
      
  // Send HTTP GET request
  int httpResponseCode = http.GET();
      
  if (httpResponseCode > 0) {
   Serial.print("HTTP Response code: ");
   Serial.println(httpResponseCode);
   String payload = http.getString();
   Serial.println(payload);
   OnLine = true;
  }
  else 
  {
   Serial.print("Error code: ");
   Serial.println(httpResponseCode);
   OnLine = false;
  }
  // Free resources
  http.end();
 }
 else 
 {
  Serial.println("WiFi Disconnected");
  OnLine = false;
 }
  //  lastTime = millis();

}

//======================================================================================================================
//===== Zadost o OffLine data ze serveru                                                                           =====
//======================================================================================================================
void WiFiGetOffLine()
{
 if (WiFi.status() == WL_CONNECTED)
 {
  HTTPClient http;
  
  String htmlStr;
  
  //strcpy(htmlStr, ServerAdr);
  
  // Time request
  htmlStr = ServerAdr; 
  htmlStr += "webapi.php?uid=zw&password=morava&command=ACCESSLIST&data=<xml><readerid>";
  htmlStr += readerID;
  htmlStr += "</readerid></xml>";

  // Osoby request
  //http://192.168.0.1/dochazka/webapi.php?uid=zw&password=morava&command=ACCESSLIST&data=<xml><readerid>1</readerid></xml>
  //htmlStr = ServerAdr; 
  //htmlStr += "webapi.php?uid=zw&password=morava&command=ACCESSLIST&data=<xml><readerid>1</readerid></xml>";
  

  // Your Domain name with URL path or IP address with path
  http.begin(htmlStr.c_str());
      
  // Send HTTP GET request
  int httpResponseCode = http.GET();
      
  if (httpResponseCode > 0) {
   Serial.print("HTTP Response code: ");
   Serial.println(httpResponseCode);
   String ResponseStr = http.getString();
   //Serial.println(ResponseStr);   
  
   String Cip;
   unsigned long int found = ResponseStr.indexOf("<status>");
   unsigned long int foundEnd; 
   if (found > 0)
   {
    if ((ResponseStr[found + 8] == 'O') && (ResponseStr[found + 9] == 'K'))
    {
     // Serial.println(String("Status: ") + ResponseStr[found + 8] + ResponseStr[found + 9]);  
     ResponseStr = ResponseStr.substring(found + 19, ResponseStr.length());
     


     preferencesWiFi.begin("Cipy", false);
     unsigned int Pozice = 0;
     String PoziceStr;
     do
     {
      found = ResponseStr.indexOf("<cip>");
      foundEnd = ResponseStr.indexOf("</cip>");
      if (found > 0) 
      {
       Cip = ResponseStr.substring(found + 5, foundEnd);
       //Serial.print(Pozice);
       //Serial.print(": ");
       //Serial.println(Cip.c_str());
       preferencesWiFi.putString(String(Pozice).c_str(), Cip);
       Pozice++;
       ResponseStr = ResponseStr.substring(foundEnd + 6, ResponseStr.length()); 
      }
     } while (found < foundEnd - 1);
     preferencesWiFi.putInt("CipyPocet", Pozice - 1);
     preferencesWiFi.end();
     
     Serial.print("Zapsano cipu: ");
     Serial.println(Pozice - 1);

     //Serial.println(ResponseStr);
    }
    
   }
   
  }
  else 
  {
   Serial.print("Error code: ");
   Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
 }
 else 
 {
  Serial.println("WiFi Disconnected");
 }
  //  lastTime = millis();

}


//======================================================================================================================
//===== Zadost o nove zaznamy ze serveru                                                                           =====
//======================================================================================================================
void WiFiGetNewRecord(byte Resend)
{
 if (WiFi.status() == WL_CONNECTED)
 {
  HTTPClient http;
  
  String htmlStr;
  
  //strcpy(htmlStr, ServerAdr);
  
  // Time request
  htmlStr = ServerAdr; 
  htmlStr += "door-lock.php?readerID=";
  htmlStr += readerID;
  htmlStr += "&init=1";
  if (Resend == 1) htmlStr += "&resend=1";


  Serial.println(htmlStr.c_str()); 
  // Zaznamy request
  // https://door-lock-test.isporttest.cz/api/door-lock.php?readerID=01&init=1

  // Your Domain name with URL path or IP address with path
  http.begin(htmlStr.c_str());
      
  // Send HTTP GET request
  int httpResponseCode = http.GET();
      
  if (httpResponseCode > 0) 
  {
   Serial.print("HTTP Response code: ");
   Serial.println(httpResponseCode);
   String ResponseStr = http.getString();
   Serial.println(ResponseStr);   

   
   DynamicJsonDocument JsonDoc(8192);

   deserializeJson(JsonDoc, ResponseStr.c_str());
   //JsonObject JsonObj = JsonDoc.as<JsonObject>();
    
   String State = JsonDoc["status"]; 
   Serial.println(State.c_str());
   int error = JsonDoc["error"]; 
   Serial.println(error);
   String errorMessage = JsonDoc["errorMessage"]; 
   Serial.println(errorMessage);
 
   

   if ((State[0] == 'O') && (State[1] == 'K') && (error == 0))
   {
    OnLine = true;
    unsigned int i = 0;
    struct tm timeinfo;
    String Str;
    do
    {
     if (JsonDoc["records"][i]["pin"].as<String>() != "null")
     { 
      Str = JsonDoc["records"][i]["time_from"].as<String>();
      Str = Str.substring(17, 19);
      timeinfo.tm_sec = Str.toInt();
      
      Str = JsonDoc["records"][i]["time_from"].as<String>();
      Str = Str.substring(14, 16);
      timeinfo.tm_min = Str.toInt();
      
      Str = JsonDoc["records"][i]["time_from"].as<String>();
      Str = Str.substring(11, 13);
      timeinfo.tm_hour = Str.toInt();
      
      Str = JsonDoc["records"][i]["time_from"].as<String>();
      Str = Str.substring(8, 10);
      timeinfo.tm_mday = Str.toInt();
      
      Str = JsonDoc["records"][i]["time_from"].as<String>();
      Str = Str.substring(5, 7);
      timeinfo.tm_mon = Str.toInt() - 1;
      
      Str = JsonDoc["records"][i]["time_from"].as<String>();
      Str = Str.substring(0, 4);
      timeinfo.tm_year = Str.toInt() - 1900;
      
      long int TimeIn = mktime(&timeinfo);
      Serial.print("Time start: ");
      Serial.println(TimeIn);




      Str = JsonDoc["records"][i]["time_to"].as<String>();
      Str = Str.substring(17, 19);
      timeinfo.tm_sec = Str.toInt();
      
      Str = JsonDoc["records"][i]["time_to"].as<String>();
      Str = Str.substring(14, 16);
      timeinfo.tm_min = Str.toInt();
      
      Str = JsonDoc["records"][i]["time_to"].as<String>();
      Str = Str.substring(11, 13);
      timeinfo.tm_hour = Str.toInt();
      
      Str = JsonDoc["records"][i]["time_to"].as<String>();
      Str = Str.substring(8, 10);
      timeinfo.tm_mday = Str.toInt();
      
      Str = JsonDoc["records"][i]["time_to"].as<String>();
      Str = Str.substring(5, 7);
      timeinfo.tm_mon = Str.toInt() - 1;
      
      Str = JsonDoc["records"][i]["time_to"].as<String>();
      Str = Str.substring(0, 4);
      timeinfo.tm_year = Str.toInt() - 1900;
      
      long int TimeTo = mktime(&timeinfo);
      Serial.print("Time stop: ");
      Serial.println(TimeTo);
      
      CommandTable[i].Command = JsonDoc["records"][i]["command"].as<String>();
      CommandTable[i].Pin = JsonDoc["records"][i]["pin"].as<String>();
      CommandTable[i].TimeFrom = String(TimeIn);
      CommandTable[i].TimeTo = String(TimeTo); 
     
      Serial.println(CommandTable[i].Command.c_str());
      Serial.println(CommandTable[i].Pin.c_str());
      Serial.println(CommandTable[i].TimeFrom.c_str());
      Serial.println(CommandTable[i].TimeTo.c_str());
      Serial.println("---------------------");
      i++;
     }
     else break; 

    } while (JsonDoc["records"][i]["pin"].as<String>() != "null");
    // Pokud je nejaky zaznam tak ho zapis nebo smaz z FLASH
    if (i > 0) WriteToFlash(i);
   }
   else OnLine = false;
 
   
  }
  else 
  {
   OnLine = false; 
   Serial.print("Error code: ");
   Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
 }
 else 
 {
  Serial.println("WiFi Disconnected");
 }
 
}


//======================================================================================================================
//===== Ulozeni nebo smazani zaznamu z FLASH                                                                       =====
//======================================================================================================================
void WriteToFlash(int Pocet)
{
 preferencesWiFi.begin("Rezervace", false);
 
 String StrPin, StrData;
 for (unsigned int i = 0; i <= Pocet; i++)
 {
  // Pridani noveho pinu
  if (CommandTable[i].Command == "insert")
  {
   StrPin = CommandTable[i].Pin;  
   StrData = "I:" + CommandTable[i].TimeFrom + ";O:" + CommandTable[i].TimeTo + ";";
   preferencesWiFi.putString(StrPin.c_str(), StrData.c_str());
  }

  // Smazani stareho pinu
  if (CommandTable[i].Command == "delete")
  {
   if (preferencesWiFi.isKey(CommandTable[i].Pin.c_str()))
   {
    preferencesWiFi.remove(CommandTable[i].Pin.c_str());
    Serial.print("Mazu pin: ");
    Serial.println(CommandTable[i].Pin.c_str());

   }
  }
 }
 preferencesWiFi.end();
 // Odesli odpoved na zapis a mazani FLASH
 WiFiSendReply(Pocet);

}


//======================================================================================================================
//===== Odeslani cipu na server                                                                                    =====
//======================================================================================================================
void WiFiSendCip()
{
 if (WiFi.status() == WL_CONNECTED)
 {
  HTTPClient http;
  
  
  struct tm timeinfo;
  
 
  char DateStrCip[14]; 
  if(!getLocalTime(&timeinfo)) Serial.println("Failed to obtain time 1");
  strftime(DateStrCip, sizeof(DateStrCip), "T%y%m%d%H%M%S", &timeinfo);

  String htmlStr;
  htmlStr = ServerAdr; 
  htmlStr += "c_action.php?readerID=";
  htmlStr += readerID;
  htmlStr += "&actionID=";
  htmlStr += ActionID;
  htmlStr += "&data=";
  htmlStr += (char*)CipHEXAscii;
  htmlStr += "&timestamp=";
  htmlStr += DateStrCip;
  htmlStr += "&rs=01";
  
  //String htmlStr = ServerAdr + "c_action.php?readerID=" + TermAdr + "&actionID=" + ActionIDCip + "&data=" + (char*)CipHEXAscii + "&timestamp=" + DateStrCip + "&rs=01";
  // Your Domain name with URL path or IP address with path
  
  http.begin(htmlStr.c_str());
      
  // Send HTTP GET request
  int httpResponseCode = http.GET();
      
  if (httpResponseCode > 0) {
   Serial.print("HTTP Response code: ");
   Serial.println(httpResponseCode);
   String ResponseStr = http.getString();
   Serial.println(ResponseStr);

   unsigned long int found = ResponseStr.indexOf("OW3: ");

   if ((found > 0) && (found < ResponseStr.length()))
   {
    ResponseStr = ResponseStr.substring(found + 6, found + 6 + 16);
    strcpy(ROW3, ResponseStr.c_str());
    Serial.println(ROW3);
   }
  }
  else 
  {
   Serial.print("Error code: ");
   Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
 }
 else 
 {
  Serial.println("WiFi Disconnected");
 }
  //  lastTime = millis();
 
}


//======================================================================================================================
//===== Kontrola OffLine cipu v pameti FLASH                                                                       =====
//======================================================================================================================
void CheckOffLine()
{
 preferencesWiFi.begin("Cipy", true);
 unsigned int Pozice = 0, i;
 unsigned int PoziceEnd = preferencesWiFi.getInt("CipyPocet");
 String FlashCip;

 //Serial.print("Reader Short Cip: ");
 //Serial.println((char*)CipHEXAscii);
 
 strcpy(ROW3, "0000000000000000");
 do
 {
  FlashCip = preferencesWiFi.getString(String(Pozice).c_str());
  if ((FlashCip[0] == CipHEXAscii[2]) && (FlashCip[1] == CipHEXAscii[3]) && (FlashCip[2] == CipHEXAscii[4]) && (FlashCip[3] == CipHEXAscii[5]) &&
     (FlashCip[4] == CipHEXAscii[6]) && (FlashCip[5] == CipHEXAscii[7]) && (FlashCip[6] == CipHEXAscii[8]) && (FlashCip[7] == CipHEXAscii[9]))
  {
   Pozice = 65500;
   ROW3[RelePosition - 1] = OffLineReleTime + 48;
  
   Serial.print("FlashCip: ");
   Serial.println(FlashCip);
   Serial.println(ROW3);
  } 
  Pozice++; 
 } while ((Pozice < PoziceEnd - 1) && ( Pozice < 5000));
 preferencesWiFi.end();
}
 

//======================================================================================================================
//===== Kontrola pinu v pameti FLASH                                                                               =====
//======================================================================================================================
void CheckPin()
{
 preferencesCheckPin.begin("Rezervace", true);


 unsigned int Pin;
 Pin = String(CipHEXAscii).toInt();
 String Str;
 // Smazani predesleho casu otevreni dveri
 ROW3[RelePosition - 1] = 48;

 // Pridani nul
 if (Pin > 999) Str = String(Pin);
 if ((Pin <= 999) && (Pin > 99)) Str = '0' + String(Pin);
 if ((Pin <= 99) && (Pin > 9)) Str + "00" = String(Pin);
 if (Pin <= 9) Str = "000" + String(Pin);
 
 // Kontrola jestli pin existuje
 if (preferencesCheckPin.isKey(Str.c_str()))
 {
  Serial.print("Pin nalezen: ");
  Serial.println(Str.c_str());

  // Kontrola casu opravneni
  time_t now;
  unsigned long int EpochNow = time(&now);
  unsigned long int TimeIn = preferencesCheckPin.getString(Str.c_str()).substring(2, 12).toInt();
  unsigned long int TimeOut = preferencesCheckPin.getString(Str.c_str()).substring(15, 25).toInt();
  
  Serial.print("TimeNow: ");
  Serial.println(EpochNow);


  Serial.print("TimeIn: ");
  Serial.println(TimeIn);

  Serial.print("TimeOut: ");
  Serial.println(TimeOut);



  if ((TimeIn < EpochNow) && (TimeOut > EpochNow))
  {
   Serial.print("Otevri....");
   ROW3[RelePosition - 1] = OffLineReleTime + 48;
  }
  
 }
 else
 {
  Serial.print("Pin nenalezen: ");
  Serial.println(Str.c_str());
 }
 
 



 preferencesCheckPin.end();
}


//======================================================================================================================
//===== Odpoved na zapis pozadavku do FLASH                                                                        =====
//======================================================================================================================
void WiFiSendReply(unsigned int Pocet)
{
 if (WiFi.status() == WL_CONNECTED)
 {
  HTTPClient http;
  
  String htmlStr;
  
  // Done request
  htmlStr = ServerAdr; 
  htmlStr += "door-lock.php?readerID=";
  htmlStr += readerID;
  htmlStr += "&done=" + String(Pocet);

  Serial.println(htmlStr.c_str()); 
  // Zaznamy request
  // https://door-lock-test.isporttest.cz/api/door-lock.php?readerID=01&init=1

  // Your Domain name with URL path or IP address with path
  http.begin(htmlStr.c_str());
      
  // Send HTTP GET request
  int httpResponseCode = http.GET();
      
  if (httpResponseCode > 0) {
   Serial.print("HTTP Response code: ");
   Serial.println(httpResponseCode);
   String ResponseStr = http.getString();
   Serial.println(ResponseStr);   


   DynamicJsonDocument JsonDoc(1024);

   deserializeJson(JsonDoc, ResponseStr.c_str());
   //JsonObject JsonObj = JsonDoc.as<JsonObject>();
    
   String State = JsonDoc["status"]; 
   Serial.println(State.c_str());
   int error = JsonDoc["error"]; 
   Serial.println(error);
   String errorMessage = JsonDoc["errorMessage"]; 
   Serial.println(errorMessage);
   
  }
  else 
  {
   Serial.print("Error code: ");
   Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
 }
 else 
 {
  Serial.println("WiFi Disconnected");
 }
 
}


//======================================================================================================================
//===== Udrzba FLASH, smazani starych pinu                                                                         =====
//======================================================================================================================
void DeleteOldPin()
{
 // Kontrola jestli je Cas OK
 if (TimeOK)
 {
  // Nacteni aktualniho casu
  time_t now;
  unsigned long int EpochNow = time(&now);
  
  // Prohledani ulozenych PINu
  Serial.println("Mazani starych pinu z FLASH:"); 
  preferencesCheckPin.begin("Rezervace", false);
 
  unsigned int Pocet = 0, i; 
  char PinStr[4];
  for (i = 0; i < 10000; i++)
  {
   sprintf(PinStr, "%04d", i);  
   

   if (preferencesCheckPin.isKey(PinStr))
   {
    unsigned long int TimeOut = preferencesCheckPin.getString(String(PinStr).c_str()).substring(15, 25).toInt();
    // Nalezen stary pin, smaz ho
    if (EpochNow > TimeOut)
    {
     Serial.print("Nalezen stary PIN: ");
     Serial.print(PinStr);
     Serial.print(" DATA: ");
     Serial.println(preferencesCheckPin.getString(PinStr));
     preferencesCheckPin.remove(PinStr);
     Pocet++;
    }
   }
  }
  Serial.print("Pocet smazanych pinu: ");
  Serial.println(Pocet);
  preferencesCheckPin.end();
 }


}
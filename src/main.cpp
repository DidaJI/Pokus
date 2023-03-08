//**********************************************************************************************************************
//******           i T O U CH   K E Y B O A R D   W i F i   E S P 3 2 - S 3   -  W i F i   Č T E Č K A            ******
//******            Klávesnicová kombinovaná čtečka (EMARINE, Mifare/Desfire, Dallas, Wiegand) s WiFi             ******
//******                 ESP32, EM4095, SL032, Zelená a červená ikona, RGB nápis a čtečková LED                   ******
//**********************************************************************************************************************
//****** Verze : 1.02                                                                                             ******
//****** Poslední úpravy : 15.12.2022                                                     napsal : Dráždil Zdeněk ******
//****** Verze desky 1.0                                                                                          ******
//**********************************************************************************************************************

// Includes ************************************************************************************************************
#include <Arduino.h>
#include <time.h>
#include <string.h>
#include <esp_task_wdt.h>
#include <Preferences.h>
#include <nvs_flash.h>

// Pridana poznamka...

#include "main.h"
//#include "em4095.h"
#include "wificomm.h"

// 3 seconds WDT
#define WDT_TIMEOUT 15

// Definice tasku
TaskHandle_t TaskPin;
TaskHandle_t TaskWiFi;


hw_timer_t * timer1ms = NULL;
portMUX_TYPE timerMux1ms = portMUX_INITIALIZER_UNLOCKED;

/* Nastaveni PWM pro beeper */
unsigned int PWMFreq = 1; /* 1 Hz */
const int PWMChannel1 = 0;
const int PWMResolution = 10;

Preferences preferences;

// loop smycky jednotlivych tasku
void TaskPinCode(void * parameter);
void TaskWiFiCode(void * parameter);

void SetRGBLED(unsigned char R, unsigned char G, unsigned char B);
void SetSRGBLED(unsigned char R, unsigned char G, unsigned char B);
void InitTime();
void SetTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst);
void SetPWM(unsigned int PWM, unsigned int Delka);

void IRAM_ATTR IntRow_1(); 
void IRAM_ATTR IntRow_2(); 
void IRAM_ATTR IntRow_3(); 
void IRAM_ATTR IntRow_4(); 
void DecodeKey(unsigned char Row, unsigned char Col); 

// Private variables ***************************************************************************************************
unsigned int TimeOutBlockRead, TimeOutBeep, TimeOutRGBCtecka, TimeOutSemafor, TimeOutRGB, MifareTimeOut, TimeOutRele, TimeOutReboot;
unsigned int TimeOutRuseniLed, TimeOutRuseni, TimeOutLive, ACK, PN532TimeOut, TimeOutMaster;
unsigned int TimeOutWiFiConnect, TimeOutColl, TimeOutKey, TimeOutCheckDeleteTime, LastKeyTimeOut;
unsigned long int TimeOutGetOffLine, TimeOutGetTime, TimeOutGetNewRecord;
unsigned char TXBuff2[30], PN532Buff[20], MifareTXBuff[20], MifareRXBuff[30], RuseniProc, MifareLongCT, RXIndexCT, RX1Char, KeybColl, KeyPosition;
char KeyStr[5];
char ROW3[17];
char LastKey;
String ActionIDCip, DateStrCip, Serial2RXBuff;
boolean MifareRecCT, MifareCipRes, OnLine, WiFiStatus, BlockKey, FreeCol, TimeOK;



// Obslouzeni preruseni ************************************************************************************************
// Obslouzeni preruseni od casovace kazdou 1ms
void IRAM_ATTR onTimer1ms() 
{
  portENTER_CRITICAL_ISR(&timerMux1ms);
  if (TimeOutBlockRead > 0) TimeOutBlockRead--;
  if (TimeOutMaster > 0) TimeOutMaster--;
  if (TimeOutBeep == 1) SetPWM(0, 0);
  if (TimeOutBeep > 0) TimeOutBeep--;
  if (TimeOutRGBCtecka > 0) TimeOutRGBCtecka--;
  if (MifareTimeOut > 0) MifareTimeOut--;
  if (TimeOutRGB > 0) TimeOutRGB--;
  if (TimeOutSemafor > 0) TimeOutSemafor--;
  if (TimeOutRuseniLed > 0) TimeOutRuseniLed--;
  if (TimeOutRuseni > 0) TimeOutRuseni--;
  if (TimeOutLive > 0) TimeOutLive--;
  else TimeOutLive = 2000;	
  if (PN532TimeOut > 0) PN532TimeOut--;
  if (TimeOutGetTime > 0) TimeOutGetTime--; 
  if (TimeOutGetNewRecord > 0) TimeOutGetNewRecord--; 
  if (TimeOutRele > 0) TimeOutRele--;
  if (TimeOutWiFiConnect > 0) TimeOutWiFiConnect--;
  if (TimeOutColl > 0) TimeOutColl--;
  if (TimeOutKey > 0) TimeOutKey--;
  if (TimeOutCheckDeleteTime > 0) TimeOutCheckDeleteTime--;
   
  if (LastKeyTimeOut > 0) LastKeyTimeOut--;
  if (TimeOutReboot > 0) TimeOutReboot--; 
  
  if (TimeOutGetOffLine > 0) TimeOutGetOffLine--; 
 
  portEXIT_CRITICAL_ISR(&timerMux1ms);
}


// Program Setup *******************************************************************************************************
void setup() 
{

 Serial.begin(115200); // Debug a programovani
  
 if (ReaderType == 1) Serial1.begin(115200, SERIAL_8N1, 10, 11);  // Externi cteci moduly
  
 pinMode(LiveLED, OUTPUT);
 pinMode(DebugLED, OUTPUT);
 pinMode(RuseniLED, OUTPUT);
 
 pinMode(RLED, OUTPUT);
 pinMode(GLED, OUTPUT);
 pinMode(BLED, OUTPUT);
 
 pinMode(SRLED, OUTPUT);
 pinMode(SGLED, OUTPUT);
 pinMode(SBLED, OUTPUT);

 pinMode(OpenLED, OUTPUT);
 pinMode(CloseLED, OUTPUT);

 pinMode(Beep, OUTPUT);
 pinMode(Rele, OUTPUT);
 
 pinMode(MASTER, INPUT);
 pinMode(SLAVE, INPUT);
 
 pinMode(MASTER, INPUT_PULLUP);

 // Klavesnice
 if (ReaderType == 6)
 {
  pinMode(KeybCol_1, OUTPUT);
  pinMode(KeybCol_2, OUTPUT);
  pinMode(KeybCol_3, OUTPUT);
  digitalWrite(KeybCol_1, HIGH);
  digitalWrite(KeybCol_2, HIGH);
  digitalWrite(KeybCol_3, HIGH);
  
  pinMode(KeybRow_1, INPUT_PULLUP);
  pinMode(KeybRow_2, INPUT_PULLUP);
  pinMode(KeybRow_3, INPUT_PULLUP);
  pinMode(KeybRow_4, INPUT_PULLUP);

  // Definice preruseni klavesnicovych radku
  attachInterrupt(KeybRow_1, IntRow_1, FALLING);
  attachInterrupt(KeybRow_2, IntRow_2, FALLING);
  attachInterrupt(KeybRow_3, IntRow_3, FALLING);
  attachInterrupt(KeybRow_4, IntRow_4, FALLING);

 }
 
 // nastaveni preddelicky na Timeru_1 na /80 = 1MHz a na 1000 tiku = 1ms
 timer1ms = timerBegin(1, 80, true);
 timerAttachInterrupt(timer1ms, &onTimer1ms, true);
 timerAlarmWrite(timer1ms, 1000, true);
 timerAlarmEnable(timer1ms);

 // PWM 1 BEEPER
 ledcSetup(PWMChannel1, PWMFreq, PWMResolution);
 ledcAttachPin(Beep, PWMChannel1);
 SetPWM(0, 0);

 // Parametry(Jmeno obsluzne funkce, Jmeno tasku z definice TaskHandle_t, Velikost STACK, Vstupni parametr, Priorita 0 = nejnizsi, Jmeno, Core ID ktere se ma pro task pouzit) 
 xTaskCreatePinnedToCore(TaskPinCode, "TaskPin", 10000, NULL, 1, &TaskPin, 1);                         
 // Parametry(Jmeno obsluzne funkce, Jmeno tasku z definice TaskHandle_t, Velikost STACK, Vstupni parametr, Priorita 0 = nejnizsi, Jmeno, Core ID ktere se ma pro task pouzit) 
 xTaskCreatePinnedToCore(TaskWiFiCode, "TaskWiFi", 10000, NULL, 1, &TaskWiFi, 0);          
 
 // Initialize Variables ************************************************************************************************ 
 //AutoTest();

 digitalWrite(RLED, LOW);
 digitalWrite(OpenLED, LOW);
 digitalWrite(CloseLED, LOW);
 digitalWrite(Rele, LOW);
 SetSRGBLED(0, 0, 0);
 SetRGBLED(1, 0, 0); 
  
 RecCip0 = false;
 KeybColl = 1;
 TimeOK = false;
 LastKey = '!';
 TimeOutReboot = 61000;
   
 // EM4095
 if ((ReaderType == 0) && (ReaderType == 4));
 {	 
  //Povol interni ctecku
  EMInit();
  digitalWrite(SHD, LOW);
 }
 


 // Inicializace PN532 modulu
 // if (ReaderType == 1) PN532Init();
 
 MifareCipRes = false;
 OnLine = false;
 WiFiStatus = false;
 
 Serial.println("WiFi Reader start..."); 

 /*
 // Nastaveni casu
 struct tm tm;
 tm.tm_year = 2022 - 1900;   // Set date
 tm.tm_mon = 6-1;
 tm.tm_mday = 25;
 tm.tm_hour = 9;      // Set time
 tm.tm_min = 0;
 tm.tm_sec = 0;
 tm.tm_isdst = 1;  // 1 or 0 Letni a zimni cas
 time_t t = mktime(&tm);
 Serial.printf("Setting time: %s", asctime(&tm));
 struct timeval now = { .tv_sec = t };
 settimeofday(&now, NULL);
 */
 vTaskSuspend(TaskWiFi);
 vTaskSuspend(TaskPin);

 // Rucni smazani pameti pinu
 //nvs_flash_erase(); // erase the NVS partition and...
 //nvs_flash_init();  // initialize the NVS partition.
 //Serial.println("Pamet smazana...");




 // Test jestli je zformatovana FLASH a kolik je v ni Offline zaznamu
 preferences.begin("Rezervace", false);
 if (!preferences.isKey("RezervacePocet"))
 { 
  preferences.end();
  Serial.println("Formating FLASH");
  nvs_flash_erase(); // erase the NVS partition and...
  nvs_flash_init();  // initialize the NVS partition.
  
  preferences.begin("Rezervace", false);
  Serial.println("Create Key RezervacePocet = 0");
  preferences.putInt("RezervacePocet", 0);
 }
 else 
 {
  Serial.println("FLASH OK");
 }

 preferences.end();
 



 /*
 // Zapis testovacich pinu do pameti
 Serial.println("Zapisuji testovaci piny"); 
 preferences.begin("Rezervace", false);
 preferences.clear();
 String StrPin, StrData;
 for (unsigned int j = 0; j < 1100; j++)
 {
  if (j < 9) StrPin = "000" + String(j);  
  if ((j > 9) && (j < 99)) StrPin = "00" + String(j); 
  if ((j > 99) && (j < 999)) StrPin = "0" + String(j);   
  if (j > 999) StrPin = String(j);   
      
  StrData = "I:000000" + StrPin + ";O:000000" + StrPin + ";";
  Serial.println(StrPin.c_str());
  preferences.putString(StrPin.c_str(), StrData.c_str());
 }
 preferences.end();
 Serial.println("Konec zapisu...");
 */




 if (WriteFlashPin)
 {
  // Vypis ulozenych PINu
  Serial.println("Rezervace ve FLASH: "); 
  preferences.begin("Rezervace", true);
 
  unsigned int Pocet = 0; 
  char PinStr[4];
  for (unsigned int j = 0; j < 10000; j++)
  {
   sprintf(PinStr, "%04d", j);  
      
   if (preferences.isKey(PinStr))
   {
    Serial.print("PIN: ");
    Serial.print(PinStr);
    Serial.print(" DATA: ");
    Serial.println(preferences.getString(PinStr));
    Pocet++;
   }
  }
  Serial.print("Pocet rezervaci: ");
  Serial.println(Pocet);
  preferences.end();


 } 
 
 SetPWM(2850, 1000);

 BlockKey = true;
 TimeOutKey = 200;
 KeyPosition = 0;
 
 esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
 esp_task_wdt_add(TaskPin); //add current thread to WDT watch
 esp_task_wdt_reset();

 vTaskResume(TaskWiFi);
 vTaskResume(TaskPin);

}

   


//======================================================================================================================
//===== Pin task                                                                                                   =====
//======================================================================================================================
void TaskPinCode(void * parameter) 
{
 Serial.println("Startuji TaskPinCode..."); 

 for(;;)
 { 
  esp_task_wdt_reset();

  // Obsluha vstupniho tlacitka
  if (digitalRead(SLAVE) == LOW) {digitalWrite(Rele, HIGH); TimeOutRele = 4000;} 
    
  // Obsluha klavesnicovych sloupcu
  if ((TimeOutColl == 0) && (BlockKey == false))
  {
   TimeOutColl = 20;
   if (KeybColl < 3) KeybColl++;
   else KeybColl = 1; 
   switch (KeybColl)
   {
    case 1: digitalWrite(KeybCol_1, LOW); digitalWrite(KeybCol_2, HIGH); digitalWrite(KeybCol_3, HIGH); break;
    case 2: digitalWrite(KeybCol_1, HIGH); digitalWrite(KeybCol_2, LOW); digitalWrite(KeybCol_3, HIGH); break;
    case 3: digitalWrite(KeybCol_1, HIGH); digitalWrite(KeybCol_2, HIGH); digitalWrite(KeybCol_3, LOW); break;
   }
  }
  // Cekani na uvolneni klavesy
  if ((!digitalRead(KeybRow_1)) || (!digitalRead(KeybRow_2)) || (!digitalRead(KeybRow_3)) || (!digitalRead(KeybRow_4))) TimeOutKey = 50;
  if (TimeOutKey == 0) BlockKey = false;
 
 
  // Ochrana proti rychlemu stisku stejne klavesy
  if (LastKeyTimeOut == 0) LastKey = ';';
  
  // Pokud byl nacten cip
  if (RecCip0 == true)
  {
   RecCip0 = false;	
   
   if (TimeOutBlockRead == 0)
   {  
    TimeOutBlockRead = 1300;
    SetPWM(2850, 100);
    
    if ((StickType == 0) || (StickType == 3) || (StickType == 4))
    {	
     SetRGBLED(1, 0, 0);
     TimeOutRGBCtecka = 100;
    } 
    if ((StickType == 1) || (StickType == 2))
    {	
     SetRGBLED(0, 0, 1);
     //digitalWrite(LogoLED, HIGH);
     //digitalWrite(NapiLED, HIGH);
   
     TimeOutRGBCtecka = 100;
    }
    
     // Kontrola jestli to neni default PIN
     //unsigned int Pin;
     //Pin = String(CipHEXAscii).toInt();
     //if (Pin == 1234) ROW3[RelePosition - 1] = OffLineReleTime + 48;
     //else CheckPin();

     CheckPin(); 


    //CheckOffLine();

    // Otevri pokud je v ROW3 powel   
    if (ROW3[RelePosition - 1] > 48)
    {
     TimeOutRele = (ROW3[RelePosition - 1] - 48) * 100;
     SetPWM(2850, 400);
     
     if ((StickType == 0) || (StickType == 3) || (StickType == 4))
     {	
      SetRGBLED(0, 1, 0);
      TimeOutRGBCtecka = TimeOutBlockRead;
      TimeOutSemafor = TimeOutRele;
     }
    }
    else
    {
     SetPWM(2850, 50); vTaskDelay(150);
     SetPWM(2850, 50); vTaskDelay(150);
     SetPWM(2850, 50); 
    }

    // Odeslani cipu    
    //Serial2.println((char*)CipHEXAscii);

    // Odeslani cipu do debugu 
    //Serial.println((char*)CipHEXAscii);
    
          
   }
  }
  
 
  // Navraceni led do vychoziho stavu
  if ((TimeOutRGBCtecka == 0) && (StickType == 1))
  {
   //SetRGBLED(0, 1, 0); 
   //digitalWrite(LogoLED, HIGH);
   //digitalWrite(NapiLED, HIGH);
   //digitalWrite(ReadLED, HIGH);
  } 
  // Navraceni led do vychoziho stavu
  if ((TimeOutRGBCtecka == 0) && (StickType == 2))
  {
   //SetRGBLED(1, 0, 0); 
   //digitalWrite(LogoLED, HIGH);
   //digitalWrite(NapiLED, HIGH);
   //digitalWrite(ReadLED, HIGH);
  }

  // Obsluha RGB cteckove LED
  if (TimeOutRGBCtecka == 0)
  {
   if ((WiFiStatus) && (OnLine) && (TimeOK)) SetRGBLED(0, 0, 1);
   if ((WiFiStatus) && (OnLine) && (!TimeOK)) SetRGBLED(1, 0, 1);
   if ((WiFiStatus) && (!OnLine)) SetRGBLED(1, 1, 0);
   if ((!WiFiStatus) && (!OnLine)) SetRGBLED(1, 0, 0);
   if ((!WiFiStatus) && (OnLine)) SetRGBLED(1, 0, 0);
  }

  // Obsluha LED zamku semaforu
  if (TimeOutSemafor > 0) {digitalWrite(OpenLED, HIGH); digitalWrite(CloseLED, LOW);}
  else {digitalWrite(OpenLED, LOW); digitalWrite(CloseLED, HIGH);}
  
  // Obsluha Ruseni LED
  if (TimeOutRuseniLed == 0) digitalWrite(RuseniLED, LOW);
  else digitalWrite(RuseniLED, HIGH);
  
  // Obsluha Live LED
  if (TimeOutLive > 1000) digitalWrite(LiveLED, HIGH);
  else digitalWrite(LiveLED, LOW);

  // Obsluha Rele
  if (TimeOutRele > 0) digitalWrite(Rele, HIGH);
  else digitalWrite(Rele, LOW);

  // Restart v nastavenou dobu


  // Smazani starych pinu
  if (TimeOutCheckDeleteTime == 0)
  {
   TimeOutCheckDeleteTime = 10000;
   struct tm timeinfo;
   if(getLocalTime(&timeinfo))
   {
    char Hod[3], Min[3];
    strftime(Hod, 3, "%H", &timeinfo);
    strftime(Min, 3, "%M", &timeinfo);
    if ((Hod[0] == RemoveOldPinTime[0]) && (Hod[1] == RemoveOldPinTime[1]) && (Min[0] == RemoveOldPinTime[3]) && (Min[1] == RemoveOldPinTime[4]))  
    {
     TimeOutCheckDeleteTime = 60000;
     vTaskSuspend(TaskWiFi);
     DeleteOldPin();
     vTaskResume(TaskWiFi);
    }
   }
  }

  // Reboot v nastaveny cas
  if (TimeOutReboot == 0)
  {
   TimeOutReboot = 30000;
   struct tm timeinfo;
   getLocalTime(&timeinfo);
   char TimeStr[6];
   strftime(TimeStr, sizeof(TimeStr), "%H:%M", &timeinfo);
   if ((TimeStr[0] == RebootTime[0]) && (TimeStr[1] == RebootTime[1]) && (TimeStr[3] == RebootTime[3]) && (TimeStr[4] == RebootTime[4])) ESP.restart();
  }

  



  vTaskDelay(1);
  
 } 
}

//======================================================================================================================
//===== WiFi task                                                                                                  =====
//======================================================================================================================
void TaskWiFiCode(void * parameter)
{
 Serial.println("Startuji TaskWiFiCode..."); 
 for(;;)
 { 
  // Pripojeni k WiFi pokud neni aktivni
  if ((WiFi.status() != WL_CONNECTED) && (TimeOutWiFiConnect == 0)) 
  {
   WiFiConnect();
   portENTER_CRITICAL(&timerMux1ms);
    TimeOutWiFiConnect = TimeWiFiConncet;
   portEXIT_CRITICAL(&timerMux1ms);
  }
  
  if (WiFi.status() == WL_CONNECTED) WiFiStatus = true;
  else WiFiStatus = false;
 
  // Zadost o cas z NTP serveru
  if ((WiFi.status() == WL_CONNECTED) && (TimeOutGetTime == 0)) 
  {
   InitTime();
   // Precti cas z RTC
   Serial.print("Cas v RTC: ");
   struct tm timeinfo;
   if(!getLocalTime(&timeinfo)) Serial.println("Failed to obtain time 1");
   else TimeOK = true;
   Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S zone %Z %z ");

   if (TimeOK)
   {
    portENTER_CRITICAL(&timerMux1ms);
     TimeOutGetTime = TimeGetTime * 1000;
    portEXIT_CRITICAL(&timerMux1ms);
   } 
   else
   {
    portENTER_CRITICAL(&timerMux1ms);
     TimeOutGetTime = 5000;
    portEXIT_CRITICAL(&timerMux1ms);
   } 

   //Serial.print("Now Epoch time: ");
   //time_t now;
   //Serial.println(time(&now));
  }
 

  // Zadost o nove zaznamy ze serveru
  if ((WiFi.status() == WL_CONNECTED) && (TimeOutGetNewRecord == 0) && (TimeOK)) 
  {
   portENTER_CRITICAL(&timerMux1ms); 
    TimeOutGetNewRecord = TimeGetNewRecord * 1000; 
   portEXIT_CRITICAL(&timerMux1ms); 
   WiFiGetNewRecord(0);
  } 

  // Zadost o OffLine osoby ze serveru
  //if ((WiFi.status() == WL_CONNECTED) && (TimeOutGetOffLine == 0) && (OnLine)) {TimeOutGetOffLine = TimeGetOffLine * 1000 * 60; WiFiGetOffLine();}
  
  vTaskDelay(100);
 }
}

// hlavni smycka
void loop()
{
 // Uplne zakaze task smycky loop 
 vTaskDelete(NULL);
}


//======================================================================================================================
//===== Ovladani cteckove LED                                                                                      =====
//======================================================================================================================
void SetRGBLED(unsigned char R, unsigned char G, unsigned char B)
{ 
 if (R == 1) digitalWrite(RLED, HIGH);
 else digitalWrite(RLED, LOW);
 
 if (G == 1) digitalWrite(GLED, HIGH);
 else digitalWrite(GLED, LOW);

 if (B == 1) digitalWrite(BLED, HIGH);
 else digitalWrite(BLED, LOW);
}

//======================================================================================================================
//===== Ovladani napisových LED                                                                                    =====
//======================================================================================================================
void SetSRGBLED(unsigned char R, unsigned char G, unsigned char B)
{ 
 if (R == 1) digitalWrite(SRLED, HIGH);
 else digitalWrite(SRLED, LOW);
 
 if (G == 1) digitalWrite(SGLED, HIGH);
 else digitalWrite(SGLED, LOW);

 if (B == 1) digitalWrite(SBLED, HIGH);
 else digitalWrite(SBLED, LOW);
}

//======================================================================================================================
//===== Init TIME                                                                                                  =====
//======================================================================================================================
void InitTime()
{ 
 struct tm timeinfo;

 Serial.println("Nastavuji cas z NTP");
 configTime(0, 0, "pool.ntp.org");    // First connect to NTP server, with 0 TZ offset
 if(!getLocalTime(&timeinfo)){
   Serial.println("Cas nelze nacist");
   return;
 }
 Serial.println("Ziskan cas z NTP");
 // Now we can set the real timezone
 Serial.println("Zmena pasma na Berlin");
 String TimeZone = "CET-1CEST,M3.5.0,M10.5.0/3";
 setenv("TZ",TimeZone.c_str(), 1);  //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
 tzset();
}


//======================================================================================================================
//===== Nastaveni casu ve vnitrnim RTC                                                                             =====
//======================================================================================================================
void SetTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst)
{
  struct tm tm;

  tm.tm_year = yr - 1900;   // Set date
  tm.tm_mon = month-1;
  tm.tm_mday = mday;
  tm.tm_hour = hr;      // Set time
  tm.tm_min = minute;
  tm.tm_sec = sec;
  tm.tm_isdst = isDst;  // 1 or 0
  time_t t = mktime(&tm);
  Serial.printf("Setting time: %s", asctime(&tm));
  struct timeval now = { .tv_sec = t };
  settimeofday(&now, NULL);
}




//======================================================================================================================
//===== Obslouzeni preruseni od radku klavesnice                                                                   =====
//======================================================================================================================
void IRAM_ATTR IntRow_1() 
{
 if (!BlockKey)
 {
  BlockKey = true;
  DecodeKey(KeybColl, 1);
 } 
}

void IRAM_ATTR IntRow_2() 
{
 if (!BlockKey)
 {
  BlockKey = true;  
  DecodeKey(KeybColl, 2);
 }
}

void IRAM_ATTR IntRow_3() 
{
 if (!BlockKey)
 { 
  BlockKey = true;
  DecodeKey(KeybColl, 3);
 }
}

void IRAM_ATTR IntRow_4() 
{
 if (!BlockKey)
 { 
  BlockKey = true;
  DecodeKey(KeybColl, 4);
 }
}

//======================================================================================================================
//===== Dekodovani stisknute klavesy                                                                               =====
//======================================================================================================================
void DecodeKey(unsigned char Col, unsigned char Row) 
{
 char Key;
 
 if ((Col == 1) && (Row == 1)) Key = '*';
 if ((Col == 1) && (Row == 2)) Key = '7';
 if ((Col == 1) && (Row == 3)) Key = '4';
 if ((Col == 1) && (Row == 4)) Key = '1';

 if ((Col == 2) && (Row == 1)) Key = '0';
 if ((Col == 2) && (Row == 2)) Key = '8';
 if ((Col == 2) && (Row == 3)) Key = '5';
 if ((Col == 2) && (Row == 4)) Key = '2';

 if ((Col == 3) && (Row == 1)) Key = '#';
 if ((Col == 3) && (Row == 2)) Key = '9';
 if ((Col == 3) && (Row == 3)) Key = '6';
 if ((Col == 3) && (Row == 4)) Key = '3';
 
 // Ochrana proti dvojstisku stejne klavesy
 if (LastKey == Key) Key = '!';

 if (Key != '!')
 {
  SetPWM(2850, 50);
  Serial.println(Key);
  LastKey = Key;
  LastKeyTimeOut = 100;
 }
 
 if (Key > 47) 
 {
  KeyStr[KeyPosition] = Key;
  if (KeyPosition < 3) KeyPosition++;
  else
  {
   KeyPosition = 0;
  }
 }

 if (Key == '#')
 { 
  KeyPosition = 0;
  for (i = 0; i < 10; i++) CipHEXAscii[i] = '0';
  CipHEXAscii[6] = KeyStr[0];
  CipHEXAscii[7] = KeyStr[1];
  CipHEXAscii[8] = KeyStr[2];
  CipHEXAscii[9] = KeyStr[3];
  KeyStr[0] = '0'; KeyStr[1] = '0'; KeyStr[2] = '0'; KeyStr[3] = '0';
  RecCip0 = true;
 }

 if (Key == '*')
 { 
  KeyPosition = 0;
  KeyStr[0] = '0'; KeyStr[1] = '0'; KeyStr[2] = '0'; KeyStr[3] = '0';
  SetPWM(2850, 800);
 }

}


//======================================================================================================================
//===== Nastaveni PWM pro ovladani beeperu                                                                         =====
//======================================================================================================================
void SetPWM(unsigned int PWM, unsigned int Delka)
{
 TimeOutBeep = Delka;
 if (PWM > 0)
 {
  ledcSetup(PWMChannel1, PWM, PWMResolution);
  ledcWrite(PWMChannel1, 512);
 }
 else
 {
  ledcSetup(PWMChannel1, 1, PWMResolution);
  ledcWrite(PWMChannel1, 0);
 } 
}	   
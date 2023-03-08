#ifndef MAIN_H
#define MAIN_H

//**********************************************************************************************************************
//******                                 D E F I N I C E   K O M P I L A C E                                      ******
//**********************************************************************************************************************
#define Version "1.01"                                                                                                  // Aktuální verze FW 
#define ReaderType 6                                                                                                    // Typ připojené čtečky, 0 = interní Emarine, 1 = MIFARE 9600, 2 = Dallas, 3 = Wiegand, 4 = Combo Mifare&EMaine, 5 = XT-CARD, 6 = klávesnice  
#define StickType 0 			                                                                                        // Typ samolepky, 0 = pouze přístup, 1 = příchod nápis, 2 = odchod nápis, 3 = Šipky, 4 = Šipky zámky 
#define Direction 0                                                                                                     // 0 = prichod leva, 1 = prichod prava, 2 = odchod leva, 3 = odchod prava
#define DesfireType 0											                                                        // 0 = Rotace byte podle Balcara, 1 = Standardní rotace byte		
#define AlarmMode 0                                                                                                     // Ctecky signalizuji ovladani alarmu nebo trvalé zapínání výtahu, 0 = vypnuto, 1 = zapnuto
#define KeybType 1                                                                                                      // Volba typu klávesnice, 0 = stará černá tlačítková, 1 = membránová

#define WriteFlashPin 1                                                                                                 // Vypis aktualnich pinu ve flash po restartu zarizeni 

//#define Myssid "TP-Link_CFE6"                                                                                                 // Login do WiFi 
//#define Mypassword "14565205"                                                                                                    // Heslo do WiFi     

//#define Myssid "U Splavu_zahrada"                                                                                        // Login do WiFi 
//#define Mypassword "Usplavu987"                                                                                         // Heslo do WiFi     
//#define Myssid "CIPY"                                                                                                 // Login do WiFi 
//#define Mypassword "Pr1p0jSe"                                                                                         // Heslo do WiFi     

//#define Myssid "tenis"                                                                                                 // Login do WiFi 
//#define Mypassword "PlanaNL159"                                                                                         // Heslo do WiFi     

#define Myssid "Z-WARE"                                                                                                 // Login do WiFi 
#define Mypassword "Jana2108"                                                                                         // Heslo do WiFi     

// ZS Stepankovice
//#define Myssid "Wifi-Ucitel"                                                                                                 // Login do WiFi 
//#define Mypassword "!@3c1v0kn4p3tS"                                                                                         // Heslo do WiFi     


//#define Myssid "Expreska"                                                                                                 // Login do WiFi 
//#define Mypassword "ex.preska2017"                                                                                         // Heslo do WiFi     


//#define Myssid "Black Shark"                                                                                                 // Login do WiFi 
//#define Mypassword "Tomas9408164920"                                                                                         // Heslo do WiFi     


#define TimeWiFiConncet 5000                                                                                            // Cas v ms jak casto se snazi ctecka znovu pripojit WiFi
//#define ServerAdr "http://213.226.227.230/dochazka/"
//https://sportovistevm.isportsystem.cz/api/door-lock.php?init=1&readerID=01
//#define ServerAdr "https://door-lock-test.isporttest.cz/api/"                                                          // Adresa kde jsou scripty
//#define ServerAdr "https://puklice.isportsystem.cz/api/"                                                             // Adresa kde jsou scripty
//#define ServerAdr "https://penzionumachu.isportsystem.cz/api/"
//#define ServerAdr "https://penzionusplavu.isportsystem.cz/api/"
//#define ServerAdr "https://dolany-kladno.isportsystem.cz/api/"
//#define ServerAdr "https://fitzenyuhrineves.isportsystem.cz/api/"
//#define ServerAdr "https://dobronin.isportsystem.cz/api/" 
//#define ServerAdr "https://sportovisteplana.isportsystem.cz/api/"

//#define ServerAdr "https://stepankovice.isportsystem.cz/api/door-lock.php?readerID=01&init=1"
//#define ServerAdr "https://stepankovice.isportsystem.cz/api/door-lock.php?readerID=02&init=1"
      #define ServerAdr "https://zamek-test.isporttest.cz/api/door-lock.php?readerID=01&init=1"

#define readerID "01"                                                                                                   // ID terminalu
#define ActionID "41"                                                                                                   // ActionID ctecky 
#define RelePosition 1                                                                                                  // Pozice rele v ROW3 povelu
#define OffLineReleTime 40                                                                                              // Cas v desetinach s jak dlouho se spina rele v OffLine
#define TimeGetTime 60                                                                                                  // Cas v s jak casto se ctecka dotazuje na cas
#define TimeGetNewRecord 30                                                                                             // Cas v s jak casto se ctecka dotazuje na nove zaznamy
#define TimeGetOffLine 100                                                                                              // Cas v min jak casto se ctecka dotazuje na OffLine zaznamy pro otevirani dveri
#define RemoveOldPinTime "03:00"                                                                                        // Cas v kolik se provadi udrzba PINu 
#define RebootTime "04:00"                                                                                              // Cas v kolik se ma provest automaticky reset (Zadanim nesmyslneho casu se reset vypne) 


// Private Define ******************************************************************************************************
#define LiveLED 6 
#define DebugLED 5 
#define RuseniLED 4

#define RLED 41
#define GLED 1
#define BLED 2

#define SRLED 37
#define SGLED 38
#define SBLED 39


#define OpenLED 15
#define CloseLED 7 
#define Beep 40
#define Rele 35

#define MASTER 8 
#define SLAVE 19

// Stara tlacitkova klavesnice
#if KeybType == 0
 #define KeybCol_1 47 
 #define KeybCol_2 45
 #define KeybCol_3 14

 #define KeybRow_1 21
 #define KeybRow_2 13
 #define KeybRow_3 12
 #define KeybRow_4 48
#endif

// Nova membranova klavesnice
#if KeybType == 1 
 #define KeybCol_1 47 
 #define KeybCol_2 48
 #define KeybCol_3 45

 #define KeybRow_1 21
 #define KeybRow_2 14
 #define KeybRow_3 13
 #define KeybRow_4 12
#endif

#endif // MAIN_H


// Extern variables ***************************************************************************************************
extern boolean OnLine, TimeOK;
extern char ROW3[17];
void SetTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst); 

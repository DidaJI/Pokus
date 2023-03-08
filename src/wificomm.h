#ifndef wificomm_H
#define wificomm_H

#include <Arduino.h>
#include <main.h>
#include <em4095.h>
#include <WiFi.h>
#include <HTTPClient.h>




void WiFiConnect();
void WiFiGetTime();
void WiFiGetState();
void WiFiGetNewRecord(byte Resend);
void WiFiGetOffLine();
void WiFiSendCip();
void CheckOffLine();
void WriteToFlash(int Pocet);
void CheckPin();
void WiFiSendReply(unsigned int Pocet);
void DeleteOldPin();



#endif
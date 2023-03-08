//**********************************************************************************************************************
//******                                   M Y   E M 4 0 9 5   L I B R A R Y                                      ******
//******                               Moje knihovna pro čtení z EM4095 s ESP32                                   ******
//**********************************************************************************************************************

#include <Arduino.h>
#include "em4095.h"
#include "main.h"

unsigned char RFIDStr[55], CipHEX[12], CipHEXLast[12];
char CipHEXAscii[12];
unsigned char RuseniAnt = 0, Ruseni = 0;
unsigned char ReadTag, StartBit, Bit, LastBit, BitTmp, start_bit_count = 0, seq_bit_no, Parita = 0, i;
unsigned int bit_time_cnts = 0, RuseniTime = 0;
unsigned long int CteckaACK;
bool RecCip0;

unsigned char Parity(void);

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

//void IRAM_ATTR onTimer();
//void IRAM_ATTR MyINT_1();


// Obslouzeni preruseni od casovace kazdych 10 us
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  
  bit_time_cnts++;
 
  // Detekce ruseni z anteny
  RuseniTime++;
  if (RuseniTime >= 2000)
  {
   RuseniTime = 0;
   RuseniAnt = Ruseni;
   Ruseni = 0;
  } 

  portEXIT_CRITICAL_ISR(&timerMux);
}




// Obslouzeni preruseni od EM4095 PIN - DEMOD 
void IRAM_ATTR MyINT_1() 
{
 
 // this ist the interrupt routine which starts working with every bit change on the EXTINT input
 // when 9* 1 in a row are received a new RFID tag is on its way to be decoded
	
 //if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6) == 0) GPIOD -> BSRR = GPIO_BSRR_BS4;
 //if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6) == 1) GPIOD -> BSRR = GPIO_BSRR_BR4;
 
 // V pripade interni ctecky
 if ((ReaderType == 0) || (ReaderType == 4))
 {	
  // Detekce ruseni z anteny
  if (Ruseni < 254) Ruseni++;
  
  // Jestli je prijaty byte delsi nez 150uS a uplynul TimeOut predesleho cteni
  if ((bit_time_cnts > 15) && (CteckaACK == 0)) //15  //10
  {	
   LastBit = Bit;
   Bit = digitalRead(DEMOD);		
  	
   if (((Bit == 0) && ((bit_time_cnts > 43) && (bit_time_cnts < 63))) || (ReadTag)) // 45, 67   //40,75
   { 	
    // Detekce 9ti bitu 1
    if ((start_bit_count > 0) && (seq_bit_no == 0))
    {	
     if ((StartBit == 1) && ((bit_time_cnts > 17) && (bit_time_cnts < 37)))   //15, 35   //10, 40
     {
      if (Bit == 1) start_bit_count++;
     }		
     else 
     {
      ReadTag = 0;
      StartBit = 0;
      start_bit_count = 0;
     }
    }	
    else if (seq_bit_no == 0)
    {
     start_bit_count++; StartBit = 1; ReadTag = 1;
    }		
     
   
   
    // Cteni bitu karty
    if (seq_bit_no > 0)
    {
     // Reverzace bitu
     if (bit_time_cnts > 35) //30 //35
     {
      BitTmp = !BitTmp;
      RFIDStr[seq_bit_no] = BitTmp;
      seq_bit_no--;
     }
     else 
     {	
      if (Bit == 1)
      {
       RFIDStr[seq_bit_no] = BitTmp;
       seq_bit_no--;
      }	 
     }
         
     if (seq_bit_no == 0)
     {
      //Preved Bite na Byte
      CipHEX[0] = RFIDStr[54] * 8 + RFIDStr[53] * 4 + RFIDStr[52] * 2 + RFIDStr[51];
      CipHEX[1] = RFIDStr[49] * 8 + RFIDStr[48] * 4 + RFIDStr[47] * 2 + RFIDStr[46];
      CipHEX[2] = RFIDStr[44] * 8 + RFIDStr[43] * 4 + RFIDStr[42] * 2 + RFIDStr[41];
      CipHEX[3] = RFIDStr[39] * 8 + RFIDStr[38] * 4 + RFIDStr[37] * 2 + RFIDStr[36];
      CipHEX[4] = RFIDStr[34] * 8 + RFIDStr[33] * 4 + RFIDStr[32] * 2 + RFIDStr[31];
      CipHEX[5] = RFIDStr[29] * 8 + RFIDStr[28] * 4 + RFIDStr[27] * 2 + RFIDStr[26];
      CipHEX[6] = RFIDStr[24] * 8 + RFIDStr[23] * 4 + RFIDStr[22] * 2 + RFIDStr[21];
      CipHEX[7] = RFIDStr[19] * 8 + RFIDStr[18] * 4 + RFIDStr[17] * 2 + RFIDStr[16];
      CipHEX[8] = RFIDStr[14] * 8 + RFIDStr[13] * 4 + RFIDStr[12] * 2 + RFIDStr[11];
      CipHEX[9] = RFIDStr[9] * 8 + RFIDStr[8] * 4 + RFIDStr[7] * 2 + RFIDStr[6];
      CipHEX[10] = RFIDStr[4] * 8 + RFIDStr[3] * 4 + RFIDStr[2] * 2 + RFIDStr[1];
      
      Parita = Parity();
      
       
      // Nacteni cipu 2x za sebou
      if ((Parita == 1) && (CipHEX[0] == CipHEXLast[0]) && (CipHEX[1] == CipHEXLast[1]) && (CipHEX[2] == CipHEXLast[2]) && (CipHEX[3] == CipHEXLast[3])
         && (CipHEX[4] == CipHEXLast[4]) && (CipHEX[5] == CipHEXLast[5]) && (CipHEX[6] == CipHEXLast[6]) && (CipHEX[7] == CipHEXLast[7])
      	  && (CipHEX[8] == CipHEXLast[8]) && (CipHEX[9] == CipHEXLast[9]))
      {
       Parita = 1;
      }	  
      else Parita = 0;
      
     
      for (i = 0; i < 10; i++)
      {
       CipHEXLast[i] = CipHEX[i];
      }
         
      // Kontrola karty na FF
      if ((CipHEX[0] == 0x0F) && (CipHEX[1] == 0x0F) && (CipHEX[2] == 0x0F) && (CipHEX[3] == 0x0F) && (CipHEX[4] == 0x0F) &&
         (CipHEX[5] == 0x0F) && (CipHEX[6] == 0x0F) && (CipHEX[7] == 0x0F) && (CipHEX[8] == 0x0F) && (CipHEX[9] == 0x0F)) Parita = 0;
      // Kontrola karty na 00
      if ((CipHEX[0] == 0) && (CipHEX[1] == 0) && (CipHEX[2] == 0) && (CipHEX[3] == 0) && (CipHEX[4] == 0) &&
         (CipHEX[5] == 0) && (CipHEX[6] == 0) && (CipHEX[7] == 0) && (CipHEX[8] == 0) && (CipHEX[9] == 0)) Parita = 0;
    
      
      //if (Parita == 1)
     // {
       	
      //}	   
      
      if (Parita == 1)
      {
       // Prevod na ASCII HEX
       unsigned char i;
       for (i = 0; i < 10; i++)
       {
        if (CipHEX[i] > 9)
        {
         switch (CipHEX[i])  
         {
          case 10: CipHEXAscii[i] = 'A'; break;
  		    case 11: CipHEXAscii[i] = 'B'; break;
  		  	 case 12: CipHEXAscii[i] = 'C'; break;
  		  	 case 13: CipHEXAscii[i] = 'D'; break;
  			 case 14: CipHEXAscii[i] = 'E'; break;
  			 case 15: CipHEXAscii[i] = 'F';
         }
        } 	
        else CipHEXAscii[i] = CipHEX[i] + 48;
       }
       Parita = 0;
       RecCip0 = true;
      }
     }
    }	
   }
   else 
   {
 	  ReadTag = 0;	
 	  StartBit = 0;
 	  start_bit_count = 0;
   }	
  } 
 	
   // Zjsiten cip (prislo 9 jednicek)	
   if (start_bit_count == 9)
   {
    BitTmp = 1;
    start_bit_count = 0;
    seq_bit_no = 54;
   }	
  // Nuluj timer delky pulzu
  bit_time_cnts = 0;	
  // enable new change	
 } 
   
 
}


// Inicializace knihovny pro cteni Emarine cipu
void EMInit() {
 // nastaveni preddelicky na /80 = 1MHz a na 10 tiku = 10us
 timer = timerBegin(0, 80, true);
 timerAttachInterrupt(timer, &onTimer, true);
 timerAlarmWrite(timer, 10, true);
 timerAlarmEnable(timer);

 // Definice pinu DEMOD
 pinMode(DEMOD, INPUT);
 attachInterrupt(DEMOD, MyINT_1, CHANGE);

 pinMode(SHD, OUTPUT);
 pinMode(CLK, INPUT);

 //extern unsigned char RuseniAnt, Ruseni;


}



/*

void init_extint(void)
{
	// configure ext interrupt function on pin PA0
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);

	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	//EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	//EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; // Musi mit vyssi prioritu nez timer nize
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_Init(&NVIC_InitStructure);
	
}


void init_timer(void)
{
	// SystemCoreClock = 72MHz -> APB1 is maximum 36MHz for TIM2,3,4,5 but will be doubled again to 72MHz when prescaler # 0
	// This example uses TIM2 Timer
	// APB1 clock = SystemCoreClock/2 but s.o.

	TIM_TimeBaseInitTypeDef TIM_TimeBase_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBase_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;

	TIM_TimeBase_InitStructure.TIM_Period 			= 9;		// count up to 999 = 1000
	TIM_TimeBase_InitStructure.TIM_Prescaler		= 23;		//72(MHz) -1 Divided by 72 Bylo taky 71
	TIM_TimeBase_InitStructure.TIM_CounterMode 		= TIM_CounterMode_Up;
	// 9,71 = 10us Cycle Time
	// 999,71 = 1ms
	// 1MHz / 500 = 2000 Hz.
	// SystemCoreClock = 72MHz / 72 => 1 MHz Clock with this clock the timer counts up to 999 = 1000 counts =
	// 1/1000 --> 1000 Hz = 1ms cycle time

	TIM_TimeBaseInit(TIM2, &TIM_TimeBase_InitStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
	NVIC_Init(&NVIC_InitStructure);
	TIM_Cmd(TIM2, ENABLE);
}
*/


// Kontrola parity
unsigned char Parity(void)
{
 unsigned char i, j = 0, k, Cislo = 0, Pass = 1;
 
 //ROW 
 for (i = 54; i > 4; i--)
 {
  Cislo += RFIDStr[i];
    
  j++;
  if (j == 5) 
  {
   j = 0;
   if ((Cislo == 1) || (Cislo == 3) || (Cislo == 5)) Pass = 0;
   Cislo = 0;	
  } 
 }
 
 //Column 
 Cislo = 0; j = 0; 
 for (k = 1; k < 5; k++)
 {
  Cislo = 0; j = 0;
  for (i = 11; i > 0; i--)
  {
   Cislo += RFIDStr[(i * 5) - k];
     
   j++;
   if (j == 11) 
   {
    j = 0;
    if ((Cislo == 1) || (Cislo == 3) || (Cislo == 5) || (Cislo == 7) || (Cislo == 9) || (Cislo == 11)) Pass = 0;
    Cislo = 0;	
   } 
  }
 } 
 return Pass;
} 


 
 
// This is the IU LIGHT 1.0 code.
#include <StaticThreadController.h>
#include <ThreadController.h>
#include <Thread.h>
#include <assert.h>
#include <LPD8806.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Encoder.h>

int nLEDs = 16;
int maxNum = pow(2, nLEDs);
int clockPin = 23;
long clockData = -999;

int repTL_data = 2;
int spltMtoRTR_data = 3;
int repTR_data = 4;
int spltBLtoMem_data = 5;
int ARegtoMux_data = 6;
int jl_data = 7;
int mem_data = 8;
int pc_data = 9;
int mux_data = 10;
int DReg_data = 29;
int alu_data = 30;
int ARegtoMem_data = 23;
int spltBLtoA_data = 22;
int spltMtoD_data = 21;
int repBR_data = 20;

LPD8806 repTL = LPD8806(nLEDs, repTL_data, clockPin);
LPD8806 spltMtoRTR = LPD8806(nLEDs, spltMtoRTR_data, clockPin);
LPD8806 repTR = LPD8806(nLEDs, repTR_data, clockPin);
LPD8806 spltBLtoMem = LPD8806(nLEDs, spltBLtoMem_data, clockPin);
LPD8806 ARegtoMux = LPD8806(nLEDs, ARegtoMux_data, clockPin);
LPD8806 jl = LPD8806(nLEDs, jl_data, clockPin);
LPD8806 mem = LPD8806(nLEDs, mem_data, clockPin);
LPD8806 pc = LPD8806(nLEDs, pc_data, clockPin);
LPD8806 mux = LPD8806(nLEDs, mux_data, clockPin);
LPD8806 DReg = LPD8806(nLEDs, DReg_data, clockPin);
LPD8806 alu = LPD8806(nLEDs, alu_data, clockPin);
LPD8806 ARegtoMem = LPD8806(nLEDs, ARegtoMem_data, clockPin);
LPD8806 spltBLtoA = LPD8806(nLEDs, spltBLtoA_data, clockPin);
LPD8806 spltMtoD = LPD8806(nLEDs, spltMtoD_data, clockPin);
LPD8806 repBR = LPD8806(nLEDs, repBR_data, clockPin);

Encoder enc(34, 33);

void setup()
{
  Serial.begin(9600);
}



void loop()
{
  setClockSpeed();
  // need to test the outputs of the rot and make a function to map to a delay.
}



void spltMtoD_(int num)
{
  int i;
  // create some kind of logic to decide between repTR or Dreg
  for(i = 0; i < nLEDs; i++)
  {
    if(bitRead(num, i) == 1)
    {
      spltMtoD.setPixelColor(i, spltMtoD.Color(127, 0, 0));
    }
    else
    {
      spltMtoD.setPixelColor(i, 0);
    }
  }
  spltMtoD.show();
}



void out_repTL(int num)
{
  int i;
  for(i = 0; i < nLEDs; i++)
  {
    if(bitRead(num, i) == 1)
    {
      repTL.setPixelColor(i, repTL.Color(127, 127, 127));
    }
    else
    {
      repTL.setPixelColor(i, 0);
    }
  }
  repTL.show();
}

void out_repTR(int num)
{
  int i;
  for(i = 0; i < nLEDs; i++)
  {
    if(bitRead(num, i) == 1)
    {
      repTR.setPixelColor(i, repTR.Color(127, 127, 127));
    }
    else
    {
      repTR.setPixelColor(i, 0);
    }
  }
  repTR.show();
}

int binaryOut(int a, int b, int op)
{
  switch (op)
  {
    case 0:
      return a + b;
      break;
    case 1:
      return a - b;
      break;
    case 2:
      return a * b;
      break;
    case 3:
      return a / b;
      break;
  }
}


// This needs to be ported

//uint16_t registerD(uint16_t registerIn, int registerLoad){
//  uint16_t registerOut;
//  if(registerLoad == true) 
//  {
//    registerOut = registerIn;
//    previousRegisterValue = registerIn;
//  }
//  else 
//  {
//    registerOut = previousRegisterValue;
//  }
//  return registerOut;
//}



void setClockSpeed()
{
  long newClock = enc.read();
  
  if (clockData != newClock) 
  {
    if (newClock <= 5000 && newClock >= 0)
    {
      clockData = newClock;
    }
  }
  Serial.print(clockData);
}

// still needs to be ported
//uint16_t registerA(uint16_t registerALUIn, int registerLoad, uint16_t registerROMin){
//  uint16_t registerOut;
//  
//  int yep = 0xE000 == (0xE000 & registerROMIn); //left-shift the value
//  uint16_t yup = first_three_bits == (registerROMIn & first_three_bits);
//  if(yep == 1) 
//  { //if the first three digits of the rom are 111
//    registerOut = registerROMIn; //set the registerOut and previousRegisterValue to equal RegisterROMIn
//    previousRegisterValue = registerROMIn;
//  }
//  else if(registerLoad == true) 
//  { //if the registerLoad value is true
//    registerOut = registerALUIn; //set registerOut and previousRegisterValue to equal RegisterALUIn
//    previousRegisterValue = registerALUIn;
//  }
//  else 
//  {
//    registerOut = previousRegisterValue; //otherwise, the previousRegisterValue does not change
//  }
//  return registerOut;
//}


// unfinished and needs to be ported
//uint16_t ALU(uint16_t a, uint16_t b, int computeBits[6]){
//  
//  
//  int aluComputeBits[18] = {0b101010, 0b111111,
//                            0b111010, 0b001100,
//                            0b110000, 0b001101,
//                            0b110001, 0b001111,
//                            0b110011, 0b011111,
//                            0b110111, 0b001110,
//                            0b110010, 0b000010,
//                            0b010011, 0b000111,
//                            0b000000, 0b010101};
//
//  int computeBitInt = arrayToInt(computeBits);
//  int aluComputeOut[18] = {0 , 1, -1, a, b, ~a, ~b, -a, -b, a+1, b+1, a-1, b-1, a+b, a-b, b-a, a&b, a|b};
//
//  for(int i = 0; i<18; i++)
//  {
//    if(aluComputeBits[i] == computeBitInt)
//    {
//      return aluComputeOut[i];
//    }  
//  }
//}

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
#include <SD.h>

int nLEDs = 16;
int maxNum = pow(2, nLEDs);
int clockPin = 23;
long clockData = -999;

int repTL_data = 2;
int spltMtoRTL_data = 3;
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

//Made this one 31 for now, don't know it's actual value so it's going to need to be set!!!
int ARegtoPC = 31;

LPD8806 repTL = LPD8806(nLEDs, repTL_data, clockPin);
LPD8806 spltMtoRTL = LPD8806(nLEDs, spltMtoRTR_data, clockPin);
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

//THIS NEEDS INITIALIZATION, I DON'T KNOW THE LED/PIN NUMBER FOR IT
LPD8806 ARegtoPC = LPD8806(nLEDs, ARegtoPC, clockPin);

//Very important note! Most significant bit of instruction is stored in index 0 of instruction
//Example: Most significant bit of instruction number 0 (the first one in the file so it's index 0) 
//is stored at instructions[0][0], and least significant bit is stored at instruction[0][15]
int instructions[16][16];
int instrIndex = 0;

//Think we need some kind of memory array, not certain though. Gonna make it 64 values long for now
int memory[64][16];

//Think we need arrays to hold values of A and D registers as well
int DReg[16];
int AReg[15];

Encoder enc(34, 33);
                           
void setup()
{
  Serial.begin(9600);
  //STEP 1: Get instructions from file. Gonna start by opening the file from the sd card
  File myFile;
  int x = 0, y = 0;
  char c;
  myFile = SD.open("instructions.txt");
                           
  if (myFile) {
    Serial.println("instructions.txt:");
    
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
    	c = myFile.read();
      //if we hit a newline, move a column down in our instruction 2-D array
      if(c == '\n')
        //move down a column
        y++;
        //make sure we start at beginning of next instruction
        x = 0;
        //skip to next iteration of loop
        continue;
      //if we didn't hit newline, then read in char from the file's instruction, convert to int, and store in
      //our instructions array
      instructions[y][x] = (c-48);
      x++;
    }
    // close the file:
    myFile.close();
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening instructions.txt");
  }
  
  //Now the file stuff is done
}



void loop()
{
  int i;
  setClockSpeed();
  // need to test the outputs of the rot and make a function to map to a delay.
  //Once we've gone past the end of our instruction set, start over!
  if(instrIndex == 16)
    instrIndex == 0;
  
  //If it's an a-instruction load the A register
  if(instructions[instrIndex][0] == 0)
  {
     for(i = 1; i < 16; i++)
     {
        AReg[i] = instructions[instrIndex][i];
     }
  }
  //If it's a c-instruction
  if(instructions[instrIndex][0] == 1)
  {
     //Do c-instruction stuff here
  }
  //After each loop we need to increment instrIndex so we can loop over the same instructions again
  instrIndex++;
}


//Controls output LEDs for top middle splitter. Goes to D if D is in the destination bits given in our instruction
//Takes in same data as our top right repeater, and also takes in an instruction which will tell us whether or not
//To send from splitter to D
void spltMtoD(int outALU[16], int instruction[16])
{
  int i;
  if(instruction[11] == 1)
  {
     for(i = 0; i < nLEDs; i++)
     {
       if(outALU[i] == 1)
       {
         spltMtoD.setPixelColor(i, spltMtoD.Color(255, 0, 0));
       }
       else
       {
         spltMtoD.setPixelColor(i, 0);
       }
     }
  }
  else
  {
     for(i = 0; i < nLEDs; i++)
     {
        spltMtoD.setPixelColor(i, 0);
     }
     
  }
  spltMtoD.show();
}

//Controls output LEDs for top middle splitter. Goes to top left repeater if A or M are in the destination bits given 
//in our instruction. Takes in same data as our top right repeater, and also takes in an instruction which 
//will tell us whether or not to send from splitter to top left repeater
void spltMtoRTL(int outALU[16], int instruction[16])
{
  int i;
  if(instruction[10] == 1 || instruction[12] == 1)
  {
     for(i = 0; i < nLEDs; i++)
     {
       if(outALU[i] == 1)
       {
         spltMtoRTL.setPixelColor(i, spltMtoRTL.Color(255, 0, 0));
       }
       else
       {
         spltMtoRTL.setPixelColor(i, 0);
       }
     }
  }
  else
  {
     for(i = 0; i < nLEDs; i++)
     {
        spltMtoRTL.setPixelColor(i, 0);
     }
     
  }
  spltMtoRTL.show();
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

void out_AReg(int ainstr[16])
{
  int i;
  
  ARegtoMux.setPixelColor(0, 0);
  ARegtoMem.setPixelColor(0, 0);
  ARegtoPC.setPixelColor(0,0);
  for(i = 1; i < nLEDs; i++)
  {
    if(ainstr[i] == 1)
    {
      ARegtoMux.setPixelColor(i, ARegtoMux.Color(255, 0, 0));
      ARegtoMem.setPixelColor(i, ARegtoMem.Color(255, 0, 0));
      ARegtoPC.setPixelColor(i, ARegtoPC.Color(255, 0, 0));
    }
    else
    {
      ARegtoMux.setPixelColor(i, 0);
      ARegtoMem.setPixelColor(i, 0);
      ARegtoPC.setPixelColor(i,0);
    }
  }
  ARegtoMux.show();
  ARegtoMem.show();
  ARegtoPC.show();
}

void out_DReg()
{
   int i;
   for(i = 0; i < 15; i++)
   {
      if(DReg[i] == 1)
      {
         DReg.setPixelColor(i, DReg.Color(255, 0, 0));
      }
     else
     {
        DReg.setPixelColor(i, 0);
     }
   }
   DReg.show();
}

//Repeaters just spit back out whatever is put in
void out_repBR(int outALU[16])
{
   int i;
   for(i = 0; i < 16; i++)
   {
      if(outALU[i] == 1)
      {
         repBR.setPixelColor(i, repBR.Color(255, 0, 0));
      }
     else
     {
        repBR.setPixelColor(i, 0);
     }
   }
   repBR.show();
}

//Same as above but just with top right repeater. Even has the same data as the bottom right
//so we'll give it the same input
void out_repTR(int outALU[16])
{
   int i;
   for(i = 0; i < 16; i++)
   {
      if(outALU[i] == 1)
      {
         repTR.setPixelColor(i, repTR.Color(255, 0, 0));
      }
     else
     {
        repTR.setPixelColor(i, 0);
     }
   }
   repTR.show();
}

//Test function, not actually needed
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

//Takes in 3 least significant bits of our instruction (which control jump) and if any are true
//Then we turn on all the jump logic LED's
unsigned int jumpLogicOut(int instruction[16])
{
   //If we're going to make some kind of jump, light up all of the 16 leds attached to jump logic
   if(instruction[15] == 1 || instruction[14] == 1 || instruction[13] == 1)
   {
      for(i = 0; i < nLEDs; i++)
      {
         jl.setPixelColor(i, jl.Color(255, 0, 0));
         
      }
   }
   else
      jl.setPixelColor(i, jl.Color(0,0,0));
  
  jl.show();
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

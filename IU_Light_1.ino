// This is the latest IU LIGHT code.
#include <math.h>
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

LPD8806 repTL = LPD8806(nLEDs, repTL_data, clockPin);
LPD8806 spltMtoRTL = LPD8806(nLEDs, spltMtoRTL_data, clockPin);
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

//Very important note! Most significant bit of instruction is stored in index 0 of instruction
//Example: Most significant bit of instruction number 0 (the first one in the file so it's index 0) 
//is stored at instructions[0][0], and least significant bit is stored at instruction[0][15]
int instructions[16][16];
int instrIndex = 0;

//Think we need some kind of memory array, not certain though. Gonna make it 64 values long for now
int memory[64][16];

//Think we need arrays to hold values of A and D registers as well
//Requires a rename
int DReg_val[16];
int AReg_val[16];

//Going to create a global variable for this instead of dealing with malloc/free later
int outALU[16];

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
    instrIndex = 0;
  
  //If it's an a-instruction load the A register
  if(instructions[instrIndex][0] == 0)
  {
     AReg_val[0] = 0;
     for(i = 1; i < 16; i++)
     {
        // AReg[i] = instructions[instrIndex][i]; // AReg not in scope, did you mean AReg_value?
        AReg_val[i] = instructions[instrIndex][i];
     }
  }
  //If it's a c-instruction
  if(instructions[instrIndex][0] == 1)
  {
     spltMtoD_(instructions[instrIndex]);
     spltMtoRTL_(instructions[instrIndex]);
     out_repTL();
     out_AReg(AReg_val);
     out_DReg();
     out_repBR();
     out_repTR();
     spltBLtoMem_(instructions[instrIndex]);
     spltBLtoA_(instructions[instrIndex]);
     jumpLogicOut(instructions[instrIndex]);
     outMem();
     ALU_out(mux_(instructions[instrIndex]), instructions[instrIndex]);
     
  }
  //After each loop we need to increment instrIndex so we can loop over the same instructions again
  instrIndex++;
}

//Function that takes in 16 length int array and negates all of the elements
void negate16Bit(int *input)
{
   int i;
   for(i = 0; i < 16; i++)
   {
      input[i] = !(input[i]);
   }
}

//Handles binary addition associated with increment (stuff like carrying)
void inc16Bit(int *input)
{
   int i;
   if(input[15] == 0)
   {
      input[15] = 1;
      return;
   }
   //If our least significant bit isn't a zero we have to do some binary addition
   else
   {
      //Start by zeroeing the least significant bit
      input[15] = 0;
      for(i = 14; i > 0; i--)
      {
         //Flip 1's to 0's until we reach a 0. Then flip that to a 1 and exit.
         if(input[i] == 0)
         {
            input[i] = 1;
            return;
         }
         //If the current bit isn't a zero, flip it from 1 to zero and move on
         else
         {
            input[i] = 0;
         }//close else
      }//close for
   }//close else
}//close inc16Bit

//Handles twos compliment, to be used in ALU computations
void twosComp16Bit(int *input)
{
   int i;
   negate16Bit(input);
   inc16Bit(input);
}//close twosComp16Bit

//Does bitwise addition of two binary numbers from int arrays
void bitWiseAdd(int *out, int *in1, int *in2)
{
   int i, c = 0;
   for(i = 0; i < 16; i++)
   {
      out[i] = 0;
   }

   for(i = 15; i >= 0; i--)
   {
      out[i] = ((in1[i] ^ in2[i]) ^ c);
      c = ((in1[i] & in2[i]) | (in1[i] & c)) | (in2[i] & c);
   }
}


//Controls output LEDs for top middle splitter. Goes to D if D is in the destination bits given in our instruction
//Takes in same data as our top right repeater, and also takes in an instruction which will tell us whether or not
//To send from splitter to D
void spltMtoD_(int instruction[16])
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
       
       //If we pass our data to the DReg from this splitter, it means we want to load it into the DReg
       // DReg[i] = outALU[i];
          DReg_val[i] = outALU[i];
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
void spltMtoRTL_(int instruction[16])
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

//Still working with same data as our other repeaters and top middle splitter, so we'll pass in the same thing
void out_repTL()
{
  int i;
   for(i = 0; i < 16; i++)
   {
      if(outALU[i] == 1)
      {
         repTL.setPixelColor(i, repTL.Color(255, 0, 0));
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
  // ARegtoPC.setPixelColor(0,0);
  pc.setPixelColor(0,0);
  for(i = 1; i < nLEDs; i++)
  {
    if(ainstr[i] == 1)
    {
      ARegtoMux.setPixelColor(i, ARegtoMux.Color(255, 0, 0));
      ARegtoMem.setPixelColor(i, ARegtoMem.Color(255, 0, 0));
      // ARegtoPC.setPixelColor(i, ARegtoPC.Color(255, 0, 0));
      pc.setPixelColor(i, pc.Color(255, 0, 0));
    }
    else
    {
      ARegtoMux.setPixelColor(i, 0);
      ARegtoMem.setPixelColor(i, 0);
      // ARegtoPC.setPixelColor(i,0);
      pc.setPixelColor(i,0);
    }
  }
  ARegtoMux.show();
  ARegtoMem.show();
  // ARegtoPC.show();
  pc.show();
}

void out_DReg()
{
   int i;
   for(i = 0; i < 15; i++)
   {
      if(DReg_val[i] == 1)
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
void out_repBR()
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
void out_repTR()
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

//Decides whether or not to send data to memory from bottom left splitter. Still working with same ALU output data,
//so we'll have the same input as our other repeaters and splitter. We'll see if M is in the destination
//of our instruction, and that will decide whether or not to send data to memory
void spltBLtoMem_(int instruction[16])
{
  int i, j, memLoc = 0;
  if(instruction[12] == 1)
  {
     //If we're sending data from this splitter to memory, we want to load the memory location specified
     //by our A-Register with the value from our outALU
       
     //Step 1, find destination. For now we only have a 64-bit memory, so we're going to limit where that
     //destination can be accordingly
     for(j = 0; j < 7; j++)
     {
        memLoc += (pow(2.0, j) * AReg_val[j]); 
     }
     
     //Now as we loop through our outALU array to set our LEDs we can set our memory at the specified location
     for(i = 0; i < nLEDs; i++)
     {
       if(outALU[i] == 1)
       {
         spltBLtoMem.setPixelColor(i, spltBLtoMem.Color(255, 0, 0));
       }//close if
       else
       {
         spltBLtoMem.setPixelColor(i, 0);
       }//close else
       //Since we're using an address from our AReg we want to show that the memory is receiving that data
       //so we're going to light up the ARegtoMem LEDs here.
       if(AReg_val[i] == 1)
       {
          ARegtoMem.setPixelColor(i, ARegtoMem.Color(255, 0, 0));
       }
       else
          ARegtoMem.setPixelColor(i, 0);
       //Step 2: Set memory values to outALU values
       memory[memLoc][i] = outALU[i];
     }//close for
  }//close if
  else
  {
     for(i = 0; i < nLEDs; i++)
     {
        spltBLtoMem.setPixelColor(i, 0);
     }
     
  }
  spltBLtoMem.show();
}

//Decides whether or not to send data from bottom left splitter to AReg. If it it sent, the values of AReg are overwritten
//Still using same ALU output as several other functions.
void spltBLtoA_(int instruction[16])
{
  int i;
  if(instruction[10] == 1)
  {
     for(i = 0; i < nLEDs; i++)
     {
       if(outALU[i] == 1)
       {
         spltBLtoA.setPixelColor(i, spltBLtoA.Color(255, 0, 0));
       }
       else
       {
         spltBLtoA.setPixelColor(i, 0);
       }
       //Set AReg values to outALU values if data is sent from splitter to AReg
       AReg_val[i] = outALU[i];
     }
  }
  else
  {
     for(i = 0; i < nLEDs; i++)
     {
        spltBLtoA.setPixelColor(i, 0);
     }
     
  }
  spltBLtoA.show();
}

//Takes in 3 least significant bits of our instruction (which control jump) and if any are true
//Then we turn on all the jump logic LED's
void jumpLogicOut(int instruction[16])
{
   int i = 0;
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

//This function sets the LEDs coming out from memory and into the multiplexer
void outMem()
{
   int i, memLoc = 0;
   //Find memory location to display. For now we only have a 64-bit memory, so we're going to limit where that
   //destination can be accordingly
   for(i = 0; i < 7; i++)
   {
      memLoc += (pow(2.0, i) * AReg_val[i]); 
   }
   for(i = 0; i < 16; i++)
   {
      if(memory[memLoc][i] == 1)
      {
         mem.setPixelColor(i, mem.Color(255, 0, 0));
      }
      else
         mem.setPixelColor(i, 0);
   }
   mem.show();
}

//This function controls the LEDs coming out of the multiplexer and into the ALU
// compiler throws error mux_ declared void, changing to int to see if it fixes it
// also I'm assuming this was supposed to be instructions
int mux_(int instruction[16])
{
   int i;
   //Start memLoc at -1 so that if we aren't outputting M from mux it's very clear for the ALU
   //If we do output M then memLoc will get set to the correct memory location in the 
   //Second conditional below
   int memLoc = -1;
  
   //If 'a' mnemonic is 0, then output AReg value from mux into ALU
   if(instruction[3] == 0)
   {
      for(i = 0; i < 15; i++)
      {
         if(AReg_val[i] == 1)
         {
            mux.setPixelColor(i, mux.Color(255, 0, 0));
         }
         else
            mux.setPixelColor(i, 0);
      }
   }
   //If 'a' mnemonic is 1, then output value from memory specified by address in AReg
   if(instruction[3] == 1)
   {
      memLoc = 0;
      for(i = 0; i < 7; i++)
      {
         memLoc += (pow(2.0, i) * AReg_val[i]); 
      }
      for(i = 0; i < 16; i++)
      {
         if(memory[memLoc][i] == 1)
         {
            mux.setPixelColor(i, mux.Color(255, 0, 0));
         }
         else
            mux.setPixelColor(i, 0);
      }
   }
   mux.show();
   return memLoc;
}

//This function handles the actual computation done by the ALU, as well as setting the LED bus coming out of the ALU
void ALU_out(int M, int instruction[16])
{
   //Gonna do this to decrease amount of memory loads
   int c1 = instruction[4], c2 = instruction[5], c3 = instruction[6], c4 = instruction[7],
      c5 = instruction[8], c6 = instruction[9], a = instruction[3];
  
   int i;
   //Start by initializing outALU to all zeroes
   for(i = 0; i < 16; i++)
      outALU[i] = 0;
   i = 0;
  
   //If comp is 0
   if(c1 && !c2 && c3 && !c4 && c5 && !c6)
   {
      //Output is already set to 0, so if comp is zero just return
     
   }
   //If comp is 1
   if(c1 && c2 && c3 && c4 && c5 && c6)
   {
      //Output set to zeroes, so if comp is 1 just return 16-bit binary for a 1
      outALU[15] = 1;
      
   }
   //If comp is -1
   if(c1 && c2 && c3 && !c4 && c5 && !c6)
   {
      //-1 in our two's compliment binary is just 16 1's
      for(i = 0; i < 16; i++)
         outALU[i] = 1;
      
   }
   //If comp is D
   if(!c1 && !c2 && c3 && c4 && !c5 && !c6)
   {
      //If comp is D just set output equal to D and return it
      for(i = 0; i < 16; i++)
        outALU[i] = DReg_val[i];
      
   }
   //If comp is A or M
   if(c1 && c2 && !c3 && !c4 && !c5 && !c6)
   {
      //If we want A for our comp, just set output equal to AReg_val and return
      if(!a)
      {
         for(i = 0; i < 16; i++)
           outALU[i] = AReg_val[i];
         
      }
      //If we want M for our comp, use argument M to load value in memory[M], set output equal to this,
      //Then return
      if(a)
      {
         for(i = 0; i < 16; i++)
           outALU[i] = memory[M][i];
         
      }
   }
   //If we want !D
   if(!c1 && !c2 && c3 && c4 && !c5 && c6)
   {
      //Start by setting outALU equal to D
      for(i = 0; i < 16; i++)
        outALU[i] = DReg_val[i];
      //Now negate each bit
      negate16Bit(outALU);
      
   }
   //If we want !A or !M
   if(c1 && c2 && !c3 && !c4 && !c5 && c6)
   {
      //If we want !A for our comp
      if(!a)
      {
         //Start by setting outALU equal to A
         for(i = 0; i < 16; i++)
           outALU[i] = AReg_val[i];
         //Negate each bit
         negate16Bit(outALU);
         
      }
      //If we want !M for our comp
      if(a)
      {
         //Start by setting outALU equal to M
         for(i = 0; i < 16; i++)
           outALU[i] = memory[M][i];
         //Negate each bit
         negate16Bit(outALU);
         
      }
   }
   //If we want -D
   if(!c1 && !c2 && c3 && c4 && c5 && c6)
   {
      //Start by setting outALU equal to D
      for(i = 0; i < 16; i++)
        outALU[i] = DReg_val[i];
      twosComp16Bit(outALU);
      
   }
   //If we want -A or -M
   if(c1 && c2 && !c3 && !c4 && c5 && c6)
   {
      //If we want -A
      if(!a)
      {
         //Start by setting outALU equal to A
         for(i = 0; i < 16; i++)
           outALU[i] = AReg_val[i];
         twosComp16Bit(outALU);
        
      }
      //If we want -M
      if(a)
      {
         //Start by setting outALU equal to M
         for(i = 0; i < 16; i++)
           outALU[i] = memory[M][i];
         twosComp16Bit(outALU);
        
      }
   }
   //If we want D+1
   if(!c1 && c2 && c3 && c4 && c5 && c6)
   {
      //Start by setting outALU equal to D
      for(i = 0; i < 16; i++)
        outALU[i] = DReg_val[i];
      //Increment by 1
      inc16Bit(outALU);
      
   }
   //If we want A+1 or M+1
   if(c1 && c2 && !c3 && c4 && c5 && c6)
   {
      //If we want A+1
      if(!a)
      {
         //Start by setting outALU equal to A
         for(i = 0; i < 16; i++)
           outALU[i] = AReg_val[i];
         inc16Bit(outALU);
         
      }
      //If we want M+1
      if(a)
      {
         //Start by setting outALU equal to M
         for(i = 0; i < 16; i++)
           outALU[i] = memory[M][i];
         inc16Bit(outALU);
         
      }
   }
   //If we want D-1
   if(!c1 && !c2 && c3 && c4 && c5 && !c6)
   {
      int num[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
      bitWiseAdd(outALU, DReg_val, num);
      
   }
   //If we want A-1 or M-1
   if(c1 && c2 && !c3 && !c4 && c5 && !c6)
   {
      int num[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
      //If we want A-1
      if(!a)
      {
         bitWiseAdd(outALU, AReg_val, num);
         
      }
      //If we want M-1
      if(a)
      {
         bitWiseAdd(outALU, memory[M], num);
         
      }
   }
   //If we want D+A or D+M
   if(!c1 && !c2 && !c3 && !c4 && c5 && !c6)
   {
      //If we want D+A
      if(!a)
      {
         bitWiseAdd(outALU, DReg_val, AReg_val);
      }
      //If we want D+M
      if(a)
      {
         bitWiseAdd(outALU, DReg_val, memory[M]);
      }
   }
   //If we want D-A or D-M
   if(!c1 && c2 && !c3 && !c4 && c5 && c6)
   {
      int temp[16];
      //If we want D-A
      if(!a)
      {
         //Start by copying AReg into a temporary array
         for(i = 0; i < 16; i++)
         {
            temp[i] = AReg_val[i];
         }
         twosComp16Bit(temp);
         bitWiseAdd(outALU, DReg_val, temp);
         
      }
      //If we want D-M
      if(a)
      {
         //Start by copying AReg into a temporary array
         for(i = 0; i < 16; i++)
         {
            temp[i] = memory[M][i];
         }
         twosComp16Bit(temp);
         bitWiseAdd(outALU, DReg_val, temp);
         
      }
   }
   //If we want A-D or M-D
   if(!c1 && !c2 && !c3 && c4 && c5 && c6)
   {
      int temp[16];
      //Start by copying DReg_val into a temp array
      for(i = 0; i < 16; i++)
      {
         temp[i] = DReg_val[i];
      }
      //If we want A-D
      if(!a)
      {
         twosComp16Bit(temp);
         bitWiseAdd(outALU, AReg_val, temp);
      }
      //If we want M-D
      if(a)
      {
         twosComp16Bit(temp);
         bitWiseAdd(outALU, memory[M], temp);
      }
   }
   //If we want D&A or D&M
   if(!c1 && !c2 && !c3 && !c4 && !c5 && !c6)
   {
      //If we want D&A
      if(!a)
      {
         for(i = 0; i < 16; i++)
         {
            outALU[i] = ((DReg_val[i]) & (AReg_val[i]));
         }
      }
      //If we want D&M
      if(a)
      {
         for(i = 0; i < 16; i++)
         {
            outALU[i] = ((DReg_val[i]) & (memory[M][i]));
         }
      }
   }
   //If we want D|A or D|M
   if(!c1 && c2 && !c3 && c4 && !c5 && c6)
   {
      //If we want D|A
      if(!a)
      {
         for(i = 0; i < 16; i++)
         {
            outALU[i] = ((DReg_val[i]) | (AReg_val[i]));
         }
      }
      //If we want D|M
      if(a)
      {
         for(i = 0; i < 16; i++)
         {
            outALU[i] = ((DReg_val[i]) | (memory[M][i]));
         }
      }
   }
   for(i = 0; i < nLEDs; i++)
   {
       if(outALU[i] == 1)
       {
         alu_data.setPixelColor(i, alu_data.Color(255, 0, 0));
       }
       else
       {
         alu_data.setPixelColor(i, 0);
       }
   }
   alu_data.show();
}

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

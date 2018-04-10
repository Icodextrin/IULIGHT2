
void clearAll()
{
  int i;
  for(i = 0; i < nLEDs; i++)
  {
    repTL.setPixelColor(i, 0);
    spltMtoRTL.setPixelColor(i, 0);
    repTR.setPixelColor(i,0);
    spltBLtoMem.setPixelColor(i,0);
    ARegtoMux.setPixelColor(i,0);
    jl.setPixelColor(i,0);
    mem.setPixelColor(i,0);
    pc.setPixelColor(i,0);
    mux.setPixelColor(i,0);
    DReg.setPixelColor(i,0);
    alu.setPixelColor(i,0);
    ARegtoMem.setPixelColor(i,0);
    spltBLtoA.setPixelColor(i,0);
    spltMtoD.setPixelColor(i,0);
    repBR.setPixelColor(i,0);
  }
  repTL.show();
  spltMtoRTL.show();
  repTR.show();
  spltBLtoMem.show();
  ARegtoMux.show();
  jl.show();
  mem.show();
  pc.show();
  mux.show();
  DReg.show();
  alu.show();
  ARegtoMem.show();
  spltBLtoA.show();
  spltMtoD.show();
  repBR.show();
}

void initLED()
{
    repTL.begin();
    spltMtoRTL.begin();
    repTR.begin();
    spltBLtoMem.begin();
    ARegtoMux.begin();
    jl.begin();
    mem.begin();
    pc.begin();
    mux.begin();
    DReg.begin();
    alu.begin();
    ARegtoMem.begin();
    spltBLtoA.begin();
    spltMtoD.begin();
    repBR.begin();
}

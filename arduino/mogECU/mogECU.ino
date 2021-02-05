/*
 * UnimODB2 arduino ECU - https://github.com/b3bb0/unimobd
 * 
 * Connect sensors to arduino & communicate data via MCP2515 CAN/ODB2 port
 * 
 * Based on MCP_CAN_lib by Cory J. Fowler
 * 
 * Written by Alberto bebbo Capponi - 27th Jan 2021
 * 
 * 
 * Arduino PINs    <->    MCP2515 CAN Bus Module  <->  OBD viecar
 *                2 - INT
 *               10 - CS
 *               11 - SI
 *               12 - SO
 *               13 - SCK
 *                    VCC  (+5v)
 *              GND - GND  (GND)
 *                                                 -  4  (GND)
 *                                                 -  5  (GND)
 *                                          c3 (H) -  6  (CAN BUS high)
 *                                          j3 (L) -  14 (CAN BUS low)
 *                                                   16 (+12v)
 * 
 * 
 */

#include <SimpleDHT.h>
#include <mcp_can.h>
#include <SPI.h>

#define SERIALDBG 0






// What pin to connect the sensor to
#define COOLANTPIN A0 

// Other resistor before sensor
#define COOLANTRESISTOR 1195

#define COOLANTNOM 1840
#define COOLANTTEMPNOM 25

// #define BETA COEFFICIENT (usually 3000 - 4000)
// https://www.thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
#define COOLANTCOEFF 3888

int CoolantTemp;

unsigned long FastPreviousMillis = 0; // will store last time LED was updated
const long FastInterval = 1000;

unsigned long SlowPreviousMillis = 0; // will store last time LED was updated
const long SlowInterval = 5000;


// cabin temperature
SimpleDHT11 dht11(8); // PIN 8
byte CabinTemperature = 0;
byte CabinHumidity    = 0;







// used for CAM/OBD2 [start]
#define PAD 0x00
#define REPLY_ID 0x98DAF101
#define LISTEN_ID 0x98DA01F1
#define CAN0_INT 2  // INT on PIN 2
MCP_CAN CAN0(10);   // SI on PIN 10
unsigned long rxId;
byte dlc;
byte rxBuf[8];
byte txData[8];                             // Set CAN0 CS to pin 10
// used for CAM/OBD2 [stop]


void setup()
{
  #if SERIALDBG == 1
    while(!Serial);
  #endif
  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  // if(CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_20MHZ) == CAN_OK) {
  if(CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) == CAN_OK) {
    #if SERIALDBG == 1
      Serial.println("MCP2515 Initialized Successfully!");
    #endif
  } else {
    #if SERIALDBG == 1
      Serial.println("Error Initializing MCP2515...");
    #endif
  }

  // Extended ID Filters
  CAN0.init_Mask(0,0x90FFFF00); // Init first mask...
  CAN0.init_Filt(0,0x90DB3300); // Init first filter...
  CAN0.init_Filt(1,0x90DA0100); // Init second filter...
  CAN0.init_Mask(1,0x90FFFF00); // Init second mask... 
  CAN0.init_Filt(2,0x90DB3300); // Init third filter...
  CAN0.init_Filt(3,0x90DA0100); // Init fouth filter...
  CAN0.init_Filt(4,0x90DB3300); // Init fifth filter...
  CAN0.init_Filt(5,0x90DA0100); // Init sixth filter...
  
  CAN0.setMode(MCP_NORMAL); // Sends acks to received data.

  pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input
  
  #if SERIALDBG == 1
    Serial.println("OBD-II CAN Simulator");
  #else
    // start serial used for node interface
    Serial.begin(115200);
  #endif
}

void loop()
{
  if (!digitalRead(CAN0_INT)) {
    CAN0.readMsgBuf(&rxId, &dlc, rxBuf); // Get CAN data
    if (rxId == 0x98DB33F1) obdReq(rxBuf); // same requestID for most devices
  }

  updateSensors();
}

void updateSensors() {
  unsigned long currentMillis = millis();

  if (currentMillis - FastPreviousMillis >= FastInterval) {
    FastPreviousMillis = currentMillis;

    float reading;
    reading = analogRead(COOLANTPIN);
    
    float ohm;
    ohm = COOLANTRESISTOR / ((1023 / reading)  - 1);

    float temp;
    temp = (1.0 / ((log(ohm / COOLANTNOM) / COOLANTCOEFF) + 1.0 / (COOLANTTEMPNOM + 273.15))) - 273.15 ;
    
    CoolantTemp = (int) temp;

    Serial.print("CoolantTemp:"); 
    Serial.println(CoolantTemp);
  }

  if (currentMillis - SlowPreviousMillis >= SlowInterval) {
    SlowPreviousMillis = currentMillis;
    
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&CabinTemperature, &CabinHumidity, NULL)) != SimpleDHTErrSuccess) {
      // Serial.print("Read DHT11 failed, err="); Serial.print(SimpleDHTErrCode(err));
      // Serial.print(","); Serial.println(SimpleDHTErrDuration(err)); delay(1000);
    } else {
      Serial.print("CabinTemperature:");
      Serial.println((int)CabinTemperature);
      Serial.print("CabinHumidity:"); 
      Serial.println((int)CabinHumidity);
    }
  }
}


void obdReq(byte *data){
  byte numofBytes = data[0];
  byte mode = data[1] & 0x0F;
  byte pid = data[2];
  bool tx = false;
  txData[0] = 0x00;
  txData[1] = (0x40 | mode);
  txData[2] = pid;
  txData[3] = PAD;
  txData[4] = PAD;
  txData[5] = PAD;
  txData[6] = PAD;
  txData[7] = PAD;
  
  // MODE 0x01 - Show current data
  if (mode == 0x01){
    tx = true;
    
    // Supported PIDs 01-20: 10101000000110100000000000000011  (A81A0003)
    if(pid == 0x00){
      txData[0] = 0x06;
      txData[3] = 0xA8;
      txData[4] = 0x1A;
      txData[5] = 0x00;
      txData[6] = 0x03;
    }
    
    // Supported PIDs 21-40: 00000000000000100000000000000001  (00 02 00 01) 
    else if(pid == 0x20) {
      txData[0] = 0x06;
      txData[3] = 0x00;
      txData[4] = 0x02;
      txData[5] = 0x00;
      txData[6] = 0x01;
    }

    // Supported PIDs 21-40: 01000100000000000000000000010000  (44 00 00 10) 
    else if(pid == 0x40) {
      txData[0] = 0x06;
      txData[3] = 0x44;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x10;
    }
  
    // Monitor status since DTs cleared.
    else if(pid == 0x01){
      bool MIL = true;
      byte DTC = 5;
      txData[0] = 0x06;
      txData[3] = (MIL << 7) | (DTC & 0x7F);
      txData[4] = 0x07;
      txData[5] = 0xFF;
      txData[6] = 0x00;
    }

    else if(pid == 0x03) fuelsystem_data(); // Fuel system status
    else if(pid == 0x05) coolant_data(); // Engine coolant temperature
    else if(pid == 0x0C) RPM_data(); // Engine RPM
    else if(pid == 0x0D) speed_data(); // Vehicle speed
    else if(pid == 0x0F) intaketemp_data(); // Intake air temperature
    else if(pid == 0x2F) fueltank_data(); // Fuel Tank Level Input

    else if(pid == 0x42) voltage_data(); // Control module voltage
    else if(pid == 0x46) ambientTemp_data(); // Ambient air temperature
    else if(pid == 0x5C) oilTemp_data(); // Engine oil temperature

    // Supported PIDs
    else if(pid == 0x80 || pid == 0xA0 || pid == 0xA0 || pid == 0xC0 || pid == 0xE0){
      txData[0] = 0x00;
      txData[3] = 0x00;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x00;
      tx = true;
    } else{
      tx = false;
      unsupported(mode, pid);
    }
  }
  
  else if(mode == 0x02) unsupported(mode, pid); // MODE 0x02 - Show freeze frame data
  
  // MODE 0x03 - Show stored DTCs
  else if(mode == 0x03){
      byte DTCs[] = {0x40, 0x05, 0xC0, 0xBA, 0x00, 0x11, 0x80, 0x13, 0x90, 0x45, 0xA0, 0x31};
      DTCs[0] = 0x40 | mode;
      iso_tp(mode, pid, 12, DTCs);
  }
  
  // MODE 0x04 - Clear DTCs and stored values
  else if(mode == 0x04){
      // Need to cleat DTCs.  We just acknowledge the command for now.
      txData[0] = 0x01;
      tx = true;
  }
  
  // MODE 0x05 - Test Results, oxygen sensor monitoring (non CAN only)
  else if(mode == 0x05){
      unsupported(mode, pid);
  }
  
  // MODE 0x06 - Test Results, On-Board Monitoring (Oxygen sensor monitoring for CAN only)
  else if(mode == 0x06){
    if(pid == 0x00){        // Supported PIDs 01-20
      txData[0] = 0x06;
      txData[3] = 0x00;
      txData[4] = 0x00;
      txData[5] = 0x00;
      txData[6] = 0x00;
      tx = true;
    }
    else{
      unsupported(mode, pid);
    }
  }
  
  // MODE 0x07 - Show pending DTCs (Detected during current or last driving cycle)
  else if(mode == 0x07){
      byte DTCs[] = {0x40, 0x05, 0xC0, 0xBA, 0x00, 0x11, 0x80, 0x13, 0x90, 0x45, 0xA0, 0x31};
      DTCs[0] = (0x40 | mode);
      iso_tp(mode, pid, 12, DTCs);
  }
  
  // MODE 0x08 - Control operation of on-board component/system
  else if(mode == 0x08){
      unsupported(mode, pid);
  }
  
  // MODE 0x09 - Request vehcile information
  else if(mode == 0x09){
    if(pid == 0x00){        // Supported PIDs 01-20
      txData[0] = 0x06;
      txData[3] = 0x54;
      txData[4] = 0x40;
      txData[5] = 0x00;
      txData[6] = 0x00;
      tx = true;
    }

    // VIN (17 to 20 Bytes) Uses ISO-TP
    else if(pid == 0x02){
      // 1ZVBP8AM7D5220181
      byte VIN[] = {0x40, pid, 0x01, 0x31, 0x5a, 0x56, 0x42, 0x50, 0x38, 0x41, 0x4d, 0x37, 0x44, 0x35, 0x32, 0x32, 0x30, 0x31, 0x38, 0x31};
      VIN[0] = (0x40 | mode);
      iso_tp(mode, pid, 20, VIN);
    }

    // Calibration ID
    else if(pid == 0x04){
      byte CID[] = {0x40, pid, 0x01, 0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x20, 0x4F, 0x42, 0x44, 0x49, 0x49, 0x73, 0x69, 0x6D, 0x51, 0x52, 0x53, 0x54};
      CID[0] = (0x40 | mode);
      iso_tp(mode, pid, 23, CID);
    }

    // CVN
    else if(pid == 0x06){
      byte CVN[] = {0x40, pid, 0x02, 0x11, 0x42, 0x42, 0x42, 0x22, 0x43, 0x43, 0x43};
      CVN[0] = (0x40 | mode);
      iso_tp(mode, pid, 11, CVN);
    }

    // ECM Name
    else if(pid == 0x0A){
      byte ECMname[] = {0x40, pid, 0x01, 0x45, 0x43, 0x4D, 0x00, 0x2D, 0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x4F, 0x42, 0x44, 0x49, 0x49, 0x73, 0x69, 0x6D};
      ECMname[0] = (0x40 | mode);
      iso_tp(mode, pid, 23, ECMname);
    }

    // ESN
    else if(pid == 0x0D){
      byte ESN[] = {0x40, pid, 0x01, 0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x2D, 0x4F, 0x42, 0x44, 0x49, 0x49, 0x73, 0x69, 0x6D, 0x00};
      ESN[0] = (0x40 | mode);
      iso_tp(mode, pid, 20, ESN);
    }
    else unsupported(mode, pid); 
  }
  
  // MODE 0x0A - Show permanent DTCs 
  else if(mode == 0x0A){
      byte DTCs[] = {0x40, 0x05, 0xC0, 0xBA, 0x00, 0x11, 0x80, 0x13, 0x90, 0x45, 0xA0, 0x31};
      DTCs[0] = (0x40 | mode);
      iso_tp(mode, pid, 12, DTCs);
  }
  else unsupported(mode, pid);
  
  if(tx)
    CAN0.sendMsgBuf(REPLY_ID, 8, txData);
}

// Generic debug serial output
void unsupported(byte mode, byte pid){
  negAck(mode, 0x12);
  unsupportedPrint(mode, pid);  
}

// Generic debug serial output
void negAck(byte mode, byte reason){
  byte txData[] = {0x03,0x7F,mode,reason,PAD,PAD,PAD,PAD};
  CAN0.sendMsgBuf(REPLY_ID, 8, txData);
}

// Generic debug serial output
void unsupportedPrint(byte mode, byte pid){
  char msgstring[64];
  sprintf(msgstring, "Mode $%02X: Unsupported PID $%02X requested!", mode, pid);

  #if SERIALDBG == 1
    Serial.println(msgstring);
  #endif
}

// Blocking example of ISO transport
void iso_tp(byte mode, byte pid, int len, byte *data){
  byte tpData[8];
  int offset = 0;
  byte index = 0;
  
  // First frame
  tpData[0] = 0x10 | ((len >> 8) & 0x0F);
  tpData[1] = 0x00FF & len;
  for(byte i=2; i<8; i++) tpData[i] = data[offset++];
  
  CAN0.sendMsgBuf(REPLY_ID, 8, tpData);
  index++; // We sent a packet so increase our index.
  
  bool not_done = true;
  unsigned long sepPrev = millis();
  byte sepInvl = 0;
  byte frames = 0;
  bool lockout = false;
  
  while(not_done){
    // Need to wait for flow frame
    if (!digitalRead(CAN0_INT)) {
      CAN0.readMsgBuf(&rxId, &dlc, rxBuf);
    
      if((rxId == LISTEN_ID) && ((rxBuf[0] & 0xF0) == 0x30)){
        if((rxBuf[0] & 0x0F) == 0x00){ // Continue
          frames = rxBuf[1];
          sepInvl = rxBuf[2];
          lockout = true;
        } else if((rxBuf[0] & 0x0F) == 0x01){ // Wait
          lockout = false;
          delay(rxBuf[2]);
        } else if((rxBuf[0] & 0x0F) == 0x03){ // Abort
          not_done = false;
          return;
        }
      }
    }

    if (((millis() - sepPrev) >= sepInvl) && lockout){
      sepPrev = millis();

      tpData[0] = 0x20 | index++;
      for (byte i=1; i<8; i++){
        if(offset != len)
          tpData[i] = data[offset++];
        else
          tpData[i] = 0x00;
      }
      
      CAN0.sendMsgBuf(REPLY_ID, 8, tpData); // Do consecutive frames as instructed via flow frame
      
      if (frames-- == 1) lockout = false;
    }

    if (offset == len) {
      not_done = false;
    } else {
      char msgstring[32];
      sprintf(msgstring,"Offset: 0x%04X\tLen: 0x%04X", offset, len);
      #if SERIALDBG == 1
        Serial.println(msgstring);
      #endif
    }
    
    if((millis() - sepPrev) >= 1000) not_done = false; // Timeout
  }
  
}


void RPM_data() {
  int rpms = RPM_read(); // TODO: read from sensor
  rpms = rpms * 4;
  
  txData[0] = 0x04;
  txData[3] = (rpms >> 8);
  txData[4] = rpms;
}

void coolant_data() {
  int temp = coolant_read() + 40;
  txData[0] = 0x03;
  txData[3] = temp;
}

void intaketemp_data() {
  int temp = intaketemp_read() + 40;
  txData[0] = 0x03;
  txData[3] = temp;
}



// Fuel system status
void fuelsystem_data() {
  txData[0] = 0x03;
  txData[3] = 0x02; // Closed loop, using oxygen sensor feedback to determine fuel mix. should be almost always this
  // txData[3] = 0x01; // open due to insufficient engine temperature
  // txData[3] = 0x04; // Open loop due to engine load, can trigger DFCO
  // txData[3] = 0x08; // Open loop due to system failure
  // txData[3] = 0x10; // Closed loop, using at least one oxygen sensor but there is a fault in the feedback system
}

void speed_data() {
  txData[0] = 0x03;
  txData[3] = speed_read();
}


// Fuel Tank Level Input (%)
void fueltank_data() {
  txData[0] = 0x03;
  txData[3] = fueltank_read();
}

void voltage_data() { // Control module voltage 
  int volts = voltage_read() * 10; // TODO: read from sensor
  
  txData[0] = 0x04;
  txData[3] = (volts >> 8);
  txData[4] = volts;
  
}

void ambientTemp_data() { // Ambient air temperature
  txData[0] = 0x03;
  txData[3] = ambientTemp_read() + 40;
}

void oilTemp_data() { // Engine oil temperature
  txData[0] = 0x03;
  txData[3] = oilTemp_read() + 40;
}


int voltage_read() {
  return 1243; // 1243 = 12.43 v
}

int fueltank_read() {
  // 255 = full, 1 = empty
  return 200;
}

int RPM_read() {
  return 1900;
}

int coolant_read() {
  return CoolantTemp;
}

int intaketemp_read() {
  return 31;
}

int ambientTemp_read() {
  return (int)CabinTemperature;
}

int speed_read() {
  return 35;
}

int oilTemp_read() {
  return 90;
}

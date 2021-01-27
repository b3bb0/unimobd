/*
 * UnimODB2 arduino ECU - https://github.com/b3bb0/unimobd
 * 
 * Connect sensors to arduino & communicate data via MCP2515 CAN/ODB2 port
 * 
 * Based on MCP_CAN_lib by Cory J. Fowler
 * 
 * Written by Alberto bebbo Capponi - 27th Jan 2021
 * 
 */

#include <mcp_can.h>
#include <SPI.h>

#define PAD 0x00

// What CAN ID type?  Standard or Extended
#define standard 0

// 7E0/8 = Engine ECM
// 7E1/9 = Transmission ECM

#if standard == 1
  #define REPLY_ID 0x7E9
  #define LISTEN_ID 0x7E1
  #define FUNCTIONAL_ID 0x7DF  
#else
  #define REPLY_ID 0x98DAF101
  #define LISTEN_ID 0x98DA01F1
  #define FUNCTIONAL_ID 0x98DB33F1
#endif


// CAN RX Variables
unsigned long rxId;
byte dlc;
byte rxBuf[8];

// CAN Interrupt and Chip Select
#define CAN0_INT 2                              // Set CAN0 INT to pin 2
MCP_CAN CAN0(9);                                // Set CAN0 CS to pin 9


void setup()
{
  Serial.begin(115200);
  while(!Serial);
  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_20MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");

  #if standard == 1
    // Standard ID Filters
    CAN0.init_Mask(0,0x7F00000);                // Init first mask...
    CAN0.init_Filt(0,0x7DF0000);                // Init first filter...
    CAN0.init_Filt(1,0x7E10000);                // Init second filter...
    
    CAN0.init_Mask(1,0x7F00000);                // Init second mask... 
    CAN0.init_Filt(2,0x7DF0000);                // Init third filter...
    CAN0.init_Filt(3,0x7E10000);                // Init fouth filter...
    CAN0.init_Filt(4,0x7DF0000);                // Init fifth filter...
    CAN0.init_Filt(5,0x7E10000);                // Init sixth filter...

  #else
    // Extended ID Filters
    CAN0.init_Mask(0,0x90FFFF00);                // Init first mask...
    CAN0.init_Filt(0,0x90DB3300);                // Init first filter...
    CAN0.init_Filt(1,0x90DA0100);                // Init second filter...
    
    CAN0.init_Mask(1,0x90FFFF00);                // Init second mask... 
    CAN0.init_Filt(2,0x90DB3300);                // Init third filter...
    CAN0.init_Filt(3,0x90DA0100);                // Init fouth filter...
    CAN0.init_Filt(4,0x90DB3300);                // Init fifth filter...
    CAN0.init_Filt(5,0x90DA0100);                // Init sixth filter...
  #endif
  
  CAN0.setMode(MCP_NORMAL);                          // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                          // Configuring pin for /INT input
  
  Serial.println("OBD-II CAN Simulator");
}

void loop()
{
  if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &dlc, rxBuf);             // Get CAN data
    
    // First request from most adapters...
    if(rxId == FUNCTIONAL_ID){
      obdReq(rxBuf);
    }       
  }
}

void obdReq(byte *data){
  byte numofBytes = data[0];
  byte mode = data[1] & 0x0F;
  byte pid = data[2];
  bool tx = false;
  byte txData[] = {0x00,(0x40 | mode),pid,PAD,PAD,PAD,PAD,PAD};

  //txData[1] = 0x40 | mode;
  //txData[2] = pid; 
  
  // MODE 0x01 - Show current data
  if (mode == 0x01){

    // Supported PIDs 01-20
    if(pid == 0x00){
      txData[0] = 0x06;
      txData[3] = 0x80;
      txData[4] = 0x38;
      txData[5] = 0x00;
      txData[6] = 0x01;
      tx = true;
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
      tx = true;
    }

    // Fuel system status
    else if(pid == 0x03){
      txData[0] = 0x03;
      txData[3] = 0xFA;
      tx = true;
    }

    // Engine coolant temperature
    else if(pid == 0x05){
      txData[0] = 0x03;
      txData[3] = 0xFA;
      tx = true;
    }

    // Engine RPM
    else if(pid == 0x0C){
      txData[0] = 0x04;
      txData[3] = 0x9C;
      txData[4] = 0x40;
      tx = true;
    }

    // Vehicle speed
    else if(pid == 0x0D){
      txData[0] = 0x03;
      txData[3] = 0xFA;
      tx = true;
    }

    // Intake air temperature
    else if(pid == 0x0F){
      txData[0] = 0x03;
      txData[3] = 0xFA;
      tx = true;
    }

    // Engine oil Temperature
    else if(pid == 0x5C){
      txData[0] = 0x03;
      txData[3] = 0x1E;
      tx = true;
    }

    // Supported PIDs 21-40 || 41-60 || 61-80 || 81-A0 || A1-C0 || C1-E0 || E1-FF
    else if(pid == 0x20 || pid == 0x40 || pid == 0x60 || pid == 0x80 || pid == 0xA0 || pid == 0xC0 || pid == 0xE0){
      txData[0] = 0x06;
      txData[3] = 0x00;
      txData[4] = 0x08;
      txData[5] = 0x00;
      txData[6] = 0x01; // 0x0D || 0x00;
      tx = true;
    }
    else{
      unsupported(mode, pid);
      //  0x02 >  Freeze DTC
      //  0x04 >  Calculated engine load
      //  0x06 >  Short term fuel trim - Bank 1
      //  0x07 >  Long tern fuel trim - Bank 1
      //  0x08 >  Short term fuel trim - Bank 2
      //  0x09 >  Long term fuel trim - Bank 2
      //  0x0A >  Fuel pressure (gauge)
      //  0x0B > Intake manifold absolute pressure
      //  0x0E >  Timing advance
      //  0x10 >  MAF air flow rate
      //  0x11 >  Throttle position
      //  0x12 >  Commanded secondary air status
      //  0x13 >  Oxygen sensors present (in 2 banks)
      //  0x14 >  Oxygen Sensor 1 (Voltage & Trim)
      //  0x15 >  Oxygen Sensor 2 (Voltage & Trim)
      //  0x16 >  Oxygen Sensor 3 (Voltage & Trim)
      //  0x17 >  Oxygen Sensor 4 (Voltage & Trim)
      //  0x18 >  Oxygen Sensor 5 (Voltage & Trim)
      //  0x19 >  Oxygen Sensor 6 (Voltage & Trim)
      //  0x1A >  Oxygen Sensor 7 (Voltage & Trim)
      //  0x1B >  Oxygen Sensor 8 (Voltage & Trim)
      //  0x1C >  OBD standards this vehicle conforms to
      //  0x1D >  Oxygen sensors present (in 4 banks)
      //  0x1E >  Auxillary input status
      //  0x1F >  Run time since engine start
      //  0x21 >  Distance traveled with MIL on (Mulfunction indicator light)
      //  0x22 >  Fuel rail pressure (Relative to Manifold Vacuum)
      //  0x23 >  Fuel rail gauge pressure (diesel or gasoline direct injection)
      //  0x24 >  Oxygen Sensor 1 (Fuel to Air & Voltage)
      //  0x25 >  Oxygen Sensor 2 (Fuel to Air & Voltage)
      //  0x26 >  Oxygen Sensor 3 (Fuel to Air & Voltage)
      //  0x27 >  Oxygen Sensor 4 (Fuel to Air & Voltage)
      //  0x28 >  Oxygen Sensor 5 (Fuel to Air & Voltage)
      //  0x29 >  Oxygen Sensor 6 (Fuel to Air & Voltage)
      //  0x2A >  Oxygen Sensor 7 (Fuel to Air & Voltage)
      //  0x2B >  Oxygen Sensor 8 (Fuel to Air & Voltage)
      //  0x2C >  Commanded EGR
      //  0x2D >  EGR Error
      //  0x2E >  Commanded evaporative purge
      //  0x2F >  Fuel tank level input
      //  0x30 >  Warm-ups since codes cleared
      //  0x31 >  Distance traveled since codes cleared
      //  0x32 >  Evap. System Vapor Pressure
      //  0x33 >  Absolute Barometric Pressure
      //  0x34 >  Oxygen Sensor 1 (Fuel to Air & Current) 
      //  0x35 >  Oxygen Sensor 2 (Fuel to Air & Current) 
      //  0x36 >  Oxygen Sensor 3 (Fuel to Air & Current) 
      //  0x37 >  Oxygen Sensor 4 (Fuel to Air & Current) 
      //  0x38 >  Oxygen Sensor 5 (Fuel to Air & Current) 
      //  0x39 >  Oxygen Sensor 6 (Fuel to Air & Current) 
      //  0x3A >  Oxygen Sensor 7 (Fuel to Air & Current) 
      //  0x3B >  Oxygen Sensor 8 (Fuel to Air & Current) 
      //  0x3C >  Catalyst Temperature: Bank 1, Sensor 1
      //  0x3D >  Catalyst Temperature: Bank 2, Sensor 1
      //  0x3E >  Catalyst Temperature: Bank 1, Sensor 2
      //  0x3F >  Catalyst Temperature: Bank 2, Sensor 2
      //  0x41 >  Monitor status this drive cycle
      //  0x42 >  Control module voltage
      //  0x43 >  Absolute load value
      //  0x44 >  Fuel-Air commanded equivalence ratio
      //  0x45 >  Relative throttle position
      //  0x46 >  Ambient air temperature
      //  0x47 >  Absolute throttle postion B
      //  0x48 >  Absolute throttle postion C
      //  0x49 >  Accelerator pedal position D
      //  0x4A >  Accelerator pedal position E
      //  0x4B >  Accelerator pedal position F
      //  0x4C >  Commanded throttle actuator
      //  0x4D > Time run with MIL on
      //  0x4E >  Time since troble codes cleared
      //  0x4F >  Time since trouble codes cleared
      //  0x50 >  Maximum value for Fuel-Air equivalence ratio, oxygen sensor voltage, oxygen sensro current, and intake manifold absolute-pressure
      //  0x51 >  Fuel Type
      //  0x52 >  Ethanol Fuel %
      //  0x53 >  Absolute evap system vapor pressure
      //  0x54 >  Evap system vapor pressure
      //  0x55 >  Short term secondary oxygen sensor trim, A: bank 1, B: bank 3
      //  0x56 >  Long term secondary oxygen sensor trim, A: bank 1, B: bank 3
      //  0x57 >  Short term secondary oxygen sensor trim, A: bank 2, B: bank 4
      //  0x58 >  Long term secondary oxygen sensor trim, A: bank 2, B: bank 4
      //  0x59 >  Fuel rail absolute pressure
      //  0x5A >  Relative accelerator pedal position
      //  0x5B >  Hybrid battery pack remaining life
      //  0x5D >  Fuel injection timing
      //  0x5F >  Emissions requirements to which vehicle is designed
      //  0x5E >  Engine fuel rate
      //  0x61 >  Driver's demand engine - percent torque
      //  0x62 >  Actual engine - percent torque
      //  0x63 >  Engine reference torque
      //  0x64 >  Engine percent torque data
      //  0x65 >  Auxiliary input / output supported
      //  0x66 >  Mas air flow sensor
      //  0x67 >  Engine coolant temperature
      //  0x68 >  Intake air temperature sensor
      //  0x69 >  Commanded EGR and EGR error
      //  0x6A >  Commanded Diesel intake air flow control and relative intake air flow position
      //  0x6B >  Exhaust gas recirculation temperature
      //  0x6C >  Commanded throttle actuator control and relative throttle position
      //  0x6D >  Fuel pressure control system
      //  0x6E >  Injection pressure control system
      //  0x6F >  Turbocharger compressor inlet pressure
      //  0x70 >  Boost pressure control
      //  0x71 >  Variable Geometry turbo sensor
      //  0x72 >  Wastegate control  
      //  0x73 >  Exhaust pressure
      //  0x74 >  Turbocharger RPM
      //  0x75 >  Turbocharger temperature
      //  0x76 >  Turbocharger temperature
      //  0x77 >  Charge air cooler temperature (CACT)
      //  0x78 >  Exhaust Gas Temperature (EGT) bank 1
      //  0x79 >  Exhaust Gas Temperature (EGT) bank 2
      //  0x7A >  Diesel particulate filter (DPF)
      //  0x7B >  Diesel particulate filter (DPF)
      //  0x7C >  Diesel particulate filter (DPF) temperature
      //  0x7D >  NOx NTE control area status
      //  0x7E >  PM NTE control area status
      //  0x7F >  Engine run time
      //  0x81 >  Engine run time for Auxiliary Emissions Control Device (AECD)
      //  0x82 >  Engine run time for Auxiliary Emissions Control Device (AECD)  
      //  0x83 >  NOx sensor
      //  0x84 >  Manifold surface temperature
      //  0x85 >  NOx reqgent system
      //  0x86 >  Particulate Matter (PM) sensor
      //  0x87 >  Intake manifold absolute pressure
    }
  }
  
  // MODE 0x02 - Show freeze frame data
  else if(mode == 0x02){
      unsupported(mode, pid);
  }
  
  // MODE 0x03 - Show stored DTCs
  else if(mode == 0x03){
      byte DTCs[] = {(0x40 | mode), 0x05, 0xC0, 0xBA, 0x00, 0x11, 0x80, 0x13, 0x90, 0x45, 0xA0, 0x31};
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
      byte DTCs[] = {(0x40 | mode), 0x05, 0xC0, 0xBA, 0x00, 0x11, 0x80, 0x13, 0x90, 0x45, 0xA0, 0x31};
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
      byte VIN[] = {(0x40 | mode), pid, 0x01, 0x31, 0x5a, 0x56, 0x42, 0x50, 0x38, 0x41, 0x4d, 0x37, 0x44, 0x35, 0x32, 0x32, 0x30, 0x31, 0x38, 0x31};
      iso_tp(mode, pid, 20, VIN);
    }

    // Calibration ID
    else if(pid == 0x04){
      byte CID[] = {(0x40 | mode), pid, 0x01, 0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x20, 0x4F, 0x42, 0x44, 0x49, 0x49, 0x73, 0x69, 0x6D, 0x51, 0x52, 0x53, 0x54};
      iso_tp(mode, pid, 23, CID);
    }

    // CVN
    else if(pid == 0x06){
      byte CVN[] = {(0x40 | mode), pid, 0x02, 0x11, 0x42, 0x42, 0x42, 0x22, 0x43, 0x43, 0x43};
      iso_tp(mode, pid, 11, CVN);
    }

    // ECM Name
    else if(pid == 0x0A){
      byte ECMname[] = {(0x40 | mode), pid, 0x01, 0x45, 0x43, 0x4D, 0x00, 0x2D, 0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x4F, 0x42, 0x44, 0x49, 0x49, 0x73, 0x69, 0x6D};
      iso_tp(mode, pid, 23, ECMname);
    }

    // ESN
    else if(pid == 0x0D){
      byte ESN[] = {(0x40 | mode), pid, 0x01, 0x41, 0x72, 0x64, 0x75, 0x69, 0x6E, 0x6F, 0x2D, 0x4F, 0x42, 0x44, 0x49, 0x49, 0x73, 0x69, 0x6D, 0x00};
      iso_tp(mode, pid, 20, ESN);
    }

    // UNSUPPORTED
    else{
      unsupported(mode, pid); 
      // 0x01 >  VIN message count for PID 02. (Only for ISO 9141-2, ISO 14230-4 and SAE J1850.)
      // 0x03 >  Calibration ID message count for PID 04. (Only for ISO 9141-2, ISO 14230-4 and SAE J1850.)
      // 0x05 >  Calibration Verification Number (CVN) message count for PID 06. (Only for ISO 9141-2, ISO 14230-4 and SAE J1850.)
      // 0x07 >  In-use performance tracking message count for PID 08 and 0B. (Only for ISO 9141-2, ISO 14230-4 and SAE J1850.)
      // 0x08 >  In-use performance tracking for spark ignition vehicles.
      // 0x09 >  ECU name message count for PID 0A.
      // 0x0B >  In-use performance tracking for compression ignition vehicles.
      // 0x0C >  ESN message count for PID 0D.
    }
  }
  
  // MODE 0x0A - Show permanent DTCs 
  else if(mode == 0x0A){
      byte DTCs[] = {(0x40 | mode), 0x05, 0xC0, 0xBA, 0x00, 0x11, 0x80, 0x13, 0x90, 0x45, 0xA0, 0x31};
      iso_tp(mode, pid, 12, DTCs);
  }
  else { 
    unsupported(mode, pid);
    // UDS Modes: Diagonstic and Communications Management =======================================
    // MODE 0x10 - Diagnostic Session Control
    // MODE 0x11 - ECU Reset
    // MODE 0x27 - Security Access  
    // MODE 0x28 - Communication Control
    // MODE 0x3E - Tester Present
    // MODE 0x83 - Access Timing Parameters
    // MODE 0x84 - Secured Data Transmission
    // MODE 0x85 - Control DTC Sentings
    // MODE 0x86 - Response On Event
    // MODE 0x87 - Link Control
    // UDS Modes: Data Transmission ==============================================================
    // MODE 0x22 - Read Data By Identifier
    // MODE 0x23 - Read Memory By Address
    // MODE 0x24 - Read Scaling Data By Identifier
    // MODE 0x2A - Read Data By Periodic Identifier  
    // MODE 0x2C - Dynamically Define Data Identifier
    // MODE 0x2E - Write Data By Identifier  
    // MODE 0x3D - Write Memory By Address
    // UDS Modes: Stored Data Transmission =======================================================
    // MODE 0x14 - Clear Diagnostic Information
    // MODE 0x19 - Read DTC Information
    // UDS Modes: Input Output Control ===========================================================
    // MODE 0x2F - Input Output Control By Identifier
    // UDS Modes: Remote Activation of Routine ===================================================
    // MODE 0x31 - Routine Control
    // UDS Modes: Upload / Download ==============================================================
    // MODE 0x34 - Request Download
    // MODE 0x35 - Request Upload
    // MODE 0x36 - Transfer Data
    // MODE 0x37 - Request Transfer Exit
    // MODE 0x38 - Request File Transfer
  }
  
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
  Serial.println(msgstring);
}

// Blocking example of ISO transport
void iso_tp(byte mode, byte pid, int len, byte *data){
  byte tpData[8];
  int offset = 0;
  byte index = 0;
//  byte packetcnt = ((len & 0x0FFF) - 6) / 7;
//  if((((len & 0x0FFF) - 6) % 7) > 0)
//    packetcnt++;

  // First frame
  tpData[0] = 0x10 | ((len >> 8) & 0x0F);
  tpData[1] = 0x00FF & len;
  for(byte i=2; i<8; i++){
    tpData[i] = data[offset++];
  }
  CAN0.sendMsgBuf(REPLY_ID, 8, tpData);
  index++; // We sent a packet so increase our index.
  
  bool not_done = true;
  unsigned long sepPrev = millis();
  byte sepInvl = 0;
  byte frames = 0;
  bool lockout = false;
  while(not_done){
    // Need to wait for flow frame
    if(!digitalRead(CAN0_INT)){
      CAN0.readMsgBuf(&rxId, &dlc, rxBuf);
    
      if((rxId == LISTEN_ID) && ((rxBuf[0] & 0xF0) == 0x30)){
        if((rxBuf[0] & 0x0F) == 0x00){
          // Continue
          frames = rxBuf[1];
          sepInvl = rxBuf[2];
          lockout = true;
        } else if((rxBuf[0] & 0x0F) == 0x01){
          // Wait
          lockout = false;
          delay(rxBuf[2]);
        } else if((rxBuf[0] & 0x0F) == 0x03){
          // Abort
          not_done = false;
          return;
        }
      }
    }

    if(((millis() - sepPrev) >= sepInvl) && lockout){
      sepPrev = millis();

      tpData[0] = 0x20 | index++;
      for(byte i=1; i<8; i++){
        if(offset != len)
          tpData[i] = data[offset++];
        else
          tpData[i] = 0x00;
      }
      
      // Do consecutive frames as instructed via flow frame
      CAN0.sendMsgBuf(REPLY_ID, 8, tpData);
      
      if(frames-- == 1)
        lockout = false;
        
    }

    if(offset == len)
      not_done = false;
    else{
      char msgstring[32];
      sprintf(msgstring,"Offset: 0x%04X\tLen: 0x%04X", offset, len);
      Serial.println(msgstring);
    }


    // Timeout
    if((millis() - sepPrev) >= 1000)
      not_done = false;
  }
  
}

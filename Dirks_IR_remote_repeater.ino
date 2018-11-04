/* Changes from original software code 
 * - void loop(): changed delay(1000) to delay(100)
 * - commented out all Serial.print statements 
 * - void loop(): commented out if (serial.read) 
 * - added GotOne = false after enableIRIn 
 * - added My_Receiver.blink13(true);
 */
/* Hardware setup:
 * A) Repeater itself: 
 * - Built around an ATMega328 (168 is too small, 14kB program, 1.2K sram)
 * - IR receiver diode (3-pin type, outputting raw signal, not modulated carrier-type): GND / 5V / data=D11 (atmega pin 17) 
 * - IR send diode (generic 2-pin IR diode): GND / D3 (atmega pin 5) 
 * - green BLINK led: GND / D13 (atmega pin 19) 
 * - 16 Mhz xtal pins 9/10 (with 2 x 22pF caps to Gnd) (only needed if ATMega was configured for external Xtal before) 
 * - pin 1 (reset) with 10K pull-up resistor to Vcc 
 * - pins 7 / 20 : Vcc
 * - pins 8 / 22 : Gnd
 * - 2 x 0.1uF decoupling capacitors across ATMega pins 7/8 and 20/22. 
 * B) To program the ATMega: 
 * - ATMega ICSP or serial programmer, depending on the initial ATMega configuration. 
 *      - if already containing bootloader and configured to use internal oscillator: serial programmer 
 *      - if no bootloader or if configured for use with external xtal: ICSP programmer 
 *      - Arduino UNO / Diecimila etc. can be used for both 
 * 
 * Programming the ATMega328: 
 * - Used instructions to re-configure to "breadboard AVR": 
 *    - Google "From Arduino to a Microcontroller on a Breadboard" 
 *    - Go to section "Minimal circuit (Eliminating external clock)" 
 *    - Download "breadboard-1-6-x.zip" which will add an entry in "Board" section 
 *    - Unzip into a folder "hardware" which you need to create in your Arduino folder 
 *    - Download "ArduinoISP" sketch into the Arduino Uno board which you will use as an ICSP programmer
 *    - Connect ATMega to Uno using 4 wires (MOSI-MISO-SCK-Reset) as well as GND/VCC. 
 *    - If ATMega was programmed/used before with external XTAL, bootloading needs a Xtal 
 *    - Once bootloader was loaded and configured as "internal clock", you can remove the Xtal 
 *    - ATMega now has bootloader and doesnt require Xtal. 
 *    - Downloading new sketch can be done using Rx/Tx pins on the ATMega 
 *    - If using laptop with USB port, a USB-to-RS232 adapter is needed. E.g. Arduino with CPU removed. 
 *    - If using laptop with serial port, a simple level-changer +-12v to 0/5V is enough (e.g. MAX323 chip). 
*/

/*
 *  To read remote control IR codes on serial port: 
 *    - Connect Rx/Tx on final PCB to UNO D0/D1 (=Rx/Tx) 
 *    - Connect UNO Gnd to PCB Gnd 
 *    - Remove ATMega from Uno board 
 *    - Connect UNO via USB cable to PC 
 *    - Use e.g. PUTTY, set up COM port (COM4) at 57600 
 *    - Press button on remote control and codes should be shown in Putty 
 */

/*
 *  To (re)program while on PCB: 
 *    - Remove ATMega from Uno board 
 *    - Connect Rx/Tx/Gnd from PCB to Uno (D0/D1/Gnd) 
 *    - Connect Reset on Uno to ATMega pin 1 on PCB
 *    - In Arduino IDE: set "Tools/Board" to "ATMega328 on breadboard (8MHz internal clock) 
 *    - Select "Tools/Programmer" to "Arduino as ISP" 
 *    - Upload sketch as per usual 
 */

/* Example program for from IRLib – an Arduino library for infrared encoding and decoding
 * Version 1.5  June 2014
 * Copyright 2014 by Chris Young http://cyborg5.com
 * Based on original example sketch for IRremote library 
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://www.righto.com/
 */
/*
 * IRrecord: record and play back IR signals 
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the appropriate pin.(See IRLibTimer.h) for your 
 * machine's timers and interrupts.
 * Record a value by pointing your remote at the device then send any character
 * from the serial monitor to send the recorded value.
 * Also demonstrates how to use toggle bits which must be controlled outside
 * the library routines.
 * The logic is:
 * If an IR code is received, record it.
 * If a serial character is received, send the IR code.
 */

#include <IRLib.h>

int RECV_PIN = 11;

bool Use_Serial = true; 

IRrecv My_Receiver(RECV_PIN);
IRdecode My_Decoder;
IRsend My_Sender;
/*
 * Because this version of the library separated the receiver from the decoder,
 * technically you would not need to "store" the code outside the decoder object
 * for this overly simple example. All of the details would remain in the object.
 * However we are going to go ahead and store them just to show you how.
 */
// Storage for the recorded code
IRTYPES codeType;          // The type of code
unsigned long codeValue;   // The data bits if type is not raw
int codeBits;              // The length of the code in bits

// These values are only stored if it's an unknown type and we are going to use
// raw codes to resend the information.
unsigned int rawCodes[RAWBUF]; // The durations if raw
int rawCount;                   //The number of interval samples

bool GotOne, GotNew; 

void setup()
{
  GotOne=false; GotNew=false;
  codeType=UNKNOWN; 
  codeValue=0; 
  My_Receiver.blink13(true); 
  if (Use_Serial) {
    Serial.begin(57600);
//    delay(2000);while(!Serial);//delay for Leonardo
//    Serial.println(F("Send a code from your remote and we will record it."));
//    Serial.println(F("Type any character and press enter. We will send the recorded code."));
  }
  My_Receiver.enableIRIn(); // Start the receiver
  Serial.println(F("Setup loop completed")); 
}

// Stores the code for later playback
void storeCode(void) {
  GotNew=true;
  codeType = My_Decoder.decode_type;
  if (codeType == UNKNOWN) {
    if (Use_Serial) {
      Serial.println("Received unknown code, saving as raw");
    }
    // To store raw codes:
    // Drop first value (gap)
    // As of v1.3 of IRLib global values are already in microseconds rather than ticks
    // They have also been adjusted for overreporting/underreporting of marks and spaces
    rawCount = My_Decoder.rawlen-1;
    for (int i = 1; i <=rawCount; i++) {
      rawCodes[i - 1] = My_Decoder.rawbuf[i];
    };
//    My_Decoder.DumpResults();
    codeType=UNKNOWN;
  }
  else {
    if (Use_Serial) {
      Serial.print(F("Received "));
      Serial.print(Pnames(codeType));
    }
    if (My_Decoder.value == REPEAT) {
      // Don't record a NEC repeat value as that's useless.
      if (Use_Serial) {
        Serial.println(F("repeat; ignoring."));
      }
     }
     else {
       codeValue = My_Decoder.value;
       if (codeValue == 0xE0E040BF) { codeValue = 0xFED02F; }   // change SkyCable on/off to Akai on/off 
       codeBits = My_Decoder.bits;
     }
    if (Use_Serial) {
     Serial.print(F(" Value:0x"));
     Serial.println(My_Decoder.value, HEX);
    }
  }
}
void sendCode(int repeat) {
  if(codeType== UNKNOWN) {
    // Assume 38 KHz
    My_Sender.IRsendRaw::send(rawCodes,rawCount,38);
    if (Use_Serial) {
      Serial.println(F("Sent raw"));
    }
    return;
  }
  if( !GotNew ) {//We've already sent this so handle toggle bits
    if (codeType == RC5) {
      codeValue ^= 0x0800;
    }
    else if (codeType == RC6) {
      codeValue ^= 0x10000;
    }
  }
  GotNew=false;
  My_Sender.send(codeType,codeValue,codeBits);
    if (Use_Serial) {
      Serial.print(F("Sent "));
      Serial.print(Pnames(codeType));
      Serial.print(F(" Value:0x"));
      Serial.println(codeValue, HEX);
    }
}

void loop() {
//  if (Serial.read() != -1) {
    if(GotOne) {
      sendCode(0);
      My_Receiver.enableIRIn(); // Re-enable receiver
      GotOne=false; 
    }
//  } 
  else if (My_Receiver.GetResults(&My_Decoder)) {
    My_Decoder.decode();
    GotOne=true;
    storeCode();
    delay(100);
    My_Receiver.resume(); 
  }
}


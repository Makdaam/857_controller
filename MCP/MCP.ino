#include <SoftwareSerial.h>
//Copyright Leszek Jakubowski 2017
//Distributed under Apache License 2.0

//Please read the README.md before using this code, it's about the safety of your radio

//pin which controls the LED
int pin_led = 13;
int pin = 0;
//pin connected to "white/blue" on the tuner, output 0 to start tune
int pin_start = 5;
//pin connected to the "brown/green" on the tuner, input 1 when tune done, bridged to PTT+tone in the radio
int pin_tuned = 6;
//pin connected to a button, button grounds the pin
int pin_button0 = 7;

byte a;
byte b;
byte c;
int addr = 0;
int r;
int tune_in_progress = 0;

SoftwareSerial catSerial(3, 2); // RX, TX

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the pins
  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_led, HIGH);
  pinMode(pin_start, OUTPUT);
  digitalWrite(pin_start, HIGH);
  pinMode(pin_tuned, INPUT);
  digitalWrite(pin_tuned, LOW);
  pinMode(pin_button0, INPUT);
  digitalWrite(pin_button0, HIGH);
  // initialize Command serial
  Serial.begin(57600);
  while (!Serial) {
    ;
  }
  Serial.println("READY");
  // initialize the CAT serial
  catSerial.begin(9600);
  
}

void sendCat(byte p1, byte p2, byte p3, byte p4, byte cmd)
{
  catSerial.write(p1);
  catSerial.write(p2);
  catSerial.write(p3);
  catSerial.write(p4);
  catSerial.write(cmd);  
}

byte readCat()
{
//1s timeout
  unsigned long start = millis();
  while (catSerial.available() == 0 && millis() - 1000 < start) {
  }
  return catSerial.read();

}

void clearCat()
{
  while(catSerial.available()) {
     catSerial.read();
  }
}

int readEEPROM(int address) {
  clearCat();
  sendCat((address >> 8) & 0xFF, (address & 0xFF), 0,0,0xBB);
  a = readCat();
  b = readCat();
  return verifyEEPROM(address);
}

int verifyEEPROM(int address) {
  clearCat();
  sendCat((address >> 8) & 0xFF, (address & 0xFF), 0,0,0xBB);
  if (a != readCat()) {
    return 1;
  }
  if (b != readCat()) {
    return 1;
  }
  return 0;
}  

void writeEEPROM(int address, byte value) {
  clearCat();
  r = readEEPROM(address);
  if (r==0) {
    a = value;
    sendCat((address >> 8) & 0xFF, (address &0xFF), value, b, 0xBC);
    delay(100); // give the radio some time to write the value, 100ms determined experimentally
    r = verifyEEPROM(address);
    if (r==1) {
      Serial.println("MEMORY CORRUPTED");
      Serial.println(addr);
    }  
  } else {
    Serial.println("ERROR READING");
  }
  Serial.println("Written");
}

void tune_proc() {
  if (tune_in_progress == 0){
    //set flag
    tune_in_progress = 1;
    //set LED
    digitalWrite(pin_led, LOW);
    byte original_power;
    //read power - address 155 decimal is where the TX Power is stored
    r= readEEPROM(155);
    Serial.println("Tuning");
    original_power = a;
    if(r!=0)
    {
      Serial.println("ERROR tune_proc read power");
      return;
    }
    //set power to 8W
    writeEEPROM(155,8);
    //start tune
    digitalWrite(pin_start, LOW);
    //wait for tune done or interrupt
    delay(1000);
    digitalWrite(pin_start, HIGH);
    r = digitalRead(pin_tuned);
    while(r=0)
    {
      delay(1000);
      r = digitalRead(pin_tuned);
    }
    //set power to original value
    writeEEPROM(155,original_power);
    //disable flag
    tune_in_progress = 0;
  }
}
// the loop routine runs over and over again forever:
void loop() {
  //blink the led
  if (pin)
  {
    pin = 0;
  } else {
    pin = 1;
  }
  digitalWrite(pin_led, pin);
  //check if button pressed, tune if it is
  if (digitalRead(pin_button0) == 0){
    tune_proc();
  }
  //change to interrupts later
  delay(500);
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    if (inChar == 's') {
    //scan all addresses and output their value
      for (addr=0; addr<7412; addr+=2) {
        r = readEEPROM(addr);
        if (r==0) {
          Serial.print(addr);
          Serial.print(",");
          Serial.print((int)a);
          Serial.print(",");
          Serial.println((int)b);
        } else {
          Serial.print(addr);
          Serial.print(",");
          Serial.print("ERROR");          
          Serial.print(",");
          Serial.print("ERROR");
        }
      }
    } else if (inChar == 'r') {
      //read a single byte (reads 2 bytes, prints them both), usage "r <decimal address>"
      delay(100);
      addr = Serial.parseInt();
      r = readEEPROM(addr);
      if (r==0) {
        Serial.print(addr);
        Serial.print(",");
        Serial.print((int)a);
        Serial.print(",");
        Serial.println((int)b);
      } else {
        Serial.print(addr);
        Serial.print(",");
        Serial.print("ERROR");          
        Serial.print(",");
        Serial.print("ERROR");
      }
    } else if (inChar == 'w') {
      //write a single byte, usage "w <decimal address> <decimal value>"
      delay(100);
      addr = Serial.parseInt();
      r = Serial.parseInt();
      if (addr != r) {
        Serial.println("ADDR ERROR");
        return;
      }
      r = Serial.parseInt();
      if (r != Serial.parseInt()) {
        Serial.println("VAL ERROR");
        return;        
      }
      writeEEPROM(addr,(byte)(r & 0xFF));
    } else if (inChar == 't') {
      //run tuning without the need for a button
      tune_proc();
    }
  }
  Serial.println(">");
}


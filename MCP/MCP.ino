#include <SoftwareSerial.h>

//pin which pulls TUNE do Ground when low
int pin_tune = 13;
int pin = 0;
byte a;
byte b;
byte c;
int addr = 0;
int r;

SoftwareSerial catSerial(3, 2); // RX, TX

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the TUNE pin
  digitalWrite(pin_tune, HIGH);
  pinMode(pin_tune, OUTPUT);

  // initialize Command serial
  Serial.begin(57600);
  while (!Serial) {
    ;
  }
  Serial.println("END OF LINE");
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
    delay(100); // give the radio some time to write the value
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

// the loop routine runs over and over again forever:
void loop() {
  if (pin)
  {
    pin = 0;
  } else {
    pin = 1;
  }
  digitalWrite(pin_tune, pin);
  delay(500);
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 

    if (inChar == 's') {
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
    }
  }
}


# 857 Controller
## Features
This is an Arduino board connected to a Yaesu FT-857 Radio, LDG AT-7000 Antenna Tuner (any other LDG Icom compatible tuner should work too) and optionally a PC or a mobile phone.

Planned features are:  

 * coordination of the tuning process (done)
 * a pass through CAT interface for the PC/mobile phone
 * sealed Pb battery charge/discharge monitoring

## Disclamer
Doing anything but the documented actions through the CAT interface can ZEROIZE your radio.
This does not mean factory reset, this means your **radio will be UNUSABLE**. That's why you're doing all this on your own responsibility.

Yaesu FT-8x7 radios keep their calibration data in the same EEPROM as other settings like TX Power, Frequency, beep volume etc. If a write to EEPROM corrupts the data it may cause a panic mode in the radio which will then erase the whole EEPROM.

To keep your radio safe write down all calibration parameters. These are individual to your radio, you will not find them anywhere on the Internet. You could go through the calibration procedure in the service manual if you have some expensive RF measuring equipment. It's easier to write them down while your radio is working correctly.  
  
## Backing up the calibration parameters
1. Get a pen and some paper
2. Place all PTT devices far away or connect dummy loads to both antenna outputs
3. Connect power to your FT-857
4. Turn the radio off
5. Press all three soft buttons below the display (A+B+C) and keep them pressed while turning the radio on.
6. You are now in the Soft Calibration mode, be careful
7. Use the *SELECT* knob (small one, bottom left of the radio) to select any one of the 74 calibration parameters
8. Write them all down
9. Power off the radio without touching the Func key - this will NOT SAVE any changes you might have made by accident

The Soft Calibration display shows the Adjustment number in the first line. The parameter name + Value in the second line. The third line usually displays a frequency and mode that's useful for calibration, follow the service manual to find out more.

## Hardware
![Schematic](https://github.com/Makdaam/857_controller/raw/master/schematics/schematic.png)

Connected GND on all devices, connected 13.8V from the radio to the tuner (assuming the Arduino has a separate power source). Please make sure there is not GND-GND voltage before connecting your PC to the Arduino.

All safety resistors are 680Ω, this gives us 20.2mA of current at 13.8V, so if something bad happens the worst case is 20mA going where it shouldn't and most modern digital devices can source and sink this current safely.

There is a 3.15A fuse inside the FT-857 on the 13.8V rail (not shown in the schematic).

Serial connections to FT-857 are straight with a simple safety resistor, these are TTL level RS-232 compatible. CAT interface is available there if you set the correct options in the radio menu. I assumed 9600 bps, you can set a slower speed.

PTT input in the radio is pulled up to 5V and sends a continuous CW tone when the input is pulled to ground. We are only reading the level here to figure out when the tuning sequence is complete.

Tune Start input on the Icom compatible LDG tuner is normally pulled up, pulling it to GND starts the tuning.

PTT out on the tuner is highZ usually and goes to GND when the tuner wants the radio to transmit a CW tone at around 10W. This plays nicely with the radios PTT in (beware of the power setting in the radio!).

The button grounds the pin_button0 pin and has a capacitor in parallel for debouncing, the capacity is not critical here and anything around 10-100nF will be fine (the higher the capacity, the longer it will take Arduino to notice the button is pressed).

## Software
The software presents a "READY" prompt when the board is rebooted, please use terminal which sends a whole line at a time for the best experience.

### Commands

**s** - scans through 7412 addresses and prints them all out, useful for looking where in memory a menu setting is stored

**r (decimal_address)** - reads two bytes from the radio EEPROM, the one stored at address and address+1

**w (decimal_address) (decimal_value)** - writes one byte to the EEPROM, it has a verification procedure in place, but still there is no CRC or any other error correction built into the interface, and you're running all this near an RF source, so watch out

**t** - start tuning from the console

**pushing button0** - start tuning

### Inner workings

TODO, please read the code, it's commented
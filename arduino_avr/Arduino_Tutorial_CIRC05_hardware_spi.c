/*
 * An extended version of Arduino example CIRC05
 * to use hardware SPI support in the Arduino:
 *
 *   http://blog.ringerc.id.au/2012/01/extending-arduino-example-circ-05-to.html
 * 
 */

#include <SPI.h>

/*     ---------------------------------------------------------
 *     |  Arduino Experimentation Kit Example Code             |
 *     |  (Altered to use hardware SPI by Craig Ringer)        |
 *     |  CIRC-05 .: 8 More LEDs :. (74HC595 Shift Register)   |
 *     ---------------------------------------------------------
 * 
 * We have already controlled 8 LEDs however this does it in a slightly
 * different manner. Rather than using 8 pins we will use just three
 * and an additional chip.
 *
 * This version of the example has been altered to show hardware SPI
 * use as well. It requires you to use output 11 where the wiring diagram
 * says to use output 2, and use output 13 instead of 3. No changes to the
 * wiring on the breadboard are required, just plug the wires into the 
 * different digital outputs on the Arduino as specified above.
 *
 */

boolean useSPI = true;

// Control pin Definitions
//
// The 74HC595 uses a serial communication link (SPI) which uses three pins: data, clock and latch.
//
// For compatibility with the Arduino's hardware SPI pins 11 and 13 are used for data and clock,
// though you can use any pins you like if you're using the software-only signalling shown
// in updateLEDs().
//
int data = 11; // SPI MOSI ; use this instead of pin 2
int clock = 13; // SPI SCLK ; use this instead of pin 3
int latch = 4; // SPI SS

int delayTime = 100; //the number of milliseconds to delay between LED updates
  
/*
 * setup() - this function runs once when you turn your Arduino on
 * We set the three control pins to outputs
 */
void setup()
{
  pinMode(latch, OUTPUT);
  digitalWrite(latch, LOW);
  if (useSPI) {
    // Transmit most significant bit first when sending data with SPI
    SPI.setBitOrder(MSBFIRST);
    SPI.begin();
  } else {
    pinMode(data, OUTPUT);
    pinMode(clock, OUTPUT);
  }
}

/*
 * loop() - this function will start after setup finishes and then repeat
 * we set which LEDs we want on then call a routine which sends the states to the 74HC595
 */
void loop()                     // run over and over again
{
  for(byte i = 0; i < 256; i++){
   if (useSPI) {
     updateLEDsSPI(i);
   } else {
     updateLEDs(i);
     //updateLEDsLong(i);
   }
   delay(delayTime); 
  }
}



/*
 * updateLEDs() - sends the LED states set in ledStates to the 74HC595
 * sequence
 */
void updateLEDs(byte value){
  digitalWrite(latch, LOW);     //Pulls the chips latch low
  shiftOut(data, clock, MSBFIRST, value); //Shifts out the 8 bits to the shift register
  digitalWrite(latch, HIGH);   //Pulls the latch high displaying the data
}

/*
 * updateLEDsLong() - sends the LED states set in ledStates to the 74HC595
 * sequence. Same as updateLEDs except the shifting out is done in software
 * so you can see what is happening.
 */ 
void updateLEDsLong(byte value){
  digitalWrite(latch, LOW);    //Pulls the chips latch low
  for(int i = 0; i < 8; i++){  //Will repeat 8 times (once for each bit)
    byte bit = value & B10000000; //We use a "bitmask" to select only the eighth 
                                 //bit in our number (the one we are addressing this time through
    value = value << 1;          //we move our number up one bit value so next time bit 7 will be
                                 //bit 8 and we will do our math on it
    if(bit == 128){
      digitalWrite(data, HIGH);
    } //if bit 8 is set then set our data pin high
    else
    {
      digitalWrite(data, LOW);
    }            //if bit 8 is unset then set the data pin low
    digitalWrite(clock, HIGH);                //the next three lines pulse the clock pin
    delay(1); // Make really, really sure the chip sees the change. Not really necessary.
    digitalWrite(clock, LOW);
  }
  digitalWrite(latch, HIGH);  //pulls the latch high shifting our data into being displayed
}

// The Arduino's SPI can be used to drive the IC instead of doing signalling
// in software with the shiftOut(...) library routine. SPI is faster and supports
// multiplexing multiple devices using shared data and clock signal pins.
// The Arduino platform provides built-in support for SPI.
// See http://arduino.cc/en/Reference/SPI
//
// This routine is functionally same as updateLEDs() in that it raises the latch,
// then for each bit sets the data pin and pulses the clock. It lowers
// the latch when all 8 bits are sent to apply the changes.
//
// The difference is that updateLEDs() uses the "transfer(...)" routine, a software
// routine in the Arduino library that sends that data using digitalWrite(...)
// calls. updateLEDsSPI() instead uses support for SPI built in to the 
// microcontroller in the Arduino to transmit the data, which is a LOT faster.
//
// To see just how much faster this is, comment out the delay() call in the main
// loop then compare how fast the LEDs flash with and without useSPI. With SPI
// you won't even be able to tell the LED for the most significant bit is
// flashing without adding a delay, it happens so fast.
//

void updateLEDsSPI(byte value) {
  // Select the IC to tell it to expect data
  digitalWrite(latch, HIGH);
  // Send 8 bits, MSB first, pulsing the clock after each bit
  SPI.transfer(value);
  // Lower the latch to apply the changes
  digitalWrite(latch, LOW);
}


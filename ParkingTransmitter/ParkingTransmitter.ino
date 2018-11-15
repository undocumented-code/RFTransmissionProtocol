#include <avr/sleep.h>
#include <avr/interrupt.h>

const int switchInput = 3;
const int signalTransmit = 4;
const short ID = 0x0001;
const int dataTransmitDelay = 50;

void sendMessage(unsigned int content) {
  //32-bit integer to send a full status update when something changes
  //Message:
  //  8 bits of preemble (10111111) to attenuate noise (char)
  //  16 bits of message (public key encrypted) (long)
  //  4 bits of message xor checksum
  //  4 bits of tail (1011)
  unsigned char checksum = 0x00;
  for(char i = 0; i < 4; i++) checksum ^= (((unsigned char) (content >> (4*i))) & 0x0F);
  unsigned long message = 0xBD00000B | ((unsigned long) (content & 0xFFFF)) << 8 | checksum << 4;
  digitalWrite(signalTransmit, LOW);
  delay(dataTransmitDelay * 2);
  for(char i = 31; i >= 0; i--) {
    digitalWrite(signalTransmit, HIGH);
    delay(dataTransmitDelay);
    if(message & ((unsigned long long) 1)<<i) delay(dataTransmitDelay);
    digitalWrite(signalTransmit, LOW);
    delay(dataTransmitDelay);
    if(!(message & ((unsigned long long) 1)<<i)) delay(dataTransmitDelay);
  }
}

// from https://bigdanzblog.wordpress.com/2014/08/10/attiny85-wake-from-sleep-on-pin-state-change-code-example/
void sleep() {
  GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT3);                   // Use PB3 as interrupt pin
  ADCSRA &= ~_BV(ADEN);                   // ADC off
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

  sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
  sei();                                  // Enable interrupts
  sleep_cpu();                            // sleep

  cli();                                  // Disable interrupts
  PCMSK &= ~_BV(PCINT3);                  // Turn off PB3 as interrupt pin
  sleep_disable();                        // Clear SE bit
  ADCSRA |= _BV(ADEN);                    // ADC on

  sei();                                  // Enable interrupts
} // sleep

ISR(PCINT0_vect) {
  // This is called when the interrupt occurs, but I don't need to do anything in it
}

void setup() {
  pinMode(switchInput, INPUT);
  digitalWrite(switchInput, HIGH);
  pinMode(signalTransmit, OUTPUT);
}

void loop() {
  sleep();
  sendMessage((ID << 1) | !digitalRead(switchInput));
}


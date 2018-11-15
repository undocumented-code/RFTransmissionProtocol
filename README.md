# RF Communcation Layered Example

## What does this run on?

The reference implementations here (if you can even call that) run using the Arduino Core. The transmitter is designed to work on an ATTiny device at 8MHz.
The receiver needs to be connected to the internet, and thus is based on an ESP8266 Arduino. I used the WeMos R1 Mini. 

## How does communication work?

The communication protocol was designed to be transmitted over the air, but can be done over the wire as well. It has message verification. See https://blog.tuckerosman.com/2018/11/building-communications-standard-from.html for more technical details.



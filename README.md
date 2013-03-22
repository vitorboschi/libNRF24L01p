libNRF24L01p
============

Library to implement low level access to Nordic NRF24L01+ 2.4GHz radio on Linux.

Current state
The library can read and write registers. I was able to configure the radio and detect data sent by other module using dataAvailable() method, and also read it using readData().
For tests, I'm using a Raspberry Pi in this library side, and an Arduino Nano running the excelent RF24 library, by maniacbug. You can get it here -> https://github.com/maniacbug/RF24

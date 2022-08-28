[README FR](/FR#récepteur-ssb-contrôlé-par-smartphone)

# Presentation

<br>The objective of my project is simple, to control the receiver with a smartphone rather than a touch LCD display. For this, I designed a HMI (Human Machine Interface) with a Wi-Fi access point and a web page. It would have been possible to make an Android application. However, the use of a web server directly integrated in the ESP32 microcontroller allows a more versatile use of different brands of smartphones.<br>

|  |  |
|--|--|
| The SSB receiver is composed of an ESP32 and a Si4735 integrated<br> circuit. Most of the time, this component (which costs only a few euros) <br>is controlled by an Arduino and a touch screen. The program I made allows to control  the receiver with a simple smartphone. This reduces the cost, the complete  receiver is reduced to a microcontroller ESP32 (equipped with a WiFi access point) and the SI4735. To program the ESP32 with a Si4735 I used the [Si4735](https://github.com/pu2clr/SI4735) library of PU2CLR which is very complete and very well described.| ![](/img/circuit_integre_test/img_montage.jpg) <br> Prototype used during my intership |

<br>SSB reception is made possible by a micro-code that is downloaded into the SI4735 at power-on. The integrated circuit works in the same way as an SDR (Software Defined Radio) receiver without the need for a computer. The selection of the reception frequency and the reception mode is done by the microcontroller via the I²C bus. The microcontroller does not realize audio processing, it is only used to control the SI4735 and to host the WEB page (the control interface of my project). Moreover the local oscillator of the SI4735 is clocked by a quartz of 32768 Hz.<br>

[FULL README](/EN#ssb-receiver-controlled-by-smartphone)

## License

This work is licensed under a
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License][cc-by-nc-sa].

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg

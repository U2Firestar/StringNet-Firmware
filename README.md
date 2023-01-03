# AVR / Arduino Firmware to act as a StringNet-Device

This part of the bachelor thesis project (RF-Bridge between a Smart Home and remote power outlets, over MQTT/Homie) is the firmware of the StringNet (own protocoll over UART / USB) - Device to enable (unidirectional) communication to RF-Devices. It receives commands via StringNet and acts accordingly.
Its' counterpart is [StringNet-Gateway](https://github.com/U2Firestar/StringNet-Gateway).

Used IDE is: PlatformIO using Arduino Framework

Features: 
- Imitation of RF-Transmitters for power outlets (sequences need to be analyzed via RF-receiver and oscilosope beforehand) 
- Easily portable to other microcontrollers similar to Arduino Nano
- Modular, reaction-based programm
- Dynamic database containing object-Information (Name, RF-Sequences, RFProtocol bzw. encoding und details, IDs)
- Changes to objects during operation possible

This project builds upon following librarys:
- [RC-Switch](https://github.com/sui77/rc-switch) from sui77 @ Github
- [OSFS](https://github.com/charlesbaynham/OSFS) von charlesbaynham @ Github

Therefor special thanks!

Versions:
- 1.0.0 - 08.2021 - Hardcoded initial version
- 2.0.0 - 04.2022 - Hardcoded to dynamic solution

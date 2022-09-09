# StringNet-RF-Gateway-Firmware

This part of the bachelor thesis project (MQTT/Homie-RF433-MHz-Bridge to remote power outlets) is the actuating end, easily attachable to a python and USB-capable server to enable communication to RF-Devices. It receives commands via StringNet (own protocoll over UART / USB) and acts accordingly.
Its' counterpart is [StringNet-RF-APIandGUI](https://github.com/U2Firestar/StringNet-RF-APIandGUI/)

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

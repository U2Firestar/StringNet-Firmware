# StringNet-RF-Gateway-Firmware
Technikum Wien BSA Thesis 2021/2022

This part of the bachelor thesis project is the actuating end to https://github.com/U2Firestar/StringNet-RF-APIandGUI/.
It's purpose is to receive command via StringNet (own protocoll over UART / USB) and act accordingly.

Features: 
- Imitation of RF-Transmitters for power outlets (sequences need to be analyzed via RF-receiver and oscilosope beforehand) 
- Easily portable to other microcontrollers similar to Arduino Nano
- Modular, reaction-based programm
- Dynamic database containing object-Information (Name, RF-Sequences, RFProtocol bzw. encoding und details, IDs)
- Changes to objects during operation possible

This project builds upon following librarys:
- RC-Switch from sui77 @ Github https://github.com/sui77/rc-switch
- OSFS von charlesbaynham @ Github https://github.com/charlesbaynham/OSFS

Therefor special thanks!

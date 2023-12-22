# StringNet-Firmware

**Features:**
- Listens and answers to commands over StringNet (own protocoll over UART / USB) from [StringNet-Gateway](https://github.com/U2Firestar/StringNet-Gateway)
- Imitation of unidirectional RF power outlet signals
- Modular, reaction-based, dynamic database containing object-Information (Name, RF-Sequences, RFProtocol bzw. encoding und details, IDs) with changes during operation allowed
- Addressing: Multiple StringNet-Devices on one bus possible

**Limitations:**
- RF-Sequences need to be analyzed via RF-receiver and oscilosope manually

**This project builds upon following librarys and therefor special thanks to:**
- [RC-Switch](https://github.com/sui77/rc-switch) from sui77 @ Github
- [OSFS](https://github.com/charlesbaynham/OSFS) von charlesbaynham @ Github


About: Project for Bachelorthesis, originally supported by UAS Technikum Vienna and 3S-Sedlak

IDE: PlatformIO + AVR / Arduino Framework

Versions:
- 1.0.0 - 08.2021 - Hardcoded initial version
- 2.0.0 - 04.2022 - Revised firmware to dynamic solution with FW Settings: Addressing disabled, Lifesign every 60sec

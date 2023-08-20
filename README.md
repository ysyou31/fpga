# fpga-rc
[Demo1](https://youtu.be/jwrR7UE4-a4)<br>
[Demo2](https://youtu.be/0usVB3tZH2E)<br>

This is RC Car working with PlayStation gamepad.

Sourcecodes in 'ps_remote/' foler are for the esp32. It will working as Remote Controller.
Need PlayStation Gamepad, UART RF-Transceiver and ESP32 borard.

Rest files are for the Quartus Prime. (Intel FPGA Programming tool)
RF-Transceiver module and FPGA are working as RF-receiver.
PlayStation Gamepad connection and Transmitting to the truck.ps_remote directory
Serial Rx decode module and PWM Generation module are in pwm.v
MKRVIDOR4000_top.v is main module.

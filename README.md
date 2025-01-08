# Amplifier Trigger Hub
Use 12V trigger from Denon AVR to switch NAD C352 on and off together.

This is a reimplementation of [this project](https://github.com/dennisfrett/AmplifierTriggerHub) for the RISC-V CH32V003 MCU.

## Detect Denon AVR power state
The Denon AVR has a 12V trigger output, that is high when it's on and low when it's off. Trigger is read through a voltage divider (30kΩ - 20kΩ, giving 12V -> ~4.8V) on `C2`.

## Switching NAD on / off
The NAD has an external IR in, which takes Extended NEC IR commands (without a carrier wave). Just connect a 3.5mm jack from the external IR input on the NAD to `D2`.

The IR signal is sent with [this direct NEC transmitter library](https://github.com/dennisfrett/CH32V-Direct-NEC-Transmitter).


![finished](https://github.com/user-attachments/assets/a54c109d-3b47-4f93-b10a-46e6772d3f42)

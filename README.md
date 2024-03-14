# pico-ir

Personal project for receiving and transmitting IR signals with the Raspberry Pi Pico.
It currently supports the NEC and 12-bit SIRC protocols.
This is not complete but a good reference for getting into programming IR receivers/LEDs.

## Compile rxir/txir

```bash
mkdir build
cd build
CMAKE_EXPORT_COMPILE_COMMANDS=true cmake -DPICO_BOARD=pico_w ..
```

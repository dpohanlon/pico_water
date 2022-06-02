# pico_water
Automated plant watering with the Raspberry Pi Pico. This runs a monitoring loop on the second RP2040 core to obtain moisture from an external moisture sensor, and temperature from the built in temperature sensor on the Pico board. Below a given moisture threshold, a pump is periodically engaged until the threshold is reached again. *Your mileage may very with the default thresholds!*

Build instructions
---

```console
mkdir build
cd build
cmake ../
make -j4
```

Inputs / Outputs
---

**PIN 0** : Output to the pump (HIGH on, LOW off).

**PIN 26** : ADC moisture sensor input.

**USB serial** : Sends moisture (ADC voltage) and temperature (Celcius) every 10 seconds.

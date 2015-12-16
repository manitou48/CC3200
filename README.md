## CC3200

Some example sketches and performance results for the TI CC3200 LAUNCHPAD 
using the Energia IDE.  The CC3200 is a CortexM4 running at 80mhz. 
The CC3200 is also used in the
[WiPy](http://wipy.io/)

|  Files    |  description |
| ------------- |-------------------------------------|
 etherperf.ino | various TCP/UDP wifi tests
 gpspps.ino  |  use GPS PPS to measure frequency of MCU crystal and 32khz crystal
 IRtest  | proof-of-concept of IR remote (timer PWM)
 mem2mem.ino | DMA memcpy
 spidma.ino | SPI+DMA
 rng.ino  | random entropy generatror from dueling clocks(systick/slow clock)
 slowclk.ino | slow clock tester, strange interference with delay() ??


------------------------------------

Some anecdotal performance comparions:

* [computational speed](https://github.com/manitou48/DUEZoo/blob/master/perf.txt)
* [power consumption] (https://github.com/manitou48/DUEZoo/blob/master/power.txt)
* [ISR latency] (https://github.com/manitou48/DUEZoo/blob/master/isrperf.txt)
* [SPI+DMA](https://github.com/manitou48/DUEZoo/blob/master/SPIperf.txt)
* [DMA memcpy](https://github.com/manitou48/DUEZoo/blob/master/mem2mem.txt)





# ADC_for_CH32V203
CH32 is an industrial-grade, low-cost, 32-bit general-purpose microcontroller from WCH based on the QingKe V4B RISC-V architecture.
• CPU Core: 32-bit RISC-V (QingKe V4B) running up to 144 MHz with zero-wait execution and 1-cycle multiply/divide.
• Memory: Typically features 10 KB to 20+ KB SRAM and 32 KB internal Flash, often paired with an external/on-package XIP (eXecute-In-Place) NOR flash configuration extending total storage up to 256 KB.
• Peripherals:
	• Dual USB interfaces (supporting both USB Host and USB Device)
	• CAN 2.0B interface
	• Multiple USARTs (up to 4), I2C, SPI, and dual OPA (operational amplifiers)
	• 12-bit ADC and 10-channel TouchKey
• Operating Voltage: Supports 3.3V (and 5V tolerant/compatible options depending on the package/board)
-------------------------------------------
in this code we will be initializing the 12-bit ADC peripheral already present in and using it in our main code by including ADC.c and ADC.h the output of the 12bit adc ranges from 0-4096 

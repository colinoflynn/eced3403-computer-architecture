# Assignment 3: Memory


##### 1: Memory Types in Practice [ 20 pts ]

For each of the following targets, make a table or description of the memory type(s) the microcontroller contains. For each of the memory types include the following characteristics if you are able to find them: (a) amount of memory, (b) organization (width), ( c ) access time or speed, and (d) erase/write cycles.

You should look for the following memory types:

* SRAM
* DRAM (rarely integrated into microcontrollers, but some SoCs may have this)
* EEPROM
* FLASH
* Fuse

Note that many microcontrollers will have ranges as the exact amount depend on the variant of the microcontroller. You can include the range or just specify a specific variant and choose those values. The following is the devices you must analyze for this question. Note you receive 1 point for every completed and correct subpoint. e.g., If you find the Flash amount (a), organization (b), and erase/write cycles (c) that is 3 points. There are no points for "not specified". Thus you may decide how much effort to put in for each different device to find the total required number of answers for this question (20 points).

1. ST STM32F415
2. Microchip ATMEGA328P
3. Raspberry Pi RP2350
4. Microchip PIC24FJ256GA702
5. Microchip ATSAM4LS4AA
6. Microchip AT91SAM9G25
7. Nuvoton NUC980DK7

As an example, for the STM32F303 (which is NOT in the above list, I used another one as an example) you would find the datasheet: [https://www.st.com/resource/en/datasheet/stm32f303cb.pdf](https://www.st.com/resource/en/datasheet/stm32f303cb.pdf). You might then notice this is somewhat short - for this device you also need the "reference manual": [https://www.st.com/resource/en/reference_manual/rm0316-stm32f303xbcde-stm32f303x68-stm32f328x8-stm32f358xc-stm32f398xe-advanced-armbased-mcus-stmicroelectronics.pdf](https://www.st.com/resource/en/reference_manual/rm0316-stm32f303xbcde-stm32f303x68-stm32f328x8-stm32f358xc-stm32f398xe-advanced-armbased-mcus-stmicroelectronics.pdf). Some devices will have all the answers in the datasheet, some will require both.

* SRAM:
  (a) 32 -- 48 kB (datasheet: table from page 12)
  (b) 32-bit wide (word) organization assumed
  (c) No access time specified - can run at full 72 MHz clock frequency
  (d) Not specified (SRAM, assumed infinite)

* DRAM: Not present.
 
* EEPROM: Not present.

* Flash:
  (a) 128 -- 256 kB (datasheet: table from page 12)
  (b) 64-bit wide, 2 Kbyte page size (reference manual: Section 4.2.1, page 67)
  (c) No access time specified - cannot run at full clock frequency however (72 MHz) so access time must be below that. You could infer the access time by analyzing the number of wait cycles required at different clock frequencies.
  (d) Erase/Write cycles: 10 000 (datasheet: Table 48, page 82)

* Fuse: Not present (contains flash "information block" but part of flash memory)


##### 2: AMAT Calculation [ 10 pts]

You are going to be asked to calculate the average memory access time (AMAT) for several scenarios in this question. See the slides for the AMAT formulas.

###### 2-1: Single Cache [ 1 pt ]

* Cache hit time = 1 cycle, miss rate = 10%
* Memory access = 100 cycles

###### 2-2: 2-Level Cache [ 1 pt ]

* L1 hit time = 1 cycle, miss rate = 5%
* L2 hit time = 10 cycles, miss rate = 20%
* Memory access = 100 cycles

###### 2-3: 3-Level Cache Option A [ 1 pt ]

* L1 hit time = 1 cycle, miss rate = 5%
* L2 hit time = 10 cycles, miss rate = 20%
* L3 hit time = 20 cycles, miss rate = 30%
* Memory access = 100 cycles

###### 2-4: 3-Level Cache Option B  [ 1 pt ]

* L1 hit time = 1 cycle, miss rate = 5%
* L2 hit time = 10 cycles, miss rate = 20%
* L3 hit time = 30 cycles, miss rate = 10%
* Memory access = 100 cycles

###### 2-5: Multi-Level Advantage [ 2 pts ]

Based on the single points above, what can you say about the difference of adding multiple levels to the cache?

Comment also on the difference between Option A & Option B. What difference in L3 cache between Option A and Option B would explain having a faster/slower cache but with a lower miss rate?

###### 2-6: AMAT Optimization [ 4 pts ]

Assume you have the following system:

* Memory access = 100 cycles
* Cache access time = 1 / miss_rate

The following shows some examples of the access time calculations:
```
Miss rate = 5% = 0.05
Access time = 1 / 0.05 = 20 cycles

Miss rate = 20% = 0.2
Access time = 1 / 0.2 = 5 cycles

Miss rate = 17% = 0.17
Access time = 1 / 0.17 = 5.9 cycles
```

Note that we accept non-integer access time (otherwise the best answer will be to approach the limit of either floor() or round()).

Create a 3-D plot or heatmap (or other visualization tool) to show how the AMAT changes as the miss rate (and associated access time) changes. This graph/map should have "Miss Rate" on one axes, and "Cache Access Time" on the other. The graph/map will then show the calculated AMAT.

You may need to use Python or other visuation tools to make this graph.

What is the optimal choice of miss rate in this system to minimize the AMAT?

##### 3: Cache Address Breakdown [ 5 pt ]

A system has:

* 32-bit address space
* 8 KB cache
* 64-byte cache lines
* 2-way set associative


Slide 40 from 4.4 included the K-Way associative mapping breakdown, but did not include the specific example. An example is available at [https://developer.arm.com/documentation/den0013/0400/Caches/Cache-architecture/A-real-life-example](https://developer.arm.com/documentation/den0013/0400/Caches/Cache-architecture/A-real-life-example) .

**Tasks:**

Calculate:
  * Number of sets
  * Tag bits
  * Set (Index) bits
  * Offset (Word) bits

Then show how the address `0xFEEDCAF8` is split between tag / set(index) / offset(word)

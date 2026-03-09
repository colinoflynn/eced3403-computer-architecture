# ECED3403 Lab3: Memory & DMA

Before beginning the lab, ensure you have:

1. Completed Lab 1, including setting up the development board
2. Attended or viewed the memory lecture, especially 4.1 and 4.2

This lab can be completed in groups of 1-3 and is self-organized. For the *lab portion* you need only *one submission* per group.

See the Lab Format document posted in Brightspace for submission details that apply to labs.

Note this lab is available in markdown from the course repository: [https://github.com/colinoflynn/eced3403-computer-architecture/tree/main/labs/lab3](https://github.com/colinoflynn/eced3403-computer-architecture/tree/main/labs/lab3). 

## Part 1: DMA Timing Comparisons

At the end of this section, you will have:

* [ ] Learned how to setup the DMA controller
* [ ] Time the difference between DMA copy and CPU copy for different sizes and optimization levels
* [ ] Considered the advantage of DMA for data copying

1. Create a new project (see previous lab). Replace the contents of `main.c` with the contents of the file `eced3403_lab3.c` from the repository linked above.

2. Build, debug, and run the code. Leave the compilation mode as "Debug" (optimizations off). It should compile as-is.

3. When running the code, you should see it print the number of cycles it took to complete a CPU copy vs. a DMA copy on the terminal. You can also observe the values of `cpu_cycles` and `dma_cycles`:

```CPU Copy took 676 cycles
DMA Cycles took 261 cycles
```

4. You can adjust the number of words copied by changing the line:
```c
#define BUF_WORDS 32
```

  For example you could change that to `#define BUF_WORDS 128` to copy 128 words. See the questions to answer - you will need to record the cycle count for the following word counts:
  
  * 4 words
  * 32 words
  * 256 words
  * XYZ words, where XYZ is the lower number of your student ID (do for each student in the group)

Repeat the above for both *DEBUG* and *RELEASE* builds. You will notice the CPU speed increases (and is often faster than the DMA results).

### Questions for Part 1

Answer the following questions about Part 1 of the lab:

1. Measure DMA time and provide the following data:

| Word Size                  | DMA Copy Cycles | CPU Copy Cycles | Build Mode |
|----------------------------|-----------------|-----------------|------------|
| 4                          | FILL IN         | FILL IN         | DEBUG      |
| 32                         | FILL IN         | FILL IN         | DEBUG      |
| 256                        | FILL IN         | FILL IN         | DEBUG      |
| FILL IN (student ID based) | FILL IN         | FILL IN         | DEBUG      |
| 4                          | FILL IN         | FILL IN         | RELEASE    |
| 32                         | FILL IN         | FILL IN         | RELEASE    |
| 256                        | FILL IN         | FILL IN         | RELEASE    |
| FILL IN (student ID based) | FILL IN         | FILL IN         | RELEASE    |

Include a graph of CPU vs. DMA cycle count for each situation (debug & release builds).

2. In the table above, the DMA is not always faster than the CPU. Why might using DMA still be an advantage?

## Part 2: Memory Protection

1.  For Part 2, you will reuse most of the code from Part 1. Remove most of the code from Part 1 as shown below, the easiest way to do this without deleting code is using a simple `#if 0` and matching `#endif` as below:

```c
#if 0
  /* -----------------------------------------
       PART 1 — CPU vs DMA copy timing
       ----------------------------------------- */

  //... REST OF PART 1 CODE ...

  //To complete part 2, delete part 1, comment this out, #if 0 this out
  while(1);
#endif

  /* -----------------------------------------
     PART 2 — MPU DMA Experiments
     ----------------------------------------- */
```

2. Run the code as-is - it should debug BUT you will hit a fault on running. Why fault is hit and why does this happen (observe the call to `mpu_enable_protected_region()`.

3. Comment out the offending line which causes the fault, but without disabling the MPU.

4. Using a call to `dma_memcpy_words()`, try to copy 1 word to the `secret_data` variable (HINT: you can try passing the address of `&secret_data` to DMA, or re-use the `dst_buf` from earlier and coy the result from that.

5. Observe that you are able to access the memory using the DMA engine **without** causing a fault (if you cause a fault you may have configured something incorrectly).

### Questions for Part 2

Answer the following questions about Part 2 of the lab:

1. Why did the code cause a fault initially? Specifically reference the MPU configuration used in the sample code.
2. What was your DMA access code?
2. What implications can you imagine for the DMA engine bypassing the MPU?

## Submission and Marking

The lab write-up should follow the format as posted in Brightspace - note you are primarily marked on answer to the lab questions. There is no need to duplicate the entire lab procedure, but include a short *reflections* section discussing any challenges you had or specific interesting observations outside of what was asked in the lab itself.
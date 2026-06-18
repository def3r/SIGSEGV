# HEXAGON DSP
- The Snapdragon 800 has two instances ofthe Hexagon digital-signal processor
  (DSP)
  - The modem (mDSP) is dedicated andcustomized for modem processing {**Closed
    source**}
  - the application DSP (aDSP) is used for mul-timedia acceleration
    {**Liscensed**}

- Hexagon is a multithreaded very longinstruction word (VLIW) DSP.
  -  designphilosophy is to maximize work per cycle forperformance, but target
     the microarchitec-ture to modest clock speeds and low power
- The VLIWapproach puts the burden of instruction par-allelism on the compiler
  and thereby avoidscostly and power-hungry dynamic-schedulinghardware

### Registers and memory
- The Hexagon processor features a unifiedbyte-addressable memory.
- This memory hasa single 32-bit virtual address space that holdsboth
  instructions and data
- operates inlittle-endian mode
- A full-featured memorymanagement unit (MMU) translates virtualto physical
  addresses
- two sets of user registers:general registers and control registers
  - general registers include thirty-two 32-bitregisters
  - general registers contain all pointer, sca-lar, vector, and accumulator
    data
  - con-trol registers include special-purpose registerssuch as the program
    counter, status register,and loop registers
- two identical 64-bit single-instruction, multiple-data (SIMD) executionunits
  - Each unit supports all multiply, shift,arithmetic logic unit (ALU), and bit
    manipu-lation instructions
- The Hexagon ISA includes conditionalexecution
- Similar to many DSP processors, Hexa-gon includes a zero-overhead
  hardwarecounted looping mechanism with supportfor two levels of nesting
  - This architecture allows execution of loops with no branch mispredicts or
    stalls, and no hard-ware devoted to loop branch prediction

### Compound and memop instructions
- Compound instructions combine two ormore dependent operations in a single
  instruc-tion.
- These instructions improve code size andsave power by reducing register file
  and for-warding power.
- class of instruction performs sim-ple operations directly on memory,
  includingadd, subtract, logical–or, and logical–and
- Without these memory operations (mem-ops), three instructions would be
  necessary toperform the same task
- Memops improve code size and reduce powerbecause intermediate register access
  is notneeded.

### VLIW instruction grouping
- VLIW instruction packets are variablesized and contain one to four
  instructions
- Ifa packet contains more than one instruction,the instructions execute in
  parallel

## Multithreading and microarchitecture
- Early implementations included sixhardware threads, but more recent
  coresinclude three hardware threads
  - There aremany trade-offs in choosing the number ofthreads
  - Additional threads provide morelatency tolerance and enable
    power-savingopportunities in the microarchitecture byserializing work
    rather than speculatingwork
  - additional threadsincrease cache pressure and increase the soft-ware
    programming burden.
- Hexagon is designed to look like a multi-core architecture with
  communicationthrough shared memory
- With the number of threads matched tothe execution pipe depth, all of a
  thread’sinstructions from a VLIW packet are com-plete before the next VLIW
  packet starts
- Because there is no observable latency, thecompiler is not concerned with
  instructionlatency and scheduling for latency. Thisyields higher VLIW packet
  density.
- The obvious problem with IMT(Interleaved Multithreading) is thatwhen threads
  are idle or stalled, their sliceof the processor goes unused
  - Often, packets contain only simple instruc-tions and can be completed in
    fewer thanthree cycles
  - With Hexagon V5, the pro-cessor will opportunistically execute
    packetsfaster if threads are idle or stalled and simplepackets are
    available

## System programming model
- Communication between the DSP andCPU is done through a traditional
  shared-memory-plus-interrupt mechanism
- Access to memory is cache based, and there isno explicit data mover. The DSP
  includes anextensive prefetching capability to help hidecache latency.
- The CPU and DSP are notcache coherent with each other, so coherencymust be
  maintained in software with explicitcache maintenance operations.
  - A software remote procedure call (RPC)interface lets a CPU application
    offload workto the DSP.
  - When an RPC is made, any dataassociated with the call is flushed to
    mainmemory from the CPU caches and mappedinto the DSP virtual address space
  - The DSPis then interrupted to process the RPC call
  - any results are flushed from theDSP caches back to main memory
  - completion interrupt is sent to the CPU

## Power
- Hexagon is imple-mented with aggressive low-power designtechniques, including
  hierarchical clock gat-ing with a custom clock tree, voltage scalingwith
  split-grid memories, pulse latches instead of flip-flops, and full-custom
  cachesand register files designed for low power
- An important power benchmark formobile phones is MP3 playback


- Camera and video applications requiresophisticated signal processing at
  ultra-highdefinition resolution
- “always-on” voice activation features arepushing power requirements to new
  lows
- the Hexagon Software Developer’sKit
  (https://developer.qualcomm.com/mobile-development/maximize-hardware/multimedia-optimization-hexagon-sdk/multimedia-optimization-h-2)
  provides everythingneeded to program the DSP, includingfull documentation,
  software tools, acycle-approximate simulator, and examplecode

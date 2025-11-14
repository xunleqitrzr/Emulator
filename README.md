# \[untitled\] Emulator

## This is an 8-Bit emulator

### The Emulator consists of:
- an 8-Bit CPU:
  - with variable amounts of registers
  - smaller instruction set than MISC
  - fully functioning flag logic 
  - NO support for negative numbers


- variable size RAM:
  - as fast as your RAM is
  - extendable in code


- a ROM:
  - given program sits inside the ROM 
  - during startup, the ROM gets load into RAM


- a non-implemented bus system
  - not implemented yet 
  - yeah, not much to say here

### Design
- the emulator emulates a 6502-like CPU
- register `A` is considered to be the accumulator
  - `INC` and `DEC` for example perform their operations on the `A` register
- this is just a personal project, it has it's upsides and downsides

![Picture of the design](doc/arch.svg)

### Important:
There are no security implementations yet. <br>
You are able to modify the code from within the code itself. <br>
This may sound stupid, but it is a design choice, giving developers the <br> amount of freedom and control they need.
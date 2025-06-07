# \[untitled\] Emulator

## This is an 8-Bit emulator

### The Emulator consists of:
- an 8-Bit CPU:
  - a whopping of two registers!
  - smaller instruction set than MISC
  - two flags (ZF, CF)
  - NO support for negative numbers


- 64 KiB of RAM:
  - as fast as your RAM is
  - extendable in code


- a ROM:
  - your program sits inside the ROM 
  - during startup, the ROM gets load into RAM


- a non-implemented bus system
  - not implemented yet 
  - yeah, not much to say here


### Important:
There are no security implementations yet. <br>
You are able to modify the code from within the code itself. <br>
This may sound stupid and it is. But it is too early to implement any security.
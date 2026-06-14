# \[untitled\] Emulator

## This is an 8-Bit emulator

### The Emulator consists of:
- an 8-Bit CPU:
    - 16 general-purpose registers (A through P)
    - smaller instruction set than MISC
    - fully functioning ALU and flag logic (Zero, Sign, Carry, Overflow)
    - negative number support during comparisons

- RAM and ROM:
    - 64 KiB addressable memory space
    - given program sits inside the ROM
    - during startup, the ROM gets loaded into RAM

- a centralized Bus System:
    - routes memory reads and writes
    - implements Memory-Mapped I/O (MMIO)
    - handles privilege levels (User Mode vs. System Mode)

- virtual Hardware Peripherals:
    - GPU: 32x8 text-mode visual buffer mapped to 0x4000
    - RNG: hardware random number generator mapped to 0x9000
    - Timer: system clock counter mapped to 0xF000

### Design
- the emulator features a Von Neumann architecture with a 6502-like feel
- includes a custom two-pass assembler (`easm`) to compile `.asm` files into `.bin` executables
- memory operations support direct and indirect indexed addressing
- this is just a personal project, it has its upsides and downsides

![Picture of the design](doc/arch.svg)

### Building and Running the emulator
Everything _should_ work out of the box.

**For UNIX-based systems:**<br>
Execute
```bash
  git clone https://github.com/xunleqitrzr/Emulator.git
  make all
```
inside the project root directory to build the emulator with Release **and** Debug configurations.<br>
The build files are inside the `build/` directory. It also contains an `easm` executable
which<br>is an assembler used to assemble programs to machine code instructions from a given<br>
assembly file.

Assemble a program with
```bash
  ./easm <input.asm> <output.bin>
```
and execute it with
```bash
  ./EmulatorRelease <program.bin>
```

**For Windows systems:**<br>
_Prerequisites_:
- Visual Studio Version 17 (e.g. 2022)
- git

_Procedure_:
1. Open the **"Developer Command Prompt for VS"**
2. clone the repository
    ```bash
    git clone https://github.com/xunleqitrzr/Emulator.git
    ```
3. navigate into the repository directory
4. Build with MSBuild:
   ```bash
   cmake -B build -G "Visual Studio 17 2022"
   cmake --build build --config Debug          # for Debug builds 
   cmake --build build --config Release        # for Release builds
   ```
5. Build with Visual Studio:
    1. open the solution file `Emulator.sln` inside `build/` and build with Visual Studio
6. (for both options:) the binaries are inside `build/<configuration>`, either `Debug` or `Release`


### Important:
There is a dedicated memory protection security module.<br>
- It strictly prevents the program from overwriting its own executable code in memory.<br>
- It enforces write privileges: user-space code cannot write to the reserved stack area, whereas system-level operations (like `PUSH` and `CALL`) are permitted.
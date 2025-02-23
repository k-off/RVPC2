# RVPC2

Inspired by [OLIMEX RVPC](https://github.com/OLIMEX/RVPC) project, I decided to create a #higher-faster-stronger version.

Key differences:

|Feature|V1|V2|
|-|-|-|
|**MCU**|||
|Model|CH32V003F4P6|CH32X035G8R6|
|Pins|8|28|
|RAM|2k|20k|
|ROM|16k|64k|
|USB| X | **V** |
|**Display**|||
|Color|mono (1-bit)| 8 (3-bit)|
|Communication| X | **V** |
|**Ports**|||
|PS2| **V** | X |
|USB| X | **2.0 FS** |
|Audio| X | 3.5mm jack |
|Storage| X | Micro-SD |
|Power| 5V, 5mm barrel| 5V, USB-C PD|

![preview](/pcb/pcb/v0.1-3d.png)

## Build tools:

 - IDE of your choice:
   - [MounRiver](http://www.mounriver.com/download)
   - [VSCode](https://code.visualstudio.com/download)
 - Compiler (in case it isn't bundled with ide): [riscv-none-elf-gcc-xpack](https://xpack-dev-tools.github.io/riscv-none-elf-gcc-xpack/docs/install/) instructions page

## Troubleshoot

In case MounRiver build fails with `Bad CPU type in executable`, try:
 - right click on project name in project explorer (left top corner of the window)
 - select `Properties`
 - `Build Settings`->`Target Processor`->`RISC-V Compiler`: select `WCH Toolchain(GCC12)` or newer

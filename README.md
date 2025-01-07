# CH32X035

CH32X035 RISC-V MCU family documentation and demos

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

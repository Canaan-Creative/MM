mm - Miner Manager
===

Miner Manager is the Avalon2 bitcoin mining machine's FPGA controller.

Directory Structure
===

* `docs`: various design documentation.
* `firmware`: code running in LatticeMico32 soft processor.
* `ipcores_dir`: ip cores
* `sim`:
* `synth`: directory for synthesize and build of the hardware part.
* `verilog`: HDL.

How to build?
===

First you need install the ISE for sure. then edit the `isedir` under xilinx.mk
by default we are using /home/Xilinx/14.6/ISE_DS/

1. $ make -C firmware/toolchain # Install the lm32-rtems-4.11- toolchain under /opt
2. $ make -C firmware           # Generate the final bitstream file .bit/.mcs under firmware/

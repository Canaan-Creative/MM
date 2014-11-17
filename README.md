MM - Miner Manager
==================

Miner Manager is a bitcoin task generator firmware that fit FPGA and faster mining machine

Downloads
=========
* Avalon4 MM firmware: http://downloads.canaan-creative.com/software/avalon4/mm/
* Avalon3 MM firmware: http://downloads.canaan-creative.com/software/avalon3/mm/
* Avalon2 MM firmware: http://downloads.canaan-creative.com/software/avalon2/mm/

Datasheet
=========
* http://downloads.canaan-creative.com/software/mm/

Main objectives
=============
* It is using stratum protocol
* Generator the task inside FPGA. all Double-SHA256 was done by FPGA. far more faster then CPU
* Test the nonce inside the FPGA. only report the >= DIFF taskes back to host (cgminer)
* It fit any kinds of bitcoin mining ASIC, (you may needs some VerilogHDL coding)
* It have LM32 CPU inside, fit in XC6SLX16 small FPGAs

Directory structure
===================

* `firmware`: C code running in LatticeMico32 soft processor
* `ipcores_dir`: IP cores
* `synth`: Directory for synthesize and build of the hardware part
* `verilog`: The VerilogHDL source code

How to build?
=============

First you need install the ISE for sure. then edit the `isedir` under xilinx.mk
by default we are using /home/Xilinx/14.6/ISE_DS/

1. $ make -C firmware/toolchain # Install the lm32-rtems-4.11- toolchain under /opt
2. $ make -C firmware           # Generate the final bitstream file .bit/.mcs under firmware/
3. $ make -C firmware load      # Load the config bit file to FPAG by using Xilix Platform cable

Discussion
==========
* IRC: #avalon @freenode.net
* Mailing list: http://lists.canaan-creative.com/
* Documents: https://en.bitcoin.it/wiki/Avalon
* Downloads: http://downloads.canaan-creative.com

Support
=======
* Avalon2 machine (105GHs, 315GHs) https://en.bitcoin.it/wiki/Avalon2
* Avalon3 machine (300GHs, 800GHs) https://en.bitcoin.it/wiki/Avalon3
* Avalon3 card (? GHs)
* Avalon4 machine (1THs)	   https://en.bitcoin.it/wiki/Avalon4

License
=======

This is free and unencumbered public domain software. For more information,
see http://unlicense.org/ or the accompanying UNLICENSE file.

Files under verilog/superkdf9/ have their own license (Lattice Semi-
conductor Corporation Open Source License Agreement).

Some files may have their own license disclaim by the author.

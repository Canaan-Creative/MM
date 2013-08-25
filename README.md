mm
==

Miner Manager

Directory Structure
---

* `docs`: various design documentation.
* `synth`: directory for synthesize and build of the hardware part.
* `verilog`: HDL.
* `firmware`: code running in LatticeMico32 soft processor.

License
===

BSD? MIT? GPL??

How to build?
===

$ cd synth
$ vi xilinx.mk # adjust isedir and perhaps `xil_env`
$ make
$ cd ../firmware
$ make
$ make -C ../synth # rebuild mcs
$ cd ../software
$ make

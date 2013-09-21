mm
==

Miner Manager

Directory Structure
---

* `docs`: various design documentation.
* `firmware`: code running in LatticeMico32 soft processor.
* `ipcores_dir`: ip cores
* `sim`:
* `synth`: directory for synthesize and build of the hardware part.
* `verilog`: HDL.

License
===

BSD? MIT? GPL? UNLICENSE?

How to build?
===

$ vi xilinx.mk # adjust isedir and perhaps `xil_env`
$ make -C synth
$ CROSS=/opt/lm32/bin/lm32-rtems4.11- make -C firmware
$ make -C synth # rebuild mcs
$ CROSS=/opt/lm32/bin/lm32-rtems4.11- make -C firmware

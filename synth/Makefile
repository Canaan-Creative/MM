project := mm
vendor := xilinx
intstyle := -intstyle xflow
family := spartan6
device := xc6slx16
speed := -2
package := ftg256
top_module := mm
has_bmm := y
SOFTWARE_ELF := ../firmware/main.elf
promgen_extra_opts := -spi
bitgen_extra_opts := -g DriveDone:yes -g StartupClk:Cclk -g ConfigRate:22 -g LCK_cycle:6 -bd $(SOFTWARE_ELF)
bitgen_extra_deps := $(SOFTWARE_ELF)

_default_: $(project).bit $(project).mcs $(project)_err.twr

vfiles_nodir := \
	include/.dummy.v\
	xilinx/bb.v\
	xilinx/pmi_addsub.v\
	xilinx/pmi_fifo.v\
	xilinx/bram.v\
	xilinx/clkgen.v\
	superkdf9/soc/superkdf9.v\
	main.v

xilinx_cores := $(addprefix ../ipcores_dir/,\
	icon.xco\
	ila.xco\
	vio.xco\
	fifo_uart.xco\
	)

vfiles := $(addprefix ../verilog/, $(vfiles_nodir))

include xilinx.mk

$(SOFTWARE_ELF):
	@echo "please do make in ../firmware"
	@false

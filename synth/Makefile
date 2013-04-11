project := mm
vendor := xilinx
intstyle := -intstyle xflow
family := spartan6
device := xc6slx16
speed := -2
package := ftg256
top_module := mm
has_bmm :=
promgen_extra_opts := -spi
bitgen_extra_opts := -g DriveDone:yes -g StartupClk:Cclk -g ConfigRate:22 -g LCK_cycle:6

_default_: $(project).bit $(project).mcs $(project)_err.twr

vfiles_nodir := \
	include/.dummy.v\
	lattice/bb.v\
	lattice/pmi_addsub.v\
	lattice/pmi_ram_dp_true.v\
	lattice/pmi_fifo_dc.v\
	lattice/pmi_fifo.v\
	lattice/pmi_distributed_dpram.v\
	superkdf9/soc/superkdf9_simple.v\
	main.v

xilinx_cores := $(addprefix ../ipcores_dir/,\
	icon.xco\
	ila.xco\
	fifo_uart.xco\
	)

vfiles := $(addprefix ../verilog/, $(vfiles_nodir))

include xilinx.mk

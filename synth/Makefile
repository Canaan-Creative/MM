project := mm
vendor := xilinx
intstyle := -intstyle xflow
family := spartan6
device := xc6slx16
speed := -2
package := ftg256
top_module := mm
has_bmm := y
SOFTWARE_ELF := ../firmware/mm.elf
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
	superkdf9/components/spi/wb_spi.v\
	superkdf9/components/uart_core/rxcver.v\
	superkdf9/components/uart_core/txcver_fifo.v\
	superkdf9/components/uart_core/intface.v\
	superkdf9/components/uart_core/uart_core.v\
	superkdf9/components/uart_core/txmitt.v\
	superkdf9/components/uart_core/rxcver_fifo.v\
	superkdf9/components/uart_core/modem.v\
	superkdf9/components/gpio/tpio.v\
	superkdf9/components/gpio/gpio.v\
	superkdf9/soc/superkdf9.v\
	superkdf9/soc/superkdf9_simple.v 

xilinx_cores := $(addprefix ../ipcores_dir/,\
	fifo_uart.xco\
	tx_fifo.xco\
	rx_fifo.xco\
	icon.xco\
	ila.xco\
	)

vfiles := $(addprefix ../verilog/, $(vfiles_nodir))

include xilinx.mk

$(SOFTWARE_ELF):
	make -C `dirname $@` `basename $@`

# Xilinx General Makefile, ver 1.1
# The top level module should define the variables below then include
# this file.  The files listed should be in the same directory as the
# Makefile.  
#
#   variable           description
#   ----------         -------------
#   project            project name (top level module should match this name)
#   top_module         top level module of the project
#   libdir             path to library directory
#   libs               library modules used
#   vfiles             all local .v files
#   xilinx_cores       all local .xco files
#   vendor             vendor of FPGA (xilinx, altera, etc.)
#   family             FPGA device family (spartan3e) 
#   device             FPGA device name (xc4vfx12)
#   speed              FPGA speed grade (-10)
#   package            FPGA package (-sf363)
#   flashsize          size of flash for mcs file (8192)
#   optfile            (optional) xst extra opttions file to put in .scr
#   map_opts           (optional) options to give to map
#   par_opts           (optional) options to give to par
#   intstyle           (optional) intstyle option to all tools
#   xst_extra_opts     (optional) options given to xst (e.g.: "-define {DEUBG=1}")
#   promgen_extra_opts (optional) options given to promgen (e.g.: "-spi")
#   bitgen_extra_opts  (optional) options given to bitgen (e.g.: "-g XXX:yes")
#   bitgen_extra_deps  (optional) optional prerequisites for bitgen (e.g.: 
#                                 "soft.elf")
#   has_bmm            (optional) define to non nil to enable bmm processing
#
#   files              description
#   ----------         ------------
#   $(project).ucf     ucf file (override by $(ucf_file))
#   $(project).bmm     initial bmm file (has_bmm should be non nil)
#                      (override by $(bmm_file))
#
# Library modules should have a modules.mk in their root directory,
# namely $(libdir)/<libname>/module.mk, that simply adds to the vfiles
# and xilinx_cores variable.
#
# all the .xco files listed in xilinx_cores will be generated with core, with
# the resulting .v and .ngc files placed back in the same directory as
# the .xco file.
#
# Note: xco file are device dependent, so use __DEVICE__, __PACKAGE__, __SPEED__, __FAMILY__
#       and __DIR__ (refering to the directory where xco file lives) variable instead.
#
# TODO: how to generate mig for spartan 6?

SHELL = /bin/bash

part := $(device)$(speed)$(package)

coregen_work_dir ?= ./coregen-tmp
map_opts ?= -timing -ol high -detail -pr b -register_duplication -w -mt 2
par_opts ?= -ol high -mt 4
isedir ?= /home/Xilinx/14.6/ISE_DS/
xil_env ?= . $(isedir)/settings$(shell getconf LONG_BIT).sh &>/dev/null
flashsize ?= 8192
ucf_file ?= $(project).ucf
bmm_file ?= $(project).bmm

libmks := $(patsubst %,$(libdir)/%/module.mk,$(libs)) 
mkfiles := Makefile $(libmks) xilinx.mk
include $(libmks)
junks :=

corengcs := $(foreach core,$(xilinx_cores),$(dir $(core))cache/$(part)/$(notdir $(core:.xco=.ngc)))
local_corengcs := $(foreach ngc,$(corengcs),$(notdir $(ngc)))
vfiles += $(foreach core,$(corengcs),$(core:.ngc=.v))
junk += $(local_corengcs)

userid := $(shell git rev-parse HEAD | cut -b1-8)

.PHONY: default xilinx_cores twr etwr clean distclean

default: $(project).bit $(project).mcs
xilinx_cores: $(corengcs)
twr: $(project).twr
etwr: $(project)_err.twr

# some utilities: unique {{{
_unique=$(if $(subst $(_unique_last),,$(1)),$(1),)$(eval _unique_last:=$(1))
unique=$(eval _unique_last:=)$(foreach i,$(sort $(1)),$(call _unique,$(i)))
# }}}

vlgincdir := $(call unique,$(dir $(vfiles)))

define cp_template
$(2): $(1)
	cp $(1) $(2)
endef
$(foreach ngc,$(corengcs),$(eval $(call cp_template,$(ngc),$(notdir $(ngc)))))

define coregen
$(1).ngc $(1).v: $(2).xco
	@echo "=== rebuilding $$@"
	if [ -d $$(coregen_work_dir) ]; then \
		rm -rf $$(coregen_work_dir)/*; \
	else \
		mkdir -p $$(coregen_work_dir); \
	fi
	cd $$(coregen_work_dir); \
	$(xil_env); \
	sed  -e 's/# BEGIN Project Options/NEWPROJECT DUMMY/' -e 's/__DEVICE__/$(device)/' -e 's/__PACKAGE__/$(package)/' -e 's/__FAMILY__/$(family)/' -e 's/__PACKAGE__/$(package)/' -e 's/__SPEED__/$(speed)/' -e "s|__DIR__|$$$$OLDPWD/$$(patsubst %/,%,$$(dir $$<))|" "$$$$OLDPWD/$$<" > dummy.xco; \
	coregen -b dummy.xco; \
	cd -
	xcodir=`dirname $$<`; \
	basename=`basename $$< .xco`; \
	if [ ! -r $$(coregen_work_dir/DUMMY/$$$$basename.ngc) ]; then \
		echo "'$$@' wasn't created."; \
		exit 1; \
	else \
		mkdir -p $$$$xcodir/cache/$(part); \
		cp $(coregen_work_dir)/DUMMY/$$$$basename.v $(coregen_work_dir)/DUMMY/$$$$basename.ngc $$$$xcodir/cache/$(part)/; \
	fi
endef
$(foreach core,$(corengcs),$(eval $(call coregen,$(basename $(core)),$(basename $(subst /cache/$(part),,$(core))))))
junk_cache += $(call unique,$(foreach core,$(corengcs),$(subst $(part)/,,$(dir $(core)))))
junk += $(coregen_work_dir)
products =

date = $(shell date +%F-%H-%M)

# some common junk
junk += *.xrpt

programming_files: $(project).bit $(project).mcs
	mkdir -p $@/$(date)
	mkdir -p $@/latest
	for x in .bit .mcs .cfi _bd.bmm; do cp $(project)$$x $@/$(date)/$(project)$$x; cp $(project)$$x $@/latest/$(project)$$x; done
	$(xil_env); xst -help | head -1 | sed 's/^/#/' | cat - $(project).scr > $@/$(date)/$(project).scr

$(project).mcs: $(project).bit
	$(xil_env); \
	promgen -w $(promgen_extra_opts) -s $(flashsize) -p mcs -o $@ -u 0 $^
junk += $(project).cfi $(project).prm
products += $(project).mcs

$(project).bit: $(project)_par.ncd $(bitgen_extra_deps)
	$(xil_env); \
	bitgen $(intstyle) $(bitgen_extra_opts) -g UserID:$(userid) -w $(project)_par.ncd $(project).bit
	echo $(part) > .curr_part
junk += $(project).bgn $(project).drc $(project)_bitgen.xwbt
products += $(project).bit $(project)_bd.bmm

$(project)_par.ncd: $(project).ncd
	$(xil_env); \
	if par $(intstyle) $(par_opts) -w $(project).ncd $(project)_par.ncd; then \
		:; \
	else \
		$(MAKE) etwr; \
	fi 
junk += $(project)_par.ncd $(project)_par.par $(project)_par.pad 
junk += $(project)_par_pad.csv $(project)_par_pad.txt 
junk += $(project)_par.grf $(project)_par.ptwx
junk += $(project)_par.unroutes $(project)_par.xpi

$(project).ncd: $(project).ngd
	if [ -r $(project)_par.ncd ]; then \
		cp $(project)_par.ncd smartguide.ncd; \
		smartguide="-smartguide smartguide.ncd"; \
	else \
		smartguide=""; \
	fi; \
	$(xil_env); \
	map $(intstyle) $(map_opts) $$smartguide $<
junk += $(project).ncd $(project).pcf $(project).ngm $(project).mrp $(project).map
junk += smartguide.ncd $(project).psr 
junk += $(project)_summary.xml $(project)_usage.xml

ifneq ($(has_bmm),)
$(project).ngd: $(project).ngc $(ucf_file) $(bmm_file)
	$(xil_env); ngdbuild $(intstyle) $(project).ngc -bm $(bmm_file)
else
$(project).ngd: $(project).ngc $(ucf_file)
	$(xil_env); ngdbuild $(intstyle) $(project).ngc
endif
junk += $(project).ngd $(project).bld

$(project).ngc: $(vfiles) $(local_corengcs) $(project).scr $(project).prj
	$(xil_env); xst $(intstyle) -ifn $(project).scr
junk += xlnx_auto* $(top_module).lso $(project).srp 
junk += netlist.lst xst $(project).ngc

$(project).prj: $(vfiles) $(mkfiles)
	for src in $(vfiles); do echo "verilog work $$src" >> $(project).tmpprj; done
	sort -u $(project).tmpprj > $(project).prj
	rm -f $(project).tmpprj
junk += $(project).prj

optfile += $(wildcard $(project).opt)
top_module ?= $(project)

ifneq ($(shell cat .xst_extra_opts 2>/dev/null),$(shell echo "$(xst_extra_opts)" | md5sum))
 $(warning xst_extra_opts changed, force regen)
 $(shell touch ./xilinx.opt)
endif
$(project).scr: $(optfile) $(mkfiles) ./xilinx.opt
	echo "run" > $@
	echo "-p $(part)" >> $@
	echo "-top $(top_module)" >> $@
	echo "-ifn $(project).prj" >> $@
	echo "-ofn $(project).ngc" >> $@
	echo "-vlgincdir {$(vlgincdir)}" >> $@
	if [ "x$(xst_extra_opts)" != "x" ]; then \
		echo "$(xst_extra_opts)" >> $@; \
	fi
	echo "$(xst_extra_opts)" | md5sum > .xst_extra_opts;
	cat ./xilinx.opt $(optfile) >> $@
junk += $(project).scr

$(project).post_map.twr: $(project).ncd
	$(xil_env); trce -e 10 $< $(project).pcf -o $@
junk += $(project).post_map.twr $(project).post_map.twx smartpreview.twr

$(project).twr: $(project)_par.ncd
	$(xil_env); trce $< $(project).pcf -o $(project).twr
junk += $(project).twr $(project).twx smartpreview.twr

$(project)_err.twr: $(project)_par.ncd
	$(xil_env); trce -e 10 $< $(project).pcf -o $(project)_err.twr
junk += $(project)_err.twr $(project)_err.twx

.gitignore: $(mkfiles)
	echo programming_files $(junk) | sed 's, ,\n,g' > .gitignore

clean:
	rm -rf $(junk) par_usage_statistics.html webtalk.log ./_xmsgs

distclean: clean
	rm -f .xst_extra_opts .curr_part
	rm -rf $(junk_cache)
	rm -f $(products)

old_part := $(shell cat .curr_part 2>/dev/null)
ifneq ($(part),$(old_part))
 ifeq ($(old_part),)
  old_part := none
 endif
 $(info Old part is $(old_part), target is $(part), we need to clean first!)
 $(shell rm -rf $(junk) *.ngc)
endif

# vim: set ts=8 sw=8 fdm=marker : 

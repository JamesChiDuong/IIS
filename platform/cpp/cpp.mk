CXX	:= g++
FLAGS	:= -std=c++11 -Wall -Og -g
OBJDIR  := obj-pc
RTLD	:= ../verilog
VERILATOR_ROOT ?= $(shell bash -c 'verilator -V|grep VERILATOR_ROOT | head -1 | sed -e " s/^.*=\s*//"')
VROOT   := $(VERILATOR_ROOT)
INCS	:= -I$(RTLD)/obj_dir/ -I$(VROOT)/include

VOBJDR	:= $(RTLD)/obj_dir
SYSVDR	:= $(VROOT)/include
#We need to change the name modules or delete the name. In the example, we have 2 moudles
MODULES := TranAndRecei
MODULES2:= Data_Receiver

SOURCES := $(MODULES2).cpp $(MODULES).cpp uartsim.cpp uartsim.h
## }}}
all:	$(OBJDIR)/ $(MODULES2) $(MODULES)


# Verilator's generated Makefile sets VM_*
-include $(VOBJDR)/V$(MODULES)_classes.mk
VSRC	:= $(addsuffix .cpp, $(VM_GLOBAL_FAST) $(VM_GLOBAL_SLOW))
VLIB	:= $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(VSRC)))

$(OBJDIR)/uartsim.o: uartsim.cpp uartsim.h

$(OBJDIR)/%.o: %.cpp
	$(mk-objdir)
	$(CXX) $(FLAGS) $(INCS) -c $< -o $@

$(OBJDIR)/%.o: $(SYSVDR)/%.cpp
	$(mk-objdir)
	$(CXX) $(FLAGS) $(INCS) -c $< -o $@


## }}}
##Data_Receiver
DATARESRCS := $(MODULES2).cpp uartsim.cpp
DATAREOBJ := $(subst .cpp,.o,$(DATARESRCS))
DATAREOBJS:= $(addprefix $(OBJDIR)/,$(DATAREOBJ)) $(VLIB)
$(MODULES2): $(DATAREOBJS) $(VOBJDR)/V$(MODULES2)__ALL.a
	$(CXX) $(FLAGS) $(INCS) $^ -lpthread -o $@

##Top
TOPSRCS := $(MODULES).cpp uartsim.cpp
TOPOBJ := $(subst .cpp,.o,$(TOPSRCS))
TOPOBJS:= $(addprefix $(OBJDIR)/,$(TOPOBJ)) $(VLIB)
$(MODULES): $(TOPOBJS) $(VOBJDR)/V$(MODULES)__ALL.a
	$(CXX) $(FLAGS) $(INCS) $^ -lpthread -o $@
## test
## {{{
#test:
#	./$(MODULES2)
#	./$(MODULES)
#
define	build-depends
	$(mk-objdir)
	@echo "Building dependency file"
	@$(CXX) $(FLAGS) $(INCS) -MM $(SOURCES) > $(OBJDIR)/xdepends.txt
	@sed -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdepends.txt > $(OBJDIR)/depends.txt
	@rm $(OBJDIR)/xdepends.txt
endef

.PHONY: depends
depends:
	$(build-depends)

$(OBJDIR)/depends.txt: depends

#
define	mk-objdir
	@bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi"
endef

#
# The "tags" target
#
tags:	$(SOURCES) $(HEADERS)
	@echo "Generating tags"
	@ctags $(SOURCES) $(HEADERS)

.PHONY: clean
clean:
	rm -f ./$(MODULES2)
	rm -f ./$(MODULES)
	rm -f $(MODULES2).vcd $(MODULES).vcd
	rm -rf $(OBJDIR)/

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJDIR)/depends.txt
endif

#+---------------------------------------------------------------------------
#
#  Copyright (c) 2010 Anton Gusev aka AHTOXA (HTTP://AHTOXA.NET)
#
#  File:       makefile
#
#  Contents:   makefile to build lpc2148 ARM software with gcc
#
#----------------------------------------------------------------------------

#############  program name
	TARGET	= shcng

	OPTIMIZE	= -Os

#	x86 tools
	BASE		= .
	CC			= gcc
	CXX			= g++
	LD			= g++
	AS			= $(CC) -x assembler-with-cpp
	OBJCOPY		= objcopy
	OBJDUMP		= objdump
	SIZE		= size -d
	FLASHER		= openocd
	RM			= rm
	CP			= cp
	MD			= mkdir
	
#	x86 compiler options
	CXXFLAGS 	= $(INCS) -I. -c -Wall -ffunction-sections -fdata-sections $( OPTIMIZE )
	LDFLAGS	= -Wl,--gc-sections
	


#	DIRS
	OBJDIR			= OBJS/

	COMMONDIR		= ../../COMMON/
	
	FWAKEDIR		= $(COMMONDIR)FWake/
	FWAKEBASEDIR	= $(FWAKEDIR)FWakeBase/
	FWAKEMASTERDIR	= $(FWAKEDIR)FWakeMaster/
	
	
	DIRS		= ./
	DIRS		+= $(COMMONDIR)
	DIRS		+= $(FWAKEBASEDIR)
	DIRS		+= $(FWAKEMASTERDIR)
	
	
	INCS	:= $(patsubst %, -I "%", $(DIRS)) 
	SRCS	:= $(wildcard $(addsuffix *.cpp, $(DIRS))) $(wildcard $(addsuffix *.c, $(DIRS))) $(wildcard $(addsuffix *.S, $(DIRS)))
	OBJS 	:= $(patsubst %.cpp, %.o, $(SRCS))
	
	
	
all: $(OBJS)
	@echo --linking under PC...
	$(CXX) $(LDFLAGS) $(OBJS) -o $(TARGET).exe
	size $(TARGET).exe
	

	
print:
	@echo $(OBJS)
	@echo $(SRCS)
	@echo $(INCS)
	
$(OBJDIR)%.o: %.cpp
	@echo --compiling under PC... $< 
	$(CXX) $(CXXFLAGS) $< -o $@
	
clean:
	rm -f $(OBJS) *.exe
	

# default settings

I_CC=icx
I_ICC=icx
I_ICPP=icpx
I_IFORT=ifx

# set in .bashrc to override
# 
#  INTEL_CC - windows
#  INTEL_ICC - Linux
#  INTEL_ICPP - Linux
#  INTEL_IFORT - Windows, Linux

ifdef INTEL_CC
  I_CC=$(INTEL_CC)
endif
ifdef INTEL_ICC
  I_ICC=$(INTEL_ICC)
endif
ifdef INTEL_ICPP
  I_CPP=$(INTEL_ICPP)
endif
ifdef INTEL_IFORT
  I_IFORT=$(INTEL_IFORT)
endif

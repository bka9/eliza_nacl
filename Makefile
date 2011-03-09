# Makefile for the eliza_nacl project.

.PHONY: all clean

# The files the project starts with.  Add your code files here!
CCFILES = eliza_nacl.cc

#Add your includes here.
INCLUDES =
# These libraries are necessary for PPAPI (Pepper 2) and NaCl.
LDFLAGS = -lppruntime \
          -lppapi_cpp \
          -lgoogle_nacl_platform \
          -lgio \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)

# Add your .nmf files for each nexe you build here.
all: check_variables eliza_nacl.nmf eliza_nacl_dbg.nmf

# common.mk has rules to build .o files from .cc files.
-include common.mk

# Targets to create the manifests.
eliza_nacl.nmf: eliza_nacl_x86_32.nexe eliza_nacl_x86_64.nexe
	@echo "Creating eliza_nacl.nmf..."
	$(PYTHON) generate_nmf.py --nmf $@ \
	 --x86-64 eliza_nacl_x86_64.nexe \
         --x86-32 eliza_nacl_x86_32.nexe

eliza_nacl_dbg.nmf: eliza_nacl_x86_32_dbg.nexe \
        eliza_nacl_x86_64_dbg.nexe
	@echo "Creating eliza_nacl_dbg.nmf..."
	$(PYTHON) generate_nmf.py --nmf $@ \
	--x86-64 eliza_nacl_x86_64_dbg.nexe \
        --x86-32 eliza_nacl_x86_32_dbg.nexe

# Targets to create the nexes.
eliza_nacl_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@
	$(NACL_STRIP) $@ -o $@

eliza_nacl_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@
	$(NACL_STRIP) $@ -o $@

eliza_nacl_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

eliza_nacl_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

# Target to clean up
clean:
	-$(RM) *.nmf *.o *.obj *.nexe


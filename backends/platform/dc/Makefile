# $URL$
# $Id$

ronindir = /usr/local/ronin

DYNAMIC_MODULES = 1

srcdir = ../../..
VPATH = $(srcdir)

CC      = sh-elf-gcc -ml -m4-single-only
CXX     = sh-elf-g++ -ml -m4-single-only
CXXFLAGS= -O3 -Wno-multichar -funroll-loops -fschedule-insns2 -fomit-frame-pointer -fdelete-null-pointer-checks -fno-exceptions
DEFINES = -D__DC__ -DNONSTANDARD_PORT -DUSE_MAD -DUSE_ZLIB -DDISABLE_DEFAULT_SAVEFILEMANAGER -DDISABLE_TEXT_CONSOLE -DDISABLE_COMMAND_LINE -DNOSERIAL
LDFLAGS = -Wl,-Ttext,0x8c010000 -nostartfiles $(ronindir)/lib/crt0.o
INCLUDES= -I./ -I$(srcdir) -I$(ronindir)/include/ -I$(srcdir)/engines
LIBS	= -L$(ronindir)/lib -lmad -lronin-noserial -lz -lm
EXECUTABLE = scummvm.elf
DEPDIR = .deps
PLUGIN_PREFIX =
PLUGIN_SUFFIX = .plg
PLUGIN_EXTRA_DEPS = plugin.x plugin.syms scummvm.elf
PLUGIN_LDFLAGS = -nostartfiles -Wl,-q,-Tplugin.x,--just-symbols,scummvm.elf,--retain-symbols-file,plugin.syms -L$(ronindir)/lib
MKDIR = mkdir -p
RM = rm -f
RM_REC = rm -rf
AR = sh-elf-ar cru
RANLIB = sh-elf-ranlib
HAVE_GCC3 = true
DISABLE_SCALERS = true

ifdef DYNAMIC_MODULES
DEFINES += -DDYNAMIC_MODULES
PRE_OBJS_FLAGS = -Wl,--whole-archive
POST_OBJS_FLAGS = -Wl,--no-whole-archive
ENABLED=DYNAMIC_PLUGIN
else
ENABLED=STATIC_PLUGIN
endif

ENABLE_SCUMM = $(ENABLED)
ENABLE_SCUMM_7_8 = $(ENABLED)
ENABLE_HE = $(ENABLED)
ENABLE_AGI = $(ENABLED)
ENABLE_AGOS = $(ENABLED)
ENABLE_CINE = $(ENABLED)
ENABLE_CRUISE = $(ENABLED)
ENABLE_DRASCULA = $(ENABLED)
ENABLE_GOB = $(ENABLED)
ENABLE_GROOVIE = $(ENABLED)
ENABLE_IGOR = $(ENABLED)
ENABLE_IHNM = $(ENABLED)
ENABLE_KYRA = $(ENABLED)
ENABLE_LURE = $(ENABLED)
ENABLE_M4 = $(ENABLED)
ENABLE_MADE = $(ENABLED)
ENABLE_PARALLACTION = $(ENABLED)
ENABLE_QUEEN = $(ENABLED)
ENABLE_SAGA = $(ENABLED)
ENABLE_SKY = $(ENABLED)
ENABLE_SWORD1 = $(ENABLED)
ENABLE_SWORD2 = $(ENABLED)
ENABLE_TOUCHE = $(ENABLED)
ENABLE_TUCKER = $(ENABLED)

OBJS :=	dcmain.o time.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o softkbd.o dcloader.o cache.o dc-fs.o

MODULE_DIRS += ./

include $(srcdir)/Makefile.common

scummvm.bin : scummvm.elf
	sh-elf-objcopy -S -R .stack -O binary $< $@

SCUMMVM.BIN : scummvm.bin
	scramble $< $@

plugin_dist : plugins
	for p in $(PLUGINS); do \
	  sh-elf-strip -g -o "`basename \"$$p\" | LC_CTYPE=C tr '[:lower:]' '[:upper:]'`" "$$p"; \
	done

dist : SCUMMVM.BIN plugin_dist

spotless : distclean
	$(RM) SCUMMVM.BIN scummvm.bin *.PLG

all: sudoku2
OUTFLAG= -o
# Flag: CC
#	Use this flag to define compiler to use
####CC?= cc
CC=riscv64-linux-gnu-gcc
# Flag: CFLAGS
#	Use this flag to define compiler options. Note, you can add compiler options from the command line using XCFLAGS="other flags"
PORT_CFLAGS = -O2
FLAGS_STR = "$(PORT_CFLAGS) $(XCFLAGS) $(XLFLAGS) $(LFLAGS_END)"
CFLAGS = $(PORT_CFLAGS) -I$(PORT_DIR) -dn -static -Iposix -I. -DFLAGS_STR=\"$(FLAGS_STR)\"
ifndef NO_LIBRT
#Flag: LFLAGS_END
#	Define any libraries needed for linking or other flags that should come at the end of the link line (e.g. linker scripts). 
#	Note: On certain platforms, the default clock_gettime implementation is supported but requires linking of librt.
LFLAGS_END += -lrt
endif
# Flag: PORT_SRCS
#	Port specific source files can be added here
PORT_SRCS = sudoku2.c
# Flag: LOAD
#	Define this flag if you need to load to a target, as in a cross compile environment.

# Flag: RUN
#	Define this flag if running does not consist of simple invocation of the binary.
#	In a cross compile environment, you need to define this.

OEXT = .o
EXE = .exe

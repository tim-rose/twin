#
# Makefile --Build rules for libtwin's demo code
#
language = c

C_MAIN_SRC = twin-root.c
C_SRC = twin-root.c
BUILD_PATH = ../libtwin

include makeshift.mk

$(C_MAIN): $(C_OBJ) -ltwin -lxtd

test:   test-local
test-local:; $(C_MAIN)

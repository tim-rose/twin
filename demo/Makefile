#
# Makefile --Build rules for libtwin's demo code
#
language = c

C_MAIN_SRC = twidget-demo.c twin-root.c
C_SRC = twidget-demo.c twin-root.c
BUILD_PATH = ../libtwin

include makeshift.mk

$(C_MAIN): -ltwin -lxtd

test:   test-local
test-local:; $(C_MAIN)

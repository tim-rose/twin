#
# Makefile --Build rules for libtwin's demo code.
#
# The demo code just runs through some screen theatrics.
#
language = c

C_MAIN_SRC = root-demo.c twidget-demo.c
C_SRC = root-demo.c twidget-demo.c
BUILD_PATH = ../libtwin

include makeshift.mk

$(C_MAIN): -ltwin -lxtd

test:   test-local
test-local:; $(C_MAIN)

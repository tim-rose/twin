#
# Makefile --Build rules for TextWindows
#
PACKAGE=twin

include makeshift.mk

build@demo: build@libtwin
build@test: build@libtwin

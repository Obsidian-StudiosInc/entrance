AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = -I m4

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
compile \
config.h.in \
config.h.in~ \
configure \
depcomp \
install-sh \
missing

EXTRA_DIST =
ENTRANCE_CLEANFILES =
internal_libdir = $(libdir)/entrance
internal_lib_PROGRAMS =

include src/bin/Makefile.mk
include src/daemon/Makefile.mk
include data/Makefile.mk
include data/themes/default/Makefile.mk

EXTRA_DIST += \
autogen.sh \
m4/ac_attribute.m4 \
m4/efl_compiler_flag.m4 \
utils/entrance_xephyr.sh

clean-local:
	rm -rf $(ENTRANCE_CLEANFILES)

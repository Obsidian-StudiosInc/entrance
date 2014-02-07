
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/event \
-I$(top_builddir)/src/event \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DSYSTEM_CONFIG_DIR=\"$(sysconfdir)\" \
-DPACKAGE_BIN_DIR=\"$(libdir)/$(PACKAGE)\" \
-DPACKAGE_SBIN_DIR=\"$(sbindir)/\" \
@ENTRANCE_CFLAGS@

noinst_LIBRARIES = libevent.a
libevent_a_SOURCES = entrance_event.c

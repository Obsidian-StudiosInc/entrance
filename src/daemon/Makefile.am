
MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = \
-I$(top_srcdir)/src/daemon \
-I$(top_builddir)/src/daemon \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DSYSTEM_CONFIG_DIR=\"$(sysconfdir)\" \
-DPACKAGE_BIN_DIR=\"$(libdir)/$(PACKAGE)\" \
-DPACKAGE_SBIN_DIR=\"$(sbindir)/\" \
@ENTRANCE_CFLAGS@

if HAVE_CONSOLEKIT
AM_CPPFLAGS += @ENTRANCE_CK_CFLAGS@
endif

sbin_PROGRAMS = entrance

internal_bindir=$(libdir)/entrance
internal_bin_PROGRAMS = entrance_wait

if HAVE_CONSOLEKIT
internal_bin_PROGRAMS += entrance_ck_launch
endif

entrance_SOURCES =     \
entrance_config.h      \
entrance_config.c      \
entrance_session.h     \
entrance_session.c     \
entrance_xserver.h     \
entrance_xserver.c     \
../event/entrance_event.c \
../event/entrance_event.h \
entrance_server.h     \
entrance_server.c     \
entrance_history.h    \
entrance_history.c    \
entrance_action.h     \
entrance_action.c     \
entrance_image.h      \
entrance_image.c      \
entrance.h            \
entrance.c


entrance_LDFLAGS =
entrance_LDADD = @ENTRANCE_LIBS@

entrance_wait_SOURCES = \
entrance_wait.c

entrance_wait_LDFLAGS =
entrance_wait_LDADD =

if HAVE_PAM
entrance_SOURCES += entrance_pam.h entrance_pam.c
entrance_LDADD += -lpam
else
entrance_LDADD += -lcrypt
endif

if HAVE_CONSOLEKIT
entrance_ck_launch_SOURCES = \
entrance_ck_launch.c

entrance_ck_launch_LDFLAGS =
entrance_ck_launch_LDADD = @ENTRANCE_CK_LIBS@
endif


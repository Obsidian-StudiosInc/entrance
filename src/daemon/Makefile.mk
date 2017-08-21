
# entrance

sbin_PROGRAMS = src/daemon/entrance

src_daemon_entrance_SOURCES =     \
src/event/entrance_event.c \
src/event/entrance_event.h \
src/daemon/entrance_config.h \
src/daemon/entrance_config.c \
src/daemon/entrance_session.h \
src/daemon/entrance_session.c \
src/daemon/entrance_xserver.h \
src/daemon/entrance_xserver.c \
src/daemon/entrance_server.h \
src/daemon/entrance_server.c \
src/daemon/entrance_history.h \
src/daemon/entrance_history.c \
src/daemon/entrance_action.h \
src/daemon/entrance_action.c \
src/daemon/entrance_image.h \
src/daemon/entrance_image.c \
src/daemon/entrance_theme.h \
src/daemon/entrance_theme.c \
src/daemon/entrance.h \
src/daemon/entrance.c

if HAVE_PAM
src_daemon_entrance_SOURCES += \
src/daemon/entrance_pam.h \
src/daemon/entrance_pam.c
endif

src_daemon_entrance_CPPFLAGS = \
-I$(top_srcdir)/src/daemon \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DSYSTEM_CONFIG_DIR=\"$(sysconfdir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_BIN_DIR=\"$(libdir)/$(PACKAGE)\" \
-DPACKAGE_SBIN_DIR=\"$(sbindir)/\" \
@ENTRANCE_CFLAGS@

src_daemon_entrance_LDADD = @ENTRANCE_LIBS@ -lrt

if HAVE_PAM
src_daemon_entrance_LDADD += -lpam
else
src_daemon_entrance_LDADD += -lcrypt
endif

# entrance_wait

internal_lib_PROGRAMS += src/daemon/entrance_wait

src_daemon_entrance_wait_SOURCES = \
src/daemon/entrance_wait.c

src_daemon_entrance_wait_CPPFLAGS = \
-I$(top_srcdir)/src/daemon \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DSYSTEM_CONFIG_DIR=\"$(sysconfdir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_BIN_DIR=\"$(libdir)/$(PACKAGE)\" \
-DPACKAGE_SBIN_DIR=\"$(sbindir)/\" \
@ENTRANCE_CFLAGS@

# entrance_ck_launch

if HAVE_CONSOLEKIT
internal_lib_PROGRAMS += src/daemon/entrance_ck_launch

src_daemon_entrance_ck_launch_SOURCES = \
src/daemon/entrance_ck_launch.c

src_daemon_entrance_ck_launch_CPPFLAGS = \
-I$(top_srcdir)/src/daemon \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DSYSTEM_CONFIG_DIR=\"$(sysconfdir)\" \
-DPACKAGE_LIB_DIR=\"$(libdir)\" \
-DPACKAGE_BIN_DIR=\"$(libdir)/$(PACKAGE)\" \
-DPACKAGE_SBIN_DIR=\"$(sbindir)/\" \
@ENTRANCE_CK_CFLAGS@

src_daemon_entrance_ck_launch_LDADD = @ENTRANCE_CK_LIBS@
endif

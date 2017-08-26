
internal_lib_PROGRAMS += src/bin/entrance_client

src_bin_entrance_client_SOURCES = \
src/event/entrance_event.h \
src/event/entrance_event.c \
src/bin/entrance_client.h \
src/bin/entrance_client.c \
src/bin/entrance_connect.h \
src/bin/entrance_connect.c \
src/bin/entrance_fill.h \
src/bin/entrance_fill.c \
src/bin/entrance_login.h \
src/bin/entrance_login.c \
src/bin/entrance_gui.h \
src/bin/entrance_gui.c

src_bin_entrance_client_CPPFLAGS = \
-I$(top_srcdir)/src/bin \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
-DSYSTEM_CONFIG_DIR=\"$(sysconfdir)\" \
@ENTRANCE_CLIENT_CFLAGS@

src_bin_entrance_client_LDADD = @ENTRANCE_CLIENT_LIBS@


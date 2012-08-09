AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v -id $(top_srcdir)/data/themes/old -fd $(top_srcdir)/data/themes/old

filesdir = $(datadir)/entrance/themes/old
files_DATA = old.edj

EXTRA_DIST = \
default.edc \
entrance_background.jpg \
entrance_foreground.png \
box_login.png \
box_login_shine.png \
check.png \
arrow_left.png \
logo_b.png \
logo_w.png \
star.png \
moon.png


old.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/themes/old/default.edc \
	$(top_builddir)/data/themes/old/old.edj

clean-local:
	rm -f *.edj


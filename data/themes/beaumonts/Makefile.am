AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v -id $(top_srcdir)/data/themes/beaumonts -fd $(top_srcdir)/data/themes/beaumonts

filesdir = $(datadir)/entrance/themes/
files_DATA = default.edj

EXTRA_DIST = \
beaumonts.edc \
entrance_background.jpg \
entrance_foreground.png \
box_login.png \
box_login_shine.png \
login_box.png \
check.png \
arrow_left.png \
logo_b.png \
logo_w.png \
star.png \
user.png \
password.png \
moon.png \
xsession.png \
silhouette.png


default.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/themes/beaumonts/beaumonts.edc \
	$(top_builddir)/data/themes/beaumonts/default.edj

clean-local:
	rm -f *.edj


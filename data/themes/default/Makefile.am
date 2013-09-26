AUTOMAKE_OPTIONS     = 1.4 foreign
MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v -id $(top_srcdir)/data/themes/default/images -fd $(top_srcdir)/data/themes/default/fonts

filesdir = $(datadir)/entrance/themes/
files_DATA = default.edj

EXTRA_DIST = \
 default.edc \
 images/adrielhernandez-cmyk-tux.png \
images/brunocb-tux-1-euro.png \
images/cisoun-tux-coup-de-soleil.png \
images/eliaden-tux-marin.png \
images/entrance_background.jpg \
images/fcys14-tux-breton.png \
images/fcys14-tux-croco.png \
images/fcys14-tux-pompier.png \
images/fcys14-yoshi-tux.png \
images/m4r10-tux-crema.png \
images/mybob-calimetux.png \
images/overlord59-astro-tux.png \
images/overlord59-dj-tux-mix-platine.png \
images/overlord59-magic-tux.png \
images/overlord59-tux-pianiste.png \
images/touko-tux-pirate.png


default.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/themes/default/default.edc \
	$(top_builddir)/data/themes/default/default.edj

clean-local:
	rm -f *.edj


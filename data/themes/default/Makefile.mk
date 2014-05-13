
edje_process = \
$(edje) \
-id $(top_srcdir)/data/themes/default/images \
-fd $(top_srcdir)/data/themes/default/fonts \
$< $@ || rm -f $@

edje = @edje_cc@
edje_verbose = $(edje_verbose_@AM_V@)
edje_verbose_ = $(edje_verbose_@AM_DEFAULT_V@)
edje_verbose_0 = @echo "  EDJ     " $@;

filesdir = $(datadir)/entrance/themes/
files_DATA = data/themes/default/default.edj

EXTRA_DIST += \
data/themes/default/default.edc \
data/themes/default/images/adrielhernandez-cmyk-tux.png \
data/themes/default/images/brunocb-tux-1-euro.png \
data/themes/default/images/cisoun-tux-coup-de-soleil.png \
data/themes/default/images/eliaden-tux-marin.png \
data/themes/default/images/entrance_background.jpg \
data/themes/default/images/fcys14-tux-breton.png \
data/themes/default/images/fcys14-tux-croco.png \
data/themes/default/images/fcys14-tux-pompier.png \
data/themes/default/images/fcys14-yoshi-tux.png \
data/themes/default/images/m4r10-tux-crema.png \
data/themes/default/images/mybob-calimetux.png \
data/themes/default/images/overlord59-astro-tux.png \
data/themes/default/images/overlord59-dj-tux-mix-platine.png \
data/themes/default/images/overlord59-magic-tux.png \
data/themes/default/images/overlord59-tux-pianiste.png \
data/themes/default/images/touko-tux-pirate.png


data/themes/default/default.edj: $(top_srcdir)/data/themes/default/default.edc Makefile $(EXTRA_DIST)
	$(AM_V_at)rm -f $@
	$(AM_V_at)$(MKDIR_P) data/themes/default/
	$(edje_verbose)$(edje_process)

ENTRANCE_CLEANFILES += $(top_builddir)/data/themes/default/*.edj

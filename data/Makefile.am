
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = entrance entrance.conf entrance.other entrance.service.in

SUBDIRS = themes

pamdir = $(sysconfdir)/pam.d/
pam_DATA = entrance

confdir = $(sysconfdir)
conf_DATA = entrance.conf

sudoersdir = $(sysconfdir)/sudoers.d
sudoers_DATA = sudoers/entrance

if HAVE_SYSTEMD
systemddir = /usr/lib/systemd/system/
systemd_DATA = entrance.service

sed_process = @SED@ \
-e 's,@VERSION\@,$(VERSION),g' \
-e 's,@VMAJ\@,$(VMAJ),g' \
-e 's,@prefix\@,$(prefix),g' \
-e 's,@exec_prefix\@,$(exec_prefix),g' \
-e 's,@libdir\@,$(libdir),g' \
-e 's,@includedir\@,$(includedir),g' \
-e 's,@pkgincludedir\@,$(pkgincludedir),g' \
-e 's,@SBINDIR\@,$(sbindir),g' \
< $< > $@ || rm $@

pc_verbose = $(pc_verbose_@AM_V@)
pc_verbose_ = $(pc_verbose_@AM_DEFAULT_V@)
pc_verbose_0 = @echo " SED " $@;

entrance.service: $(top_srcdir)/data/entrance.service.in Makefile
	$(AM_V_at)rm -f $@
	$(pc_verbose)$(sed_process)

endif

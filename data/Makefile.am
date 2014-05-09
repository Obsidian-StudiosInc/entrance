
MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = entrance entrance.conf entrance.other entrance.service.in sudoers/entrance.in Xsession

SUBDIRS = themes

pamdir = $(sysconfdir)/pam.d/
pam_DATA = entrance

confdir = $(sysconfdir)/entrance
conf_DATA = entrance.conf

internal_bindir=$(sysconfdir)/entrance
internal_bin_SCRIPTS = Xsession


sudoersdir = $(sysconfdir)/sudoers.d
sudoers_DATA = sudoers/entrance

sed_process = @SED@ \
-e 's,@VERSION\@,$(VERSION),g' \
-e 's,@VMAJ\@,$(VMAJ),g' \
-e 's,@prefix\@,$(prefix),g' \
-e 's,@exec_prefix\@,$(exec_prefix),g' \
-e 's,@libdir\@,$(libdir),g' \
-e 's,@includedir\@,$(includedir),g' \
-e 's,@pkgincludedir\@,$(pkgincludedir),g' \
-e 's,@SBINDIR\@,$(sbindir),g' \
-e 's,@SYSCONFDIR\@,$(sysconfdir),g' \
< $< > $@ || rm $@

pc_verbose = $(pc_verbose_@AM_V@)
pc_verbose_ = $(pc_verbose_@AM_DEFAULT_V@)
pc_verbose_0 = @echo " SED " $@;

sudoers/entrance: $(top_srcdir)/data/sudoers/entrance.in Makefile
	$(AM_V_at)$(RM) $@
	$(AM_V_at)$(MKDIR_P) sudoers/
	$(pc_verbose)$(sed_process)

entrance.conf: $(top_srcdir)/data/entrance.conf.in Makefile
	$(AM_V_at)$(RM) $@
	$(pc_verbose)$(sed_process)

if HAVE_SYSTEMD
systemddir = /usr/lib/systemd/system/
systemd_DATA = entrance.service

entrance.service: $(top_srcdir)/data/entrance.service.in Makefile
	$(AM_V_at)$(RM) $@
	$(pc_verbose)$(sed_process)

endif

clean-local:
	$(RM) entrance.conf entrance.service sudoers/entrance

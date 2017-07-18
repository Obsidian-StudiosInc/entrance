
pamdir = $(sysconfdir)/pam.d/
pam_DATA = data/entrance

confdir = $(sysconfdir)/entrance
conf_DATA = data/entrance.conf

internal_sysconfdir=$(sysconfdir)/entrance
internal_sysconf_SCRIPTS = data/Xsession

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
< $< > $@ || rm $(top_srcdir)/$@

pc_verbose = $(pc_verbose_@AM_V@)
pc_verbose_ = $(pc_verbose_@AM_DEFAULT_V@)
pc_verbose_0 = @echo "  SED     " $@;

data/entrance.conf: $(top_srcdir)/data/entrance.conf.in Makefile
	$(AM_V_at)$(RM) $@
	$(AM_V_at)$(MKDIR_P) data/
	$(pc_verbose)$(sed_process)

if HAVE_SYSTEMD
systemddir = /usr/lib/systemd/system/
systemd_DATA = data/entrance.service

data/entrance.service: $(top_srcdir)/data/entrance.service.in Makefile
	$(AM_V_at)$(RM) $@
	$(pc_verbose)$(sed_process)

endif

ENTRANCE_CLEANFILES += \
data/entrance.conf \
data/entrance.service

EXTRA_DIST += \
data/entrance \
data/entrance.arch \
data/entrance.conf.in \
data/entrance.other \
data/entrance.service.in \
data/Xsession

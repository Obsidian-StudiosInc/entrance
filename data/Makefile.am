
MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = themes

pamdir = $(sysconfdir)/pam.d/
pam_DATA = entrance

confdir = $(sysconfdir)
conf_DATA = entrance.conf

if WANT_SYSTEMD
systemddir = /usr/lib/systemd/system/
systemd_DATA = entrance.service
endif

EXTRA_DIST = entrance entrance.conf entrance.other

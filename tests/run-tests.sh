#!/bin/bash
# wrapper to run entrance with env vars



[[ ! -d /usr/share/xsessions ]] && mkdir -p /usr/share/xsessions

echo "[Desktop Entry]
Name=XSession
Comment=Xsession
Exec=/etc/entrance/Xsession
TryExec=/etc/entrance/Xsession
Icon=
Type=Application
" > /usr/share/xsessions/Xsession.desktop

#	"s/vt7/-noreset +extension GLX +extension RANDR +extension RENDER/" \
sed -i -e \
	"s|nobody|travis|"
	/etc/entrance/entrance.conf

/usr/sbin/entrance

kill -SIGUSR1 $(pgrep entrance)

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

sed -i -e "s/vt7//" /etc/entrance/entrance.conf

/usr/sbin/entrance

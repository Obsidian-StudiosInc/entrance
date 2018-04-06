#!/bin/bash
# wrapper to run entrance with env vars

export XDG_RUNTIME_DIR="/run/user/0/"

[[ ! -d /usr/share/xsessions ]] && mkdir -p /usr/share/xsessions

echo "[Desktop Entry]
Name=XSession
Comment=Xsession
Exec=/etc/entrance/Xsession
TryExec=/etc/entrance/Xsession
Icon=
Type=Application
" > /usr/share/xsessions/Xsession.desktop

/usr/sbin/entrance

EPID=$(pgrep entrance)

kill -SIGUSR1 ${EPID}

sleep 3

kill ${EPID}

#systemctl start entrance

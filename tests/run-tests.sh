#!/bin/bash
# wrapper to run entrance with env vars

find /run /var/run -type d .ecore -print

export XDG_RUNTIME_DIR="/tmp/ecore"

for d in "${XDG_RUNTIME_DIR}"{,/.ecore} /usr/share/xsessions; do
	[[ ! -d "${d}" ]] && mkdir -p "${d}"
done

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

echo "EPID=${EPID}"

kill -SIGUSR1 ${EPID}

sleep 10

killall X

ps x o pid,user,group,command

#systemctl start entrance

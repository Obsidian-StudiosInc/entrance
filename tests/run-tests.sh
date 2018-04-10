#!/bin/bash
# wrapper to run entrance with env vars

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

sed -i -e "s|nobody|travis|" /etc/entrance/entrance.conf

/usr/sbin/entrance

EPID="$(pgrep entrance)"

kill -SIGUSR1 ${EPID}

echo "Going to sleep"
sleep 120
echo ""

ps xa o pid,user,group,command
echo ""

kill ${EPID}

EPID="$(pgrep X)"

[[ ${EPID} ]] && kill -9 ${EPID}

ps xa o pid,user,group,command

#systemctl start entrance

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

SLEEP=180

echo "${0} Going to sleep for ${SLEEP}"
sleep ${SLEEP}
echo "${0} Waking up"

killall -v entrance_client

sleep 5

killall -v entrance

EPID="$(pgrep entrance)"

kill_counter=0
while sleep 1
	"${kill_counter}" -lt 20
	kill -0 "${EPID}" >/dev/null 2>&1
do
	kill_counter=$((kill_counter+1))
	[[ "${kill_counter}" -eq 10 ]] && killall -v entrance
done

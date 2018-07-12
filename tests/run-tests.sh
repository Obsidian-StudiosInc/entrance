#!/bin/bash
# wrapper to run entrance with env vars

if [[ ! -d /home/travis ]]; then
	echo "Only meant to be run by Travis-CI"
	exit 1
fi

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

sleep 5

echo "${0} Additional client tests"
export HOME=/tmp
/usr/lib/x86_64-linux-gnu/entrance/entrance_client
/usr/lib/x86_64-linux-gnu/entrance/entrance_client --help
export HOME=/home/travis

echo "${0} Test autologin"
useradd -g users -m -p 1234 -s /bin/bash myusername
sed -i -e "s|autologin\" uchar: 0|autologin\" uchar: 1|" \
	/etc/entrance/entrance.conf

/usr/sbin/entrance

SLEEP=60

echo "${0} Going to sleep for ${SLEEP}"
sleep ${SLEEP}
echo "${0} Waking up"

killall -v entrance

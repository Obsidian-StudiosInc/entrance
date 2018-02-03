#!/bin/bash

MY_PWD="$(pwd)"

[[ -d build ]] && rm -r build

CFLAGS=-g
MY_PWD+="/build"
meson \
	--prefix "${MY_PWD}" \
	--libdir "${MY_PWD}/test" \
	--sbindir "${MY_PWD}/test" \
	--datadir "${MY_PWD}/test" \
	--sysconfdir "${MY_PWD}/test" \
	-Ddebug=true \
	$@ . build
ninja -C build
mv -v build/entrance.conf build/data
cd build

DIRS=( test/entrance/themes )

for d in ${DIRS[@]}; do
	[[ ! -d "${d}" ]] && mkdir -p "${d}"
done

[[ ! -L  test/elementary ]] && ln -s /usr/share/elementary test/elementary

cp data/entrance.conf \
	src/bin/entrance_client \
	src/daemon/entrance_wait \
	test/entrance

cp data/themes/default/default.edj test/entrance/themes
chmod 664 test/entrance/themes/default.edj

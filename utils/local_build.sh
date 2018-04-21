#!/bin/bash

MY_PWD="$(pwd)"

[[ -d build ]] && rm -fr build

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
scan-build ninja -C build
mv -v build/entrance.conf build/data
cd build

DIRS=( test/entrance/themes )

for d in ${DIRS[@]}; do
	[[ ! -d "${d}" ]] && mkdir -p "${d}"
done

[[ ! -L  test/elementary ]] && ln -s /usr/share/elementary test/elementary

cp data/entrance.conf \
	../data/Xsession \
	src/bin/entrance_client \
	test/entrance

cp data/themes/default/default.edj test/entrance/themes
chmod 664 test/entrance/themes/default.edj

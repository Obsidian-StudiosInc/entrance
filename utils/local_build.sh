#!/bin/bash

MY_PWD="$(pwd)"

[[ -f Makefile ]] && ./utils/clean.sh
[[ -d build ]] && rm -r build

if [[ -n ${1} ]]; then
	./autogen.sh \
		--prefix "${MY_PWD}" \
		--libdir "${MY_PWD}/test" \
		--datarootdir "${MY_PWD}/test" \
		--sysconfdir "${MY_PWD}/test"
	make
else
	CFLAGS=-g
	MY_PWD+="/build"
	meson \
		--prefix "${MY_PWD}" \
		--libdir "${MY_PWD}/test" \
		--sbindir "${MY_PWD}/test" \
		--datadir "${MY_PWD}/test" \
		--sysconfdir "${MY_PWD}/test" \
		. build
	ninja -C build
	mv -v build/entrance.conf build/data
	cd build
fi

DIRS=(
	test/entrance/themes
)

for d in ${DIRS[@]}; do
	[[ ! -d "${d}" ]] && mkdir -p "${d}"
done

[[ ! -L  test/elementary ]] && ln -s /usr/share/elementary test/elementary

cp data/entrance.conf \
	src/bin/entrance_client \
	src/daemon/entrance_ck_launch \
	src/daemon/entrance_wait \
	test/entrance

cp data/themes/default/default.edj test/entrance/themes
chmod 664 test/entrance/themes/default.edj

#!/bin/bash

make clean

DIRS=(
	test/entrance/themes
)

for d in ${DIRS[@]}; do
	[[ ! -d ${d} ]] && mkdir -p ${d}
done

./autogen.sh \
	--disable-grub2 \
	--prefix $(pwd) \
	--libdir $(pwd)/test \
	--datarootdir $(pwd)/test

make

cp src/bin/entrance_client \
	src/daemon/entrance_ck_launch \
	src/daemon/entrance_wait \
	test/entrance

cp data/themes/default/default.edj test/entrance/themes
chmod 664 test/entrance/themes/default.edj

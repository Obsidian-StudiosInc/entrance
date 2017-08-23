#!/bin/bash

DPI=${DPI:-142}
SCREEN=${SCREEN:-1024x768}
ENTRANCE=${ENTRANCE:-src/daemon/entrance}

[[ -d build ]] && cd build

if [[ ! -f src/daemon/entrance ]]; then
	echo "src/daemon/entrance does not exist, run ./utils/local_build.sh"
	exit 1
fi

while :
do
	case "${1}" in
		-d | --dpi)
			DPI="${1}"
			shift
			;;
		-e | --entrance)
			ENTRANCE="${1}"
			shift
			;;
		-g | --gdb)
			GDB=1
			;;
		-s | --screen)
			SCREEN="${1}"
			shift
			;;
		-v | --valgrind)
			VALGRIND=1
			;;
		*)
			break
			;;

    	esac
done

#rm -f ~/.Xauthority
Xephyr :1 -nolisten tcp -noreset -ac -br -dpi "${DPI}" -screen "${SCREEN}" &
sleep 1

if [[ ${GDB} ]]; then
	gdb --args "${ENTRANCE}" -x
elif [[ ${VALGRIND} ]]; then
	valgrind \
		--leak-check=full \
		--read-var-info=yes \
		--show-reachable=yes \
		--track-origins=yes \
		"${ENTRANCE}" -x
else
	"${ENTRANCE}" -x
fi

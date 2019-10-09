#!/bin/sh

cmd="$1"

if [ -z "$JOBS" ]; then
	JOBS=1
fi

if test "$cmd" = "configure"; then
	cd apps/
	rm -f external
	ln -s ../rffe-app external
	cd ..
	cd nuttx/
	./tools/configure.sh ../rffe-board/rffe
	cd ..
elif test "$cmd" = "build"; then
	cd nuttx/
	if [ ! -e .config ]; then
		echo "Error: Build not configured yet."
		echo "Please run '$0 configure' before building."
		exit 1
	fi
    make -j ${JOBS}
	cd ..
elif test "$cmd" = "clean"; then
	cd nuttx/
	make distclean
	cd ..
	rm apps/external
elif test "$cmd" = "flash"; then
	openocd -f scripts/openocd/lpc17-cmsis.cfg -c "program nuttx/nuttx.bin 0x10000; reset; shutdown"
fi

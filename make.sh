#!/bin/sh

cmd="$1"

git_hash_tag() {
	cd apps/
	APPS_GIT_HASH=$(git describe --no-match --always --dirty --abbrev=40)
	cd ../nuttx
	NUTTX_GIT_HASH=$(git describe --no-match --always --dirty --abbrev=40)
	cd ..
	RFFE_GIT_HASH=$(git describe --no-match --always --dirty --abbrev=40)
	RFFE_GIT_TAG=$(git describe --exact-match --tags)
	echo -e "/* Auto-generated file */\n\n#define APPS_GIT_HASH  \"${APPS_GIT_HASH}\"\n#define NUTTX_GIT_HASH \"${NUTTX_GIT_HASH}\"\n#define RFFE_GIT_HASH  \"${RFFE_GIT_HASH}\"\n#define RFFE_GIT_TAG   \"${RFFE_GIT_TAG}\"\n" > rffe-app/git_version.h
}

if [ -z "$JOBS" ]; then
	JOBS=1
fi

if test "$cmd" = "configure"; then
	git_hash_tag
	cd apps/
	rm -f external
	ln -s ../rffe-app external
	cd ..
	cd nuttx/
	./tools/configure.sh ../rffe-board/rffe
	cd ..
elif test "$cmd" = "build"; then
	git_hash_tag
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
	rm -f apps/external rffe-app/git_version.h
elif test "$cmd" = "flash"; then
	openocd -f scripts/openocd/lpc17-cmsis.cfg -c "program nuttx/nuttx.bin 0x10000; reset; shutdown"
fi

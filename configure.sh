#!/usr/bin/env bash

VERSION_MAJOR=0
VERSION_MINOR=1

DIALOG="$(which dialog 2>/dev/null)"

if [ -z "$DIALOG" ]; then
	DIALOG="$(which whiptail 2>/dev/null)"
fi

if [ -z "$DIALOG" ]; then
	echo "Neither dialog or whiptail found."
	exit -1
fi

function dlg
{
	$DIALOG "$@" 3>&1 1>&2 2>&3
}

function inputSize
{
	size=""

	while [[ $size = "" ]]; do
		read size < <(dlg --inputbox $1 0 40 $2)
	done

	echo $size;
}

BLOCKSIZE=$(inputSize "Blocksize" 4096)

dlg --yesno "Do you want to see advanded options?" 0 40
ADVANCED=$?

VERSION="$VERSION_MAJOR.$VERSION_MINOR"

if [ -e "/usr/bin/git" ]; then
	VERSION="$VERSION-$(git log -1 --abbrev-commit | head -n 1 | cut -d " " -f 2)git"
fi

if [ $ADVANCED -eq 0 ]; then
	VERSION=$(dlg --inputbox "Version-string" 0 40 "$VERSION")
fi

configContent="#ifndef _CONFIG_H_INCLUDED
#define _CONFIG_H_INCLUDED

#define VERSION		\"$VERSION\"
#define VERSION_MAJOR	0
#define VERSION_MINOR	1

#define BLOCK_SIZE	$BLOCKSIZE

#endif"

echo "$configContent" > config.h

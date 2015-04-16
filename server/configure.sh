#!/usr/bin/env bash

VERSION_MAJOR=0
VERSION_MINOR=1

CXX="/usr/bin/g++"
CFLAGS="-Wall -g"
LDFLAGS="-lssl -lcrypto"
OBJ="secnet.o protocol.o blockstorage.o shuffle.o main.o"

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

dlg --yesno "Do you want to see advanded options?" 8 40
ADVANCED=$?

VERSION="$VERSION_MAJOR.$VERSION_MINOR"

if [ $(which git 2>/dev/null) != "" ]; then
	VERSION="$VERSION-$(git log -1 --abbrev-commit | head -n 1 | cut -d " " -f 2)git"
fi

if [ $ADVANCED -eq 0 ]; then
	VERSION=$(dlg --inputbox "Version-string" 0 40 "$VERSION")
fi

os=$(uname)

if [ "$os" != "Linux" ]; then
	CFLAGS="$CFLAGS -DO_LARGEFILE=\"0\" -Dlseek64=\"lseek\""

	if [ "$os" == "Darwin" ]; then
		CFLAGS="-I/usr/local/opt/openssl/include $CFLAGS -D__pid_t=\"pid_t\""
		LDFLAGS="-L/usr/local/opt/openssl/lib $LDFLAGS -lresolv"
	fi

	if [[ "$os" == *BSD ]]; then
		CXX="clang++"
	fi
else
	LDFLAGS="$LDFLAGS -lresolv"
fi

CFLAGS="$CFLAGS -DVERSION=\\\"$VERSION\\\" -DVERSION_MAJOR=$VERSION_MAJOR -DVERSION_MINOR=$VERSION_MINOR -DBLOCK_SIZE=$BLOCKSIZE"

makefileContent="CXX = $CXX
CFLAGS  = $CFLAGS
LDFLAGS = $LDFLAGS
OBJ = $OBJ

shepoo: \$(OBJ)
	\$(CXX) \$(CFLAGS) -o \$@ \$(OBJ) \$(LDFLAGS)

%.o: src/%.cpp
	\$(CXX) \$(CFLAGS) -c $<

.PHONY: clean

clean:
	rm \$(OBJ) shepoo > /dev/null 2>&1 || true"

echo "$makefileContent" > Makefile

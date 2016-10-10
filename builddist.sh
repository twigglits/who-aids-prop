#!/bin/bash

if [ "$#" != 1 ] ; then
	echo "You need to specify a destination directory!"
	exit -1
fi

VERSION=`cat CMakeLists.txt|grep VERSION | grep set | head -n 1 |cut -f 2 -d " "|cut -f 1 -d ")"`
LIBNAME=simpact-cyan

CURDIR=`pwd`
TMPDIR="$1"
if ! [ -d $TMPDIR ] ; then
	if ! mkdir $TMPDIR ; then
		echo "Couldn't create destination directory"
		exit -1
	fi
fi

cd $TMPDIR
TMPDIR=`pwd` # Get the full path
cd $CURDIR

if ! git archive --format tar --prefix=${LIBNAME}-${VERSION}/ HEAD | (cd $TMPDIR && tar xf -) ; then
	echo "Couldn't archive repository"
	exit -1
fi

cd $TMPDIR/${LIBNAME}-${VERSION}

rm -f `find . -name ".git*"`
rm builddist.sh
rm -rf sphinxdoc
rm -rf buildscripts

S=${TMPDIR}/${LIBNAME}-${VERSION}

cd $TMPDIR


cd $TMPDIR	
if ! tar cfz ${LIBNAME}-${VERSION}.tar.gz ${LIBNAME}-${VERSION}/ ; then
	echo "Couldn't create archive"
	exit -1
fi


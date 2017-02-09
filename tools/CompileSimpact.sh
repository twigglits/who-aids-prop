#!/bin/bash -e

if [ "$#" != "1" ] ; then
	echo "Please specify a version number for Simpact Cyan"
	echo
	echo "You can set GSL_OPTS for extra 'configure' options for GSL, and"
	echo "TIFF_OPTS for extra 'configure' options for the TIFF library"
	exit -1
fi

if ! cmake --version >/dev/null 2>&1 ; then
	echo "Executable 'cmake' is not known - please load necessary module or"
	echo "install necessary package. Note that this is only needed to compile"
	echo "everything, not for executing the resulting programs."
	exit -1
fi

SIMPACTVERSION="$1"
SIMPACTTGZ="simpact-cyan-${SIMPACTVERSION}.tar.gz"
SIMPACTURL="http://research.edm.uhasselt.be/jori/simpact/programs/${SIMPACTVERSION}/"

ROOT="$HOME/SimpactCyanBuild"

if [ -e "$ROOT" ] && ! [ -d "$ROOT" ] ; then
	echo "$ROOT exists but is not a directory"
	exit -1
fi

if ! [ -e "$ROOT" ] ; then
	mkdir "$ROOT"
fi

B="$ROOT/build"
if ! [ -e "$B" ] ; then
	mkdir "$B"
fi

EXTRACXXFLAGS=""
T="$ROOT/tmp"
if ! [ -e "$T" ] ; then
	mkdir "$T"
fi

cat << EOF > "$T/overridetest.cpp"
struct A
{
    virtual void f() { x = 10; }
    int x;
};

struct B : public A
{
    void f() override { x = 11; }
};

int main(void)
{
    B b;
    return 0;
}
EOF

if ! gcc -std=c++0x -c -o "$T/overridetest.o" "$T/overridetest.cpp" >/dev/null 2>&1 ; then
	EXTRACXXFLAGS="$EXTRACXXFLAGS -Doverride="
fi

cat << EOF > "$T/steadyclocktest.cpp"
#include <chrono>

int main(void)
{
    auto t0 = std::chrono::steady_clock::now();
    return 0;
}
EOF

if ! gcc -std=c++0x -c -o "$T/steadyclocktest.o" "$T/steadyclocktest.cpp" >/dev/null 2>&1 ; then
	EXTRACXXFLAGS="$EXTRACXXFLAGS -Dsteady_clock=system_clock"
fi

echo "EXTRACXXFLAGS=$EXTRACXXFLAGS"

P="$ROOT/simpactpack"
rm -rf "$P"

if ! [ -e "$P/bin" ] ; then
	mkdir -p "$P/bin"
fi

ACTIVATESCRIPT="$P/bin/activatesimpact.sh"
if ! [ -e "$ACTIVATESCRIPT" ] ; then
	cat << EOF > "$ACTIVATESCRIPT"
export PATH="$P/bin:\$PATH"
export SIMPACT_DATA_DIR="$P/share/simpact-cyan/"

if [ -z "\$PYTHONPATH" ] ; then
	export PYTHONPATH="$P/share/simpact-cyan/python"
else
	export PYTHONPATH="$P/share/simpact-cyan/python:\$PYTHONPATH"
fi
EOF
fi

source "$ACTIVATESCRIPT"

for l in \
gsl-1.16.tar.gz,http://gnu.xl-mirror.nl/gsl/ \
tiff-4.0.6.tar.gz,http://download.osgeo.org/libtiff/ \
"${SIMPACTTGZ},${SIMPACTURL}" \
	; do

	echo "$l"
        cd "$B"

        N=`echo "$l"|cut -f 1 -d ,|cut -f 1 -d ":"`
        U=`echo "$l"|cut -f 2 -d ,`
        if ! [ -e "$N" ] ; then
		echo "Downloading ${U}${N}"
                curl -o "$N" "${U}${N}"
        fi

        X=`echo "$l"|cut -f 1 -d ,|grep ":"|cat`
        if [ -z "$X" ] ; then
                D="${N:0:${#N}-7}"
        else
                D=`echo $l|cut -f 1 -d ,| cut -f 2 -d ":"`
        fi
        if ! [ -e "$D" ] ; then
                tar xfz "$N"
        fi

        if [ -e "$D/configure" ] ; then
                cd "$D"
		OPTS=""
		if [ ${N:0:4} = "tiff" ] ; then
			echo "TIFF"
			OPTS="$TIFF_OPTS"
		elif [ ${N:0:3} = "gsl" ] ; then
			echo "GSL"
			OPTS="$GSL_OPTS"
		fi
                if ! [ -e Makefile ] ; then
			echo CFLAGS=-O2 ./configure --prefix="$P" $OPTS
			sleep 1
                        CFLAGS=-O2 ./configure --prefix="$P" $OPTS
                fi
                make
                make install
        else # this should be the simpact package
                if ! [ -e "$D/build" ] ; then
                        mkdir "$D/build"
                fi

		# Let's find out some dependencies of libtiff
		EXTRA=""
		for i in `ldd $P/lib/libtiff.so | cut -f 1 -d . ` ; do 
			if [ "${i::3}" = "lib" ] ; then
				LIBNAME="${i:3}"
				if [ "$LIBNAME" != "c" ] && [ "$LIBNAME" != "pthread" ] ; then
					EXTRA="$EXTRA;$LIBNAME"
				fi
			fi
		done

                cd "$D/build"
                CC=gcc CXX=g++ cmake -DCMAKE_INSTALL_PREFIX="$P" -DADDITIONAL_LIBRARIES="$EXTRA;$P/lib/libgsl.a;$P/lib/libgslcblas.a" -DGSL_MANUAL_SETTINGS:BOOL=ON -DADDITIONAL_INCLUDE_DIRS="$P/include" -DTIFF_INCLUDE_DIR="$P/include/" -DTIFF_LIBRARY="$P/lib/libtiff.a" -DCMAKE_CXX_FLAGS="$EXTRACXXFLAGS" ..

                make -j 4
                make install
        fi
done

cat << EOF





Compilation successful, run

    source $ACTIVATESCRIPT

to be able to use Simpact Cyan

EOF


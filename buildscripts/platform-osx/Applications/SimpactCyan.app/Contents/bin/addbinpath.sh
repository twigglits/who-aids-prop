#!/bin/bash

INST=/Applications/SimpactCyan.app

add=yes
created=no
profilefile="$HOME/.bash_profile"
if [ -e "$profilefile" ] ; then
	X=`grep "LINE ADDED FOR PATH BY SIMPACT INSTALLER" "$profilefile"`
	if ! [ -z "$X" ] ; then
		add=no # Seems we already added this
		echo "Environment variable PATH already seems to be modified in $profilefile"
	fi
else
	created=yes
fi

if [ "$add" = "yes" ] ; then

	cat << EOF >> $profilefile
export PATH="\$PATH:$INST/Contents/bin" # LINE ADDED FOR PATH BY SIMPACT INSTALLER
EOF
	echo "Added PATH line to $profilefile"
	if [ "$created" = "yes" ] ; then
		# In case we're running as root, we need to change ownership
		if [ `whoami` = "root" ] ; then
			echo "$profilefile was created as root, changing ownership to ${USER}:staff"
			chown ${USER}:staff "$profilefile"
		fi
	fi
fi

exit 0


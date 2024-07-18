#!/bin/bash

INST=/Applications/SimpactCyan.app

add=yes
created=no
profilefile="$HOME/.bash_profile"
if [ -e "$profilefile" ] ; then
	X=`grep "LINE ADDED BY SIMPACT INSTALLER" "$profilefile"`
	if ! [ -z "$X" ] ; then
		add=no # Seems we already added this
		echo "Environment variable PYTHONPATH already seems to be modified in $profilefile"
	fi
else
	created=yes
fi

if [ "$add" = "yes" ] ; then

	cat << EOF >> $profilefile
export PYTHONPATH=\$(python -c "v='$INST/Contents/python/';import os;L=lambda x:[] if x is None else x.split(':');p=L(os.environ.get('PYTHONPATH'));p=p if v in p else p+[v];p2=[];exec('for i in p:\n if not i in p2:\n  p2.append(i)');print ':'.join(p2)") # LINE ADDED BY SIMPACT INSTALLER (adds path to PYTHONPATH and removes duplicates)
EOF
	echo "Added PYTHONPATH line to $profilefile"
	if [ "$created" = "yes" ] ; then
		# In case we're running as root, we need to change ownership
		if [ `whoami` = "root" ] ; then
			echo "$profilefile was created as root, changing ownership to ${USER}:staff"
			chown ${USER}:staff "$profilefile"
		fi
	fi
fi

exit 0


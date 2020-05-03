#!/bin/sh

if [ "$#" != "2" ]; then
	echo "Syntax: $0 'file extension' 'path to template.h'"
	exit 1
fi

if [ "$2" = "" ]; then
	$2=.
fi

if [ ! -r $2/template.h ]; then
	echo "Cannot find template.h which is required."
	exit 1
fi

for filex in *$1
do
	mv $filex $filex.bak
	cat $2/template.h > $filex
	cat $filex.bak >> $filex
	rm -f $filex.bak
	echo "Added header to $filex..."
done

exit 0
#!/bin/sh

arglist=$1 $2 $3 $4 $5 $6 $7 $8 $9

fail()
{
	echo ""
	echo "ERROR: $@";
	# Use a makefile with no autoconf in it.
	mv -f Makefile.in Makefile.bak
	mv -f Makefile.fail Makefile.in
	conf
	exit 1;
}

error()
{
	echo ""
	echo "ERROR: $@"
	exit 1;
}

conf()
{
	./configure $1 || error "./configure script failed!!"

	echo ""
	echo "Setup complete."
	echo ""
}

autoheader || fail "autoheader failed.  You need to upgrade autoconf."
autoconf || fail "autoconf failed.  You need to upgrade autoconf."

conf $arglist

exit


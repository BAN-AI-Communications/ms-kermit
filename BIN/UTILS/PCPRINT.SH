#!/bin/sh
#
# pcprint
#
# Prints named files on locally attached PC or terminal printer, using ANSI 
# transparent print commands.  If no files named on command line, prints from
# standard input.
#
# Can be used with MS-DOS Kermit 2.30 or later on an IBM PC, PS/2, or
# compatible, or with a VT102, VT2xx, or VT3xx terminal, that has a locally
# attached printer.  Or on any PC or terminal with locally attached printer
# that supports the ANSI printer control escape sequences.
#
# Works by sending the escape sequence ESC [ 5 i, which activates the
# terminal's transparent print mechanism (meaning that the following
# characters are sent to the local printer rather than the screen, and then
# sends the escape sequence ESC [ 4 i, which turns off transparent printing
# and puts the terminal back to normal.
#
# Note: On certain System-V based UNIX implementations, you might have to
# change the hard escape in the 'echo' commands below into the sequence
# caret (^) left bracket ([).
#
# Usage: pcprint file
#    or: pcprint file file file ...
#    or: pcprint < file
#    or: command | pcprint
#
# Author: C. Gianone, Columbia University, 1988
#
echo -n '[5i'
if [ $# -eq 0 ]; then
  cat
else
  cat $*
fi
echo -n '[4i'
# (end)

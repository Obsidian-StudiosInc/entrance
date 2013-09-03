#!/usr/bin/env bash
DPI=${DPI:-142}
SCREEN=${SCREEN:-640x480}
VALGRIND=${VALGRIND:-0}
GDB=${GDB:-0}

while [ $# -gt 0 ]; do
   case $1 in
      -g|--gdb)
         GDB=1
         ;;
      -v|--valgrind)
         VALGRIND=1
         ;;
    esac
    shift
done


#rm -f ~/.Xauthority
Xephyr :1 -nolisten tcp -noreset -ac -br -dpi $DPI -screen $SCREEN &
sleep 1
if [ $GDB -eq 1 ]
then
   gdb --args entrance -x
elif [ $VALGRIND -eq 1 ]
then
   valgrind --leak-check=full entrance -x
else
   entrance -x
fi


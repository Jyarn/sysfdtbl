#!/bin/bash
echo "ascii"
time ./sysfdtbl.out --output_TXT > /dev/null

echo "binary"
time ./sysfdtbl.out --output_binary > /dev/null
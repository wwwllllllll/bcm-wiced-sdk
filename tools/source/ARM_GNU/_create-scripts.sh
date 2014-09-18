#!/bin/sh
DATESTRING=$(date +%Y)$(date +%m)$(date +%d)
tar --exclude=*.bz2 --exclude=*.gz --exclude=install\* -cvjf build-scripts-$DATESTRING.tar.bz2 *



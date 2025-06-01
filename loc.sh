#!/usr/bin/env sh

######################################################################
# @author      : kistenklaus (karlsasssie@gmail.com)
# @file        : loc
# @created     : Sonntag Jun 01, 2025 19:49:47 CEST
#
# @description : 
######################################################################

cloc . \
  --exclude-dir=cmake,build,cmake-build-debug,deprecated,logs,.idea,.git\
  --exclude-list-file=loc.sh





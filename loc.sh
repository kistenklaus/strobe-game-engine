#!/usr/bin/env sh

######################################################################
# @author      : kistenklaus (karlsasssie@gmail.com)
# @file        : loc
# @created     : Sonntag Jun 01, 2025 19:49:47 CEST
#
# @description : 
######################################################################

cloc . \
  --exclude-dir=cmake,build,cmake-build-debug,cmake-build-release,deprecated,logs,.idea,.git,.github,.cache,third_party\
  --exclude-list-file=loc.sh





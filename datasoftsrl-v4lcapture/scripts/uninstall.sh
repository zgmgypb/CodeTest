#!/usr/bin/env bash

# This is part of v4lcapture library.
#
# Copyright (C) 2015 DataSoft Srl
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

# Error codes
#   - 255: generic error
#   - 254: incorrect parameters
#   - 253: no logfile

function error {
  >&2 echo "$(basename $0): $1"
  exit $2
}

# script must have two command line arguments
if [[ $# -lt 2 ]]; then
  >&2 echo "usage: $(basename $0) logfile prefix"
  exit 254
fi

LOGFILE=$1
PREFIX=$2

if [ -r $LOGFILE ]; then
  # if egg folder is remove
  rm_egg=true
  while read line; do
    # remove binaries in bin folder
    if [[ $line == $PREFIX/bin/* ]]; then
      rm -rf $line
      #rm -rf $line
    # remove egg folder in lib
    elif [[ $line == $PREFIX/lib/* ]] && [[ $rm_egg = true ]]; then
      # $line without $PREFIX
      temp=${line#$PREFIX}
      
      # variable to save path
      egg_path=""
      
      # loop $temp and extract path until egg folder
      IFS='/' read -ra CHUNKS <<< $temp
      for i in ${CHUNKS[*]}; do
        # loop for every folder in path
        if [[ $i == V4LCapture* ]]; then
          rm -rf "$PREFIX/$egg_path$i"
          # set block not evaluate next line
          rm_egg=false
          break
        else
          # add string to egg_path until egg folder
          egg_path+="$i/"
        fi
      done
    fi
  done < $LOGFILE
else
  error "$LOGFILE not found" 253
fi

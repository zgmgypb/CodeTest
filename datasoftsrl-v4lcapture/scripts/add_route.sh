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

# Requirements:
#   - dialog
#   - sudo

# Error codes
#   - 255: generic error
#   - 254: executable not found

REQS=('dialog' 'sudo')

CMDS=('ip' 'route')

declare -a ETH

function error {
  >&2 echo "$(basename $0): $1"
  exit $2
}

function list_eth {
  c=0
  for i in /sys/class/net/*; do
    iface=$(basename $i)
    if [[ $iface == eth* ]] || [[ $iface == eno* ]] || [[ $iface == ens* ]] ||\
      [[ $iface == enp* ]] || [[ $iface == enx* ]]; then
      ETH[c]=$iface
      let c=c+1
    fi
  done
  
  if [[ ${#ETH[*]} -eq 0 ]]; then
    error 'cannot find useful eth* interface' 255
  fi
}

function dialog_eth {
  eth=""
  
  c=0
  for i in ${ETH[*]}; do
    eth+="$c $i "
    let c=c+1
  done
}

# check if requirements are satisfied
for i in ${REQS[*]}; do
  type -P "$i" >/dev/null 2>/dev/null || error "$i command not found" 254
done

# check what command to use
pos=-1
c=0
for i in ${CMDS[*]}; do
  type -P "$i" >/dev/null 2>/dev/null
  if [[ $? -eq 0 ]]; then
    pos=$c
    break
  fi
  let c=c+1
done

if [[ $pos -eq -1 ]]; then
  error "cannot find a suitable net tool: install $(tput bold)${CMDS[0]}\
      $(tput sgr0)" 254
fi

# get a list of eth* interfaces
list_eth

# if there is choiche use a dialog
if [[ ${#ETH[*]} -gt 1 ]]; then
  # create a dialog formatted list of eth*s
  dialog_eth
  
  # dialog
  { iface=$(dialog --backtitle "V4LCapture" --title "Add static route" --menu \
    "Choose an interface:" 10 30 ${#ETH[*]} $eth 2>&1 1>&3-) ;} 3>&1
  eth=${ETH[iface]}
else
  eth=${ETH[0]}
fi

# exec command to add static route
if [[ $pos -eq 0 ]]; then
  sudo ip route add 224.0.0.0/4 dev $eth >/dev/null 2>/dev/null
  ip route | grep "224.0.0.0"
elif [[ $pos -eq 1 ]]; then
  sudo route add -net 224.0.0.0 netmask 240.0.0.0 dev $eth >/dev/null \
      2>/dev/null
  route | grep "224.0.0.0"
fi
# check if command has completed well
check=$?
if [[ $check -ne 0 ]] && [[ $check -ne 2 ]] ; then
  error 'cannot set static route' 255
fi

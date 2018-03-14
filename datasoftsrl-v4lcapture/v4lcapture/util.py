#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

import os, json, re, shutil
import subprocess as sp
from os import path

from v4lcapture import error

def print_c(string):
  '''
  Prints utf-8 decodec string from unicode string.

  < string: string to be printed
  '''
  print(string.decode(), end='')

def s2b(string):
  '''
  String to byte conversion.

  < string: string to be converted
  
  > utf-8 byte encoded string
  '''
  return string.encode()

def b2s(bytearr):
  '''
  Byte to string conversion.

  < byatearr: byte array to be decoded into string
  
  > utf-8 decodec string
  '''
  return bytearr.decode()

def list_devices():
  '''
  Return list of /dev/video* devices.
  
  > ret: list of /dev/video* devices
  '''
  ret = []
  for dev in os.listdir('/dev'):
    if dev.startswith('video'):
      ret.append('/dev/{}'.format(dev))
  return ret

def check_devices():
  '''
  Check if at least one /dev/video* device exists.
  '''
  for file in os.listdir('/dev'):
    if file.startswith('video'):
      return
  error.critical('no /dev/video* devices found')

def list_eths():
  '''
  Return list of eth* interfaces.
  '''
  ret = []
  for eth in os.listdir('/sys/class/net/'):
    if eth.startswith('eth') or eth.startswith('eno') or\
        eth.startswith('ens') or eth.startswith('enp') or \
        eth.startswith('enx'):
      ret.append(eth)
  return ret

def check_eths():
  '''
  Check if at least one eth* interface exists.
  
  < error: Error object
  '''
  for file in os.listdir('/sys/class/net/'):
    if file.startswith('eth') or file.startswith('eno') or\
        file.startswith('ens') or file.startswith('enp') or \
        file.startswith('enx'):
      return
  error.critical('no useful eth* interfaces found')

def extract_ip(address):
  '''
  Extract ip from string address.
  
  < address: string containing ip
  
  > ip into form (192, 168, 1, 1)
  '''
  regex = r'[0-9]+(?:\.[0-9]+){3}'
  
  ip = re.findall(regex, address)
  if len(ip) > 0:
    bits = ip[0].split('.')
    return (int(bits[0]), int(bits[1]), int(bits[2]), int(bits[3]))
  else:
    return (-1, -1, -1, -1)

def extract_port(address):
  '''
  Extract port from string address.
  
  < address: string containing :port
  
  > returns integer port
  '''
  try:
    port = int(address.split(':')[-1])
    return port
  except ValueError:
    return -1


class Config:
  
  '''
  Handles the reading and saving of json config.
  '''
  
  PATH = path.join(path.expanduser('~'), '.v4lcapture.json')
  
  def __init__(self, cwd):
    '''
    Searches for a config file in defined places.
    
    < cwd: string current working directory
    '''
    self._read()
  
  def _read(self):
    '''
    Reads the json config file.
    If not found create a json object (dict).
    '''
    if path.isfile(self.PATH):
      try:
        with open(self.PATH, 'r') as conf:
          self.conf = json.load(conf)
          error.log('{} loaded'.format(self.PATH))
      except OSError:
        error.critical('cannot open {}'.format(self.PATH))
    else:
      self.conf = {}
  
  def write(self):
    '''
    Writes the json config file.
    '''
    try:
      with open(self.PATH, 'w') as conf:
        json.dump(self.conf, conf, indent=4)
        error.log('changes written to {}'.format(self.PATH))
    except IOError:
      error.error('error writing to {}; changes not applied'.format(
          self.PATH))
  
  def get_dict(self):
    '''
    Returns json dict.
    
    > conf: json dict
    '''
    return self.conf
  
  def get(self, key):
    '''
    Get config parameter.
    
    < key: key to search into json dict
    
    > value corresponding to key or ''
    '''
    if key in self.conf:
      return self.conf[key]
    else:
      return ''
  
  def set(self, key, value):
    '''
    Set config parameter.
    Create it if it doesn't exist.
    
    < key: key to add/edit
    < value: value for key
    '''
    self.conf[key] = value

class Route:
  
  '''
  Set a static route in order to allow multicast traffic.
  '''
  
  execs = (
    'ip',
    'route',
  )
  
  def __init__(self):
    '''
    Check commands and select a net tool to use.
    '''
    # test for polkit
    if shutil.which('pkexec') is None:
      self.err.error('cannot find libpolkit')
    
    # search a suitable tool
    self.route = self._search()

  def _search(self):
    '''
    Searches into path for a net tool able to operate.
    
    > i: position into self.execs
    '''
    for i,x in enumerate(self.execs):
      cmd = shutil.which(x)
      if cmd is not None:
        return i
    error.error('cannot find a suitable command: '
      'install one of this tools {}'.format(self.execs))
  
  def _build(self, eth):
    '''
    Builds a list for command to pass to subprocess.
    
    < eth: the chosen eth interface
    
    > skel: command separated into chunks
    '''
    skel = ['pkexec']
    if self.route == 0:
      # ip route add 224.0.0.0/4 dev eth0
      skel.append(self.execs[self.route])
      skel.append('route')
      skel.append('add')
      skel.append('224.0.0.0/4')
      skel.append('dev')
      skel.append(eth)
      return skel
    elif self.route == 1:
      # route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0
      skel.append(self.execs[self.route])
      skel.append('add')
      skel.append('-net')
      skel.append('224.0.0.0')
      skel.append('netmask')
      skel.append('240.0.0.0')
      skel.append('dev')
      skel.append(eth)
      return skel
  
  def execute(self, eth):
    '''
    Effectively execute the command with selected tool, using subprocess.
    
    < eth: the chosen eth interface
    '''
    cmd = self._build(eth)
    try:
      sp.check_call(cmd, stdout=sp.DEVNULL, stderr=sp.DEVNULL)
    except sp.CalledProcessError as e:
      if e.returncode == 2:
        error.log('static route for multicast streaming already set')
      else:
        error.error('cannot set static route for multicast streaming')

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

from ctypes import *
from ctypes import util as cutil

from v4lcapture import util, error
from v4lcapture.util import s2b, b2s

class V4LCapture():
  
  '''
  Python side v4lcapture library wrapper.
  Wraps C functions with ctypes, working only with primitives.
  Reference in python.c source file.
  '''
  
  def __init__(self):
    lib = cutil.find_library('v4lcapture')
    if lib is not None:
      self.v4l = cdll.LoadLibrary(lib)
      error.log('libv4lcapture.so shared library loaded')
    else:
      error.critical('cannot load libv4lcapture.so shared library')

##### Utility #####
  
  def version(self):
    self.v4l.version.restype = c_char_p
    
    version = self.v4l.version()
    return b2s(version)
  
  def initCtx(self):
    self.v4l.initCtx()
  
  def openDevice(self):
    self.v4l.openDevice()
  
  def initDevice(self):
    self.v4l.initDevice()

  def initConvCtx(self):
    self.v4l.initConvCtx()
  
  def startCapture(self):
    self.v4l.startCapture()

  def initRGB(self):
    self.v4l.initRGB()

  def initScaler(self):
    self.v4l.initScaler()

  def initMuxEnc(self):
    self.v4l.initMuxEnc()
  
  def capTimeout(self):
    self.v4l.capTimeout()
  
  def readFrame(self):
    self.v4l.readFrame.restype = c_int
    
    read = self.v4l.readFrame()
    if read == 0:
      return False
    else:
      return True
  
  def RGB(self):
    self.v4l.RGB()
  
  def scale(self):
    self.v4l._scale()
    
  def muxEncode(self):
    self.v4l.muxEncode()
  
  def writeCache(self):
    self.v4l.writeCache()
  
  def uninitRGB(self):
    self.v4l.uninitRGB()
  
  def uninitScaler(self):
    self.v4l.uninitScaler()
  
  def uninitMuxEnc(self):
    self.v4l.uninitMuxEnc()
  
  def uninitConvCtx(self):
    self.v4l.uninitConvCtx()
  
  def stopCapture(self):
    self.v4l.stopCapture()
  
  def closeDevice(self):
    self.v4l.closeDevice()

  def closeLog(self):
    self.v4l.closeLog()

##### Setters #####

  def setDevice(self, device_name):
    c_device_name = create_string_buffer(s2b(device_name))
    self.v4l.setDevice.argtypes = [c_char_p]
  
    self.v4l.setDevice(c_device_name)
  
  def setIO(self, io):
    if io == 'read':
      c_io = c_int(1)
    else:
      c_io = c_int(2)
    self.v4l.setIO.argtypes = [c_int]
    
    self.v4l.setIO(c_io)
  
  def setFrames(self, number):
    c_number = c_uint64(number)
    self.v4l.setFrames.argtypes = [c_uint64]
    
    self.v4l.setFrames(c_number)
  
  def setPixFmt(self, hex4, hex3, hex2, hex1):
    a = c_char(s2b(hex4))
    b = c_char(s2b(hex3))
    c = c_char(s2b(hex2))
    d = c_char(s2b(hex1))
    self.v4l.setPixFmt.argtypes = [c_char, c_char, c_char, c_char]
    
    self.v4l.setPixFmt(a, b, c, d)
  
  def setCrop(self, top, left, width, height):
    c_top = c_int(top)
    c_left = c_int(left)
    c_width = c_int(width)
    c_height = c_int(height)
    self.v4l.setCrop.argtypes = [c_int, c_int, c_int, c_int]
    
    self.v4l.setCrop(c_top, c_left, c_width, c_height)

  def setFormat(self, width, height):
    c_width = c_int(width)
    c_height = c_int(height)
    self.v4l.setFormat.argtypes = [c_int, c_int]
    
    self.v4l.setFormat(c_width, c_height)
  
  def setFps(self, num, den):
    c_num = c_int(num)
    c_den = c_int(den)
    self.v4l.setFormat.argtypes = [c_int, c_int]
    
    self.v4l.setFps(c_num, c_den)
  
  def setFilename(self, filename):
    c_filename = create_string_buffer(s2b(filename))
    self.v4l.setFilename.argtype = [c_char_p]
    
    self.v4l.setFilename(c_filename)
  
  def setCRF(self, crf):
    c_crf = c_int(crf)
    self.v4l.setCRF.argtypes = [c_int]
    
    self.v4l.setCRF(c_crf)
  
  def setMuxer(self, muxer):
    c_muxer = create_string_buffer(s2b(muxer))
    self.v4l.setMuxer.argtype = [c_char_p]
    
    self.v4l.setMuxer(c_muxer)

  def setTune(self, tune):
    c_tune = create_string_buffer(s2b(tune))
    self.v4l.setTune.argtype = [c_char_p]
    
    self.v4l.setTune(c_tune)

  def setPreset(self, preset):
    c_preset = create_string_buffer(s2b(preset))
    self.v4l.setPreset.argtype = [c_char_p]
    
    self.v4l.setPreset(c_preset)

##### Getters #####

  def getDevice(self):
    self.v4l.getDevice.restype = c_char_p
  
    device_name = self.v4l.getDevice()
    return b2s(device_name)
  
  def getIO(self):
    self.v4l.getIO.restype = c_int
    
    io = self.v4l.getIO()
    if io == 1:
      return 'read'
    elif io == 2:
      return 'mmap'
    else:
      return None
  
  def getFrames(self):
    self.v4l.getFrames.restype = c_uint64
    
    return self.v4l.getFrames()
  
  def getCropcap(self):
    self.v4l.getCropcap.restype = c_int
    
    cropcap = self.v4l.getCropcap()
    if cropcap == 0:
      return False
    else:
      return True
  
  def getFpsList(self):
    self.v4l.getFpsList.restype = POINTER(500 * c_int)
    
    fps_list = self.v4l.getFpsList().contents
    
    # return a list of tuples as big as the real number of fps (num, den)
    ret = []
    i = 0;
    while i < len(fps_list):
      num = fps_list[i]
      den = fps_list[i+1]
      if num != -1 and den != -1:
        ret.append((num, den))
      else:
        break
      i += 2
    
    return tuple(ret)
  
  def getDefFps(self):
    self.v4l.getDefFps.restype = POINTER(2 * c_int)
    
    def_fps = self.v4l.getDefFps().contents
    return (def_fps[0], def_fpst[1])
  
  def getFps(self):
    self.v4l.getFps.restype = POINTER(2 * c_int)
    
    fps = self.v4l.getFps().contents
    return (fps[0], fps[1])
  
  def getStrFps(self):
    self.v4l.getStrFps.restype = c_int
    
    return self.v4l.getStrFps()
  
  def getRatio(self):
    self.v4l.getRatio.restype = c_double
    
    return self.v4l.getRatio()
  
  def getDefCrop(self):
    self.v4l.getDefCrop.restype = POINTER(4 * c_int)
    
    def_crop = self.v4l.getDefCrop().contents
    return (def_crop[0], def_crop[1], def_crop[2], def_crop[3]) 
  
  def getCrop(self):
    self.v4l.getCrop.restype = POINTER(4 * c_int)
    
    crop = self.v4l.getCrop().contents
    return (crop[0], crop[1], crop[2], crop[3])
  
  def getDefFormat(self):
    self.v4l.getDefFormat.restype = POINTER(2 * c_int)
    
    def_format = self.v4l.getDefFormat().contents
    return (def_format[0], def_format[1])
  
  def getFormat(self):
    self.v4l.getFormat.restype = POINTER(2 * c_int)
    
    _format = self.v4l.getFormat().contents
    return (_format[0], _format[1])
  
  def _getArea(self):
    self.v4l.getArea.restype = c_uint
    
    return self.v4l.getArea()
  
  def _getQArea(self):
    self.v4l.getQArea.restype = c_uint
    
    return self.v4l.getQArea()
  
  def getFilename(self):
    self.v4l.getFilename.restype = c_char_p
  
    filename = self.v4l.getFilename()
    return b2s(filename)
  
  def getCRF(self):
    self.v4l.getCRF.restype = c_int
    
    return self.v4l.getCRF()
  
  def getMuxer(self):
    self.v4l.getMuxer.restype = c_char_p
  
    muxer = self.v4l.getMuxer()
    return b2s(muxer)
  
  def getPreset(self):
    self.v4l.getPreset.restype = c_char_p
  
    preset = self.v4l.getPreset()
    return b2s(preset)
  
  def getRGBData(self):
    area = self._getArea()
    self.v4l.getRGBData.restype = POINTER(3 * area * c_uint8)
    
    try:
      data = self.v4l.getRGBData().contents
      return bytes(data)
    except ValueError:
      return None
 
  def getTune(self):
    self.v4l.getTune.restype = c_char_p
  
    tune = self.v4l.getTune()
    return b2s(tune)

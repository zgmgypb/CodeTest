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

from PyQt5.QtCore import QThread, pyqtSignal

from v4lcapture import util, error

class CaptureParams:
  
  '''
  Handle changing, restoring and saving parameters to inject into capture
  thread.
  '''
  
  # valid keys in parameters dict
  VALID = (
    'device',
    'io',
    'frames',
    'pix_fmt',
    'crop',
    'format',
    'fps',
    'filename',
    'crf',
    'muxer',
    'tune',
    'preset',
  )
  
  def __init__(self, conf):
    '''
    Restores parameters if any.
    
    < conf: Config object
    '''
    self.conf = conf
    
    self.enc = False # output or encoding
    self.params = {} # capture parameters
    
    # restore saved parameters
    self._restore()
  
  def _check_device(self):
    devs = util.list_devices()
    if self.get_attr('device') not in devs:
      self.set_attr('device', sorted(devs)[0])
  
  def _restore(self):
    '''
    Restore parameters from config settings dict.
    '''
    settings = self.conf.get('settings')
    if settings != '':
      self.params = settings.copy()
      self._check_device()
  
  def store(self):
    '''
    Store parameters into json config for saving.
    '''
    self.conf.set('settings', self.params.copy())
  
  def default(self):
    '''
    Go back to cable parameters (or default ones).
    '''
    device = self.get_attr('device')
    self.clear()
    self.set_attr('device', device)
  
  def set_attr(self, key, value):
    '''
    Create or change an attribute into parameters dict.
    
    < key: string key to set
    < value: string/tuple paired with key
    '''
    if key in self.VALID:
      self.params[key] = value
      error.log('{} changed to {}'.format(key, value))
  
  def get_attr(self, key):
    '''
    Get the correspoding value to key or False if key does not exist.
    
    < key: string to search into dict.
    '''
    if key in self.params:
      return self.params[key]
    return False
  
  def clear(self):
    '''
    Empty parameters dict.
    '''
    self.params.clear()
  
  def toggle_encoding(self):
    '''
    Set encoding to true if false or the other way round.
    '''
    self.enc = not self.enc
    error.log('encoding set to {}'.format(self.enc))
  
  def encoding(self):
    '''
    Return state of encoding.
    '''
    return self.enc


class CaptureThread(QThread):
  
  ready = pyqtSignal()
  
  def __init__(self, app, v4l, params, update):
    super().__init__()
    self.app = app
    self.v4l = v4l
    self.params = params
    self.update = update
    self.work = True # set to False to terminate immediately
  
  def quit(self):
    self.work = False
  
  def run(self):
    enc = self.params.encoding()
    
    self.v4l.initCtx()
    
    device = self.params.get_attr('device')
    if device:
      self.v4l.setDevice(device)
    io = self.params.get_attr('io')
    if io:
      self.v4l.setIO(io)
    self.v4l.openDevice()
    
    crop = self.params.get_attr('crop')
    if crop:
      self.v4l.setCrop(crop[0], crop[1], crop[2], crop[3])
    format_ = self.params.get_attr('format')
    if format_:
      self.v4l.setFormat(format_[0], format_[1])
    fps = self.params.get_attr('fps')
    if fps:
      self.v4l.setFps(1, fps)
    pix_fmt = self.params.get_attr('pix_fmt')
    if pix_fmt:
      self.v4l.setPixFmt(pix_fmt[0], pix_fmt[1], pix_fmt[2], pix_fmt[3])
    self.v4l.initDevice()
    
    frames = self.params.get_attr('frames')
    if frames:
      self.v4l.setFrames(frames)
    
    self.v4l.initConvCtx()
    
    filename = self.params.get_attr('filename')
    if filename:
      self.v4l.setFilename(filename)
    crf = self.params.get_attr('crf')
    if crf:
      self.v4l.setCRF(crf)
    muxer = self.params.get_attr('muxer')
    if muxer:
      self.v4l.setMuxer(muxer)
    tune = self.params.get_attr('tune')
    if tune:
      self.v4l.setTune(tune)
    preset = self.params.get_attr('preset')
    if preset:
      self.v4l.setPreset(preset)
    
    self.v4l.startCapture()
    
    self.v4l.initRGB()
    
    if enc:
      self.v4l.initScaler()
      self.v4l.initMuxEnc()
    
    infinite = False
    if frames == False or frames == 0:
      infinite = True
    
    self.ready.emit()
    
    while self.work and (infinite or count > 0):
      while True:
        self.v4l.capTimeout()
        
        if self.v4l.readFrame():
          self.v4l.RGB()
          # function called here to minimize lag in UI thread
          self.update(self.v4l.getRGBData())
          if enc:
            self.v4l.scale()
            self.v4l.muxEncode()
          break
      if not infinite:
        count -= 1
    
    if enc:
      self.v4l.writeCache()
    
    self.v4l.uninitRGB()
    
    if enc:
      self.v4l.uninitScaler()
      self.v4l.uninitMuxEnc()
    
    self.v4l.uninitConvCtx()
    self.v4l.stopCapture()
    
    self.v4l.closeDevice()

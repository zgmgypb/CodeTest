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

from PyQt5.Qt import Qt
from PyQt5.QtCore import QSize
from PyQt5.QtWidgets import (
  QWidget,
  QLabel,
  QDialog,
  QTabWidget,
  QSlider,
  QVBoxLayout,
  QGridLayout,
  QDialogButtonBox,
  QComboBox,
  QFrame,
  QSpinBox,
  QSizePolicy,
)
from PyQt5.QtGui import QImage, QPixmap

from v4lcapture import util, error

class RgbPreview(QLabel):
  
  '''
  Resizes the internal pixmal of the QLabel based on QPixmap size, maintaining
  aspect ratio.
  '''
  
  def __init__(self, parent=None):
    super().__init__(parent)
    self.pixmap = None
  
  def setPixmap(self, data, width, height):
    '''
    Actually set pixmap of QLabel, with resized width and height.
    
    < data: RGB data to display
    < width: width of RGB data
    < height: height of RGB data
    '''
    self.width = width
    self.height = height
    self.ar = width / height
    self.image = QImage(data, width, height, QImage.Format_RGB888)
    self.pixmap = QPixmap.fromImage(self.image)
    super().setPixmap(self.pixmap.scaled(self.sizeHint(), Qt.KeepAspectRatio,
        Qt.SmoothTransformation))
  
  
  def heightForWidth(self, width):
    '''
    Calculates height for specified width, with respect to aspect ratio.
    
    < width: image width
    '''
    return width / self.ar
  
  def sizeHint(self):
    '''
    Returns QSize of label.
    '''
    if self.pixmap is not None:
      if self.width > 640:
        self.width /= 2
      return QSize(self.width, self.heightForWidth(self.width))
    return QSize()


class RouteDialog(QDialog):

  '''
  Dialog with OK and Cancel buttons.
  '''

  def __init__(self, parent=None):
    '''
    Create widgets and set layout.
    
    < parent: optional parent window
    '''
    super().__init__(parent)
    
    if parent is not None:
      self.root = parent.parentWidget()
    
    self.route = util.Route()
    
    self.layout = QVBoxLayout()
    self.setLayout(self.layout)
    
    self._widgets()
    self._buttons()

  def _widgets(self):
    '''
    Create widgets to changes options.
    '''
    # eth
    eth_l = QLabel('Ethernet interface')
    eth_l.setStyleSheet('''
      QLabel {
        font-weight: bold;
      }
    ''')
    self.layout.addWidget(eth_l, Qt.AlignCenter)
    self.eth_cb = QComboBox(self)
    self.eth_cb.addItems(sorted(util.list_eths()))
    self.layout.addWidget(self.eth_cb)
  
  def _buttons(self):
    '''
    Create OK, cancel buttons.
    '''
    self.bbox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
    # accept action
    self.bbox.accepted.connect(self.accept)
    # reject action
    self.bbox.rejected.connect(self.reject)
    self.layout.addWidget(self.bbox)
  
  def accept(self):
    '''
    Apply changes and hide window.
    '''
    self.route.execute(self.eth_cb.currentText())
    error.log('static route applied')
    self.hide()
  
  def reject(self):
    '''
    Discard changes.
    '''
    self.hide()

class SimpleDialog(QDialog):
  
  '''
  Dialog with OK and Cancel buttons.
  '''
  
  def __init__(self, v4l, params, parent=None):
    '''
    Create widgets and set layout.
    
    < v4l: v4l wrapper object
    < params: CaptureParams object
    < parent: optional parent window
    '''
    super().__init__(parent)
    
    if parent is not None:
      self.root = parent.parentWidget()
    self.v4l = v4l
    self.params = params
    
    self.device = None
    
    self.layout = QVBoxLayout()
    self.setLayout(self.layout)
    
    self._widgets()
    self._buttons()
  
  def _widgets(self):
    '''
    Create widgets to changes options.
    '''
    # device
    device_l = QLabel('Device')
    device_l.setStyleSheet('''
      QLabel {
        font-weight: bold;
      }
    ''')
    self.layout.addWidget(device_l, Qt.AlignCenter)
    self.device_cb = QComboBox(self)
    self.device_cb.addItems(sorted(util.list_devices()))
    self.device_cb.setCurrentText(self.v4l.getDevice())
    self.layout.addWidget(self.device_cb)
  
  def _buttons(self):
    '''
    Create OK, apply, cancel buttons.
    '''
    self.bbox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Apply
        | QDialogButtonBox.Cancel)
    # accept action
    self.bbox.accepted.connect(self.accept)
    # apply action
    apply_b = self.bbox.button(QDialogButtonBox.Apply)
    apply_b.clicked.connect(self.apply)
    # reject action
    self.bbox.rejected.connect(self.reject)
    self.layout.addWidget(self.bbox)
  
  def apply(self):
    '''
    Apply changes.
    '''
    self.params.clear()
    self.params.set_attr('device', self.device_cb.currentText())
    self.root.capture()
    error.log('device change applied')
  
  def accept(self):
    '''
    Apply changes and hide window.
    '''
    self.apply()
    self.hide()
  
  def reject(self):
    '''
    Discard changes.
    '''
    self.hide()

class TabDialog(QDialog):
  
  '''
  Dialog window with tabs and OK and Cancel buttons.
  '''
  
  def __init__(self, v4l, params, parent=None):
    '''
    Create widgets and set layout.
    
    < v4l: v4l wrapper object
    < params: CaptureParams object
    < parent: optional parent window
    '''
    super().__init__(parent)
    
    if parent is not None:
      self.root = parent.parentWidget()
    self.v4l = v4l
    self.params = params
    self.tabs = QTabWidget()
    
    self.layout = QVBoxLayout()
    self.setLayout(self.layout)
    
    self._tabs()
    self._buttons()
  
  def _tabs(self):
    '''
    Create the two tabs.
    '''
    self.captab = CaptureTab(self.v4l, self.params)
    self.streamtab = StreamTab(self.v4l, self.params)
    self.tabs.addTab(self.captab, 'Capture')
    self.tabs.addTab(self.streamtab, 'Streaming')
    self.layout.addWidget(self.tabs)
  
  def _buttons(self):
    '''
    Create OK, apply, cancel buttons.
    '''
    self.bbox = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Apply
        | QDialogButtonBox.Cancel)
    # accept action
    self.bbox.accepted.connect(self.accept)
    # apply action
    apply_b = self.bbox.button(QDialogButtonBox.Apply)
    apply_b.clicked.connect(self.apply)
    # reject action
    self.bbox.rejected.connect(self.reject)
    self.layout.addWidget(self.bbox)
  
  def apply(self):
    '''
    Apply changes.
    '''
    self.captab.set()
    self.streamtab.set()
    self.root.capture()
    error.log('parameters changes applied')
  
  def accept(self):
    '''
    Apply changes and hide window.
    '''
    self.apply()
    self.hide()
  
  def reject(self):
    '''
    Discard changes.
    '''
    self.hide()
  
  def update(self):
    '''
    Update view of widgets fot the two tabs.
    '''
    self.captab.update()
    self.streamtab.update()


class Tab(QWidget):
  
  '''
  Defines an interface to a tab.
  '''
  
  def __init__(self, v4l, params, parent=None):
    '''
    Set layout.
    
    < v4l: v4l wrapper object
    < params: CaptureParams object
    < parent: optional parent window
    '''
    super().__init__(parent)
    
    self.v4l = v4l
    self.params = params
    
    self.layout = QGridLayout()
    self.layout.setSpacing(10)
    self.setLayout(self.layout)
  
  # abstract method
  def set(self):
    '''
    Saves and applies new setting into parameters.
    '''
    pass
  
  # abstract method
  def update(self):
    '''
    Update view of displayed widgets.
    '''
    pass


class CaptureTab(Tab):
  
  '''
  Tab containing all the capture settings and tools.
  '''
  
  def __init__(self, v4l, params, parent=None):
    '''
    Create widgets and set layout.
    
    < v4l: v4l wrapper object
    < params: CaptureParams object
    < parent: optional parent window
    '''
    super().__init__(v4l, params, parent)
    
    self._widgets()
    self.update()
  
  def _widgets(self):
    '''
    Create widgets into tab.
    '''
    style = '''
      QLabel {
        font-weight: bold;
      }
    '''
    
    # io
    io_l = QLabel('I/O Interface', self)
    io_l.setStyleSheet(style)
    self.layout.addWidget(io_l, 0, 0, 1, 2, Qt.AlignTop | Qt.AlignCenter)
    self.io_cb = QComboBox(self)
    self.io_cb.addItems(('read', 'mmap'))
    self.io_cb.setEnabled(False)
    self.layout.addWidget(self.io_cb, 1, 0, 1, 2, Qt.AlignTop)
    
    # separator
    sep = QFrame(self)
    sep.setFrameShape(QFrame.HLine)
    sep.setFrameShadow(QFrame.Sunken)
    sep.setLineWidth(1)
    sep.setMidLineWidth(0)
    self.layout.addWidget(sep, 2, 0, 1, 2, Qt.AlignTop)
    
    # fps
    fps_l = QLabel('Frames per second', self)
    fps_l.setStyleSheet(style)
    self.layout.addWidget(fps_l, 3, 0, 1, 2, Qt.AlignTop | Qt.AlignCenter)
    self.fps_cb = QComboBox(self)
    self.layout.addWidget(self.fps_cb, 4, 0, 1, 2, Qt.AlignTop | Qt.AlignCenter)
    
    # crop
    crop_l = QLabel('Cropping', self)
    crop_l.setStyleSheet(style)
    self.layout.addWidget(crop_l, 5, 0, 1, 2, Qt.AlignTop | Qt.AlignCenter)
    ## top
    top_l = QLabel('Top', self)
    top_l.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
    self.layout.addWidget(top_l, 6, 0, 1, 1, Qt.AlignTop)
    self.crop_t_sb = QSpinBox(self)
    self.layout.addWidget(self.crop_t_sb, 6, 1, 1, 1, Qt.AlignTop)
    ## left
    left_l = QLabel('Left', self)
    left_l.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
    self.layout.addWidget(left_l, 7, 0, 1, 1, Qt.AlignTop)
    self.crop_l_sb = QSpinBox(self)
    self.layout.addWidget(self.crop_l_sb, 7, 1, 1, 1, Qt.AlignTop)
    ## width
    width_l = QLabel('Width', self)
    width_l.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
    self.layout.addWidget(width_l, 8, 0, 1, 1, Qt.AlignTop)
    self.crop_w_sb = QSpinBox(self)
    self.layout.addWidget(self.crop_w_sb, 8, 1, 1, 1, Qt.AlignTop)
    ## height
    height_l = QLabel('Height', self)
    height_l.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
    self.layout.addWidget(height_l, 9, 0, 1, 1, Qt.AlignTop)
    self.crop_h_sb = QSpinBox(self)
    self.layout.addWidget(self.crop_h_sb, 9, 1, 1, 1, Qt.AlignTop)
    
    # format
    format_l = QLabel('Window size', self)
    format_l.setStyleSheet(style)
    self.layout.addWidget(format_l, 10, 0, 1, 2, Qt.AlignTop | Qt.AlignCenter)
    ## width
    fwidth_l = QLabel('Width', self)
    fwidth_l.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
    self.layout.addWidget(fwidth_l, 11, 0, 1, 1, Qt.AlignTop)
    self.format_w_sb = QSpinBox(self)
    self.layout.addWidget(self.format_w_sb, 11, 1, 1, 1, Qt.AlignTop)
    ## height
    fheight_l = QLabel('Height', self)
    fheight_l.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
    self.layout.addWidget(fheight_l, 12, 0, 1, 1, Qt.AlignTop)
    self.format_h_sb = QSpinBox(self)
    self.layout.addWidget(self.format_h_sb, 12, 1, 1, 1, Qt.AlignTop)
  
  def set(self):
    # io
    self.params.set_attr('io', self.io_cb.currentText())
    
    # fps
    self.params.set_attr('fps', int(self.fps_cb.currentText()))
    
    # crop
    self.params.set_attr('crop', (
      self.crop_t_sb.value(),
      self.crop_l_sb.value(),
      self.crop_w_sb.value(),
      self.crop_h_sb.value(),
    ))
    
    # format
    self.params.set_attr('format', (
      self.format_w_sb.value(),
      self.format_h_sb.value(),
    ))
  
  def update(self):
    # global
    (dc_t, dc_l, dc_w, dc_h) = self.v4l.getDefCrop()
    (c_t, c_l, c_w, c_h) = self.v4l.getCrop()
    (df_w, df_h) = self.v4l.getDefFormat()
    (f_w, f_h) = self.v4l.getFormat()
    
    # io
    self.io_cb.setCurrentText(self.v4l.getIO())
    
    # fps
    self.fps_cb.clear()
    for num,den in sorted(self.v4l.getFpsList(), reverse=True):
      self.fps_cb.addItem(str(den))
    curr_fps = str(self.v4l.getFps()[1])
    if len(curr_fps) == 1:
      self.fps_cb.setCurrentText(curr_fps[0])
    else:
      self.fps_cb.setCurrentText(curr_fps[:2])
    
    # crop
    if self.v4l.getCropcap():
      ## top
      self.crop_t_sb.setRange(0, dc_h-1)
      self.crop_t_sb.setValue(c_t)
      ## left
      self.crop_l_sb.setRange(0, dc_w-1)
      self.crop_l_sb.setValue(c_l)
      ## width
      self.crop_w_sb.setRange(1, dc_w)
      # solution to error in format
      if c_t == 0:
        self.crop_w_sb.setValue(c_w)
      else:
        self.crop_w_sb.setValue(c_w - c_l)
      ## height
      self.crop_h_sb.setRange(1, dc_h)
      # solution to error in format
      if c_t == 0:
        self.crop_h_sb.setValue(c_h)
      else:
        self.crop_h_sb.setValue(c_h - c_t)
    else:
      self.crop_t_sb.setEnabled(False)
      self.crop_l_sb.setEnabled(False)
      self.crop_w_sb.setEnabled(False)
      self.crop_h_sb.setEnabled(False)
    
    # format
    MAX_WIDTH = 4096
    MAX_HEIGHT = 2160
    self.format_w_sb.setRange(1, MAX_WIDTH)
    self.format_w_sb.setValue(f_w)
    self.format_h_sb.setRange(1, MAX_HEIGHT)
    self.format_h_sb.setValue(f_h)


class StreamTab(Tab):
  
  '''
  Tab containing all streaming settings.
  '''
  
  def __init__(self, v4l, params, parent=None):
    '''
    Create widgets and set layout.
    
    < v4l: v4l wrapper object
    < params: CaptureParams object
    < parent: optional parent window
    '''
    super().__init__(v4l, params, parent)
    
    self._widgets()
    self.update()
  
  def _widgets(self):
    '''
    Create widgets into tab.
    '''
    style = '''
      QLabel {
        font-weight: bold;
      }
    '''
    
    # filename
    filename_l = QLabel('Streaming address', self)
    filename_l.setStyleSheet(style)
    self.layout.addWidget(filename_l, 0, 0, 1, 7, Qt.AlignTop | Qt.AlignCenter)
    ## ip
    ip_l = QLabel('udp://', self)
    ip_l.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
    self.layout.addWidget(ip_l, 1, 0, 1, 1, Qt.AlignTop)
    self.ip_1_sb = QSpinBox(self)
    self.ip_1_sb.setRange(239, 239)
    self.layout.addWidget(self.ip_1_sb, 1, 1, 1, 1, Qt.AlignTop)
    self.ip_2_sb = QSpinBox(self)
    self.ip_2_sb.setRange(0, 254)
    self.layout.addWidget(self.ip_2_sb, 1, 2, 1, 1, Qt.AlignTop)
    self.ip_3_sb = QSpinBox(self)
    self.ip_3_sb.setRange(0, 254)
    self.layout.addWidget(self.ip_3_sb, 1, 3, 1, 1, Qt.AlignTop)
    self.ip_4_sb = QSpinBox(self)
    self.ip_4_sb.setRange(1, 254)
    self.layout.addWidget(self.ip_4_sb, 1, 4, 1, 1, Qt.AlignTop)
    ## port
    port_l = QLabel(':', self)
    port_l.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
    self.layout.addWidget(port_l, 1, 5, 1, 1, Qt.AlignTop)
    self.port_sb = QSpinBox(self)
    self.port_sb.setRange(1025, 65535)
    self.layout.addWidget(self.port_sb, 1, 6, 1, 1, Qt.AlignTop)
    
    # separator
    sep = QFrame(self)
    sep.setFrameShape(QFrame.HLine)
    sep.setFrameShadow(QFrame.Sunken)
    sep.setLineWidth(1)
    sep.setMidLineWidth(0)
    self.layout.addWidget(sep, 2, 0, 1, 7)
    
    # streaming parameters
    str_l = QLabel('Streaming settings', self)
    str_l.setStyleSheet(style)
    self.layout.addWidget(str_l, 3, 0, 1, 7, Qt.AlignTop | Qt.AlignCenter)
    
    # crf
    crf_l = QLabel('Constant Rate Factor', self)
    self.layout.addWidget(crf_l, 4, 0, 1, 7, Qt.AlignCenter)
    crf_sl_l = QLabel(self)
    crf_sl_l.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Preferred)
    self.layout.addWidget(crf_sl_l, 5, 0, 1, 1, Qt.AlignTop)
    self.crf_sl = QSlider(Qt.Horizontal, self)
    self.crf_sl.setTickPosition(QSlider.TicksLeft)
    self.crf_sl.setInvertedAppearance(True)
    self.crf_sl.setRange(1, 51)
    self.crf_sl.valueChanged.connect(
      lambda e:
        crf_sl_l.setText(str(e))
    )
    self.layout.addWidget(self.crf_sl, 5, 1, 1, 6, Qt.AlignTop)
    
    # muxer
    mux_l = QLabel('Container', self)
    self.layout.addWidget(mux_l, 6, 0, 1, 7, Qt.AlignCenter)
    self.mux_cb = QComboBox(self)
    #self.mux_cb.setEnabled(False)
    self.mux_cb.addItems((
      'mpegts',
      #'matroska',
      #'ogg',
      #'mp4',
      'avi',
      #'nut',
    ))
    self.layout.addWidget(self.mux_cb, 7, 0, 1, 7, Qt.AlignCenter)
    
    # preset
    pres_l = QLabel('H.264 Preset', self)
    self.layout.addWidget(pres_l, 8, 0, 1, 7, Qt.AlignCenter)
    self.pres_cb = QComboBox(self)
    self.pres_cb.addItems((
      'ultrafast',
      'superfast',
      'veryfast',
      'faster',
      'fast',
      'medium',
      'slow',
      'slower',
      'veryslow',
      'placebo',
    ))
    self.layout.addWidget(self.pres_cb, 9, 0, 1, 7, Qt.AlignCenter)
    
    # tune
    tune_l = QLabel('H.264 Tune', self)
    self.layout.addWidget(tune_l, 10, 0, 1, 7, Qt.AlignCenter)
    self.tune_cb = QComboBox(self)
    self.tune_cb.addItems((
      'zerolatency',
      'film',
      'animation',
      'grain',
      'stillimage',
      'psnr',
      'ssim',
      'fastdecode',
    ))
    self.layout.addWidget(self.tune_cb, 11, 0, 1, 7, Qt.AlignCenter)
  
  def set(self):
    # filename
    self.params.set_attr('filename', 'udp://{}.{}.{}.{}:{}'.format(
      self.ip_1_sb.value(),
      self.ip_2_sb.value(),
      self.ip_3_sb.value(),
      self.ip_4_sb.value(),
      self.port_sb.value()
    ))
    
    # crf
    self.params.set_attr('crf', self.crf_sl.value())
    
    # muxer
    self.params.set_attr('muxer', self.mux_cb.currentText())
    
    # preset
    self.params.set_attr('preset', self.pres_cb.currentText())
    
    # tune
    self.params.set_attr('tune', self.tune_cb.currentText())
  
  def update(self):
    filename = self.v4l.getFilename()
    (ip1, ip2, ip3, ip4) = util.extract_ip(filename)
    port = util.extract_port(filename)
    
    # filename
    ## ip
    if ip1 != -1:
      self.ip_1_sb.setValue(ip1)
      self.ip_2_sb.setValue(ip2)
      self.ip_3_sb.setValue(ip3)
      self.ip_4_sb.setValue(ip4)
    ## port
    if port > 0:
      self.port_sb.setValue(port)
    
    # crf
    self.crf_sl.setValue(int(self.v4l.getCRF()))
    
    # muxer
    self.mux_cb.setCurrentText(self.v4l.getMuxer())
    
    # preset
    self.pres_cb.setCurrentText(self.v4l.getPreset())
    
    # tune
    self.tune_cb.setCurrentText(self.v4l.getTune())

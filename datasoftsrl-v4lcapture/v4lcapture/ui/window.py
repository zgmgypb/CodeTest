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

import sys, time, pkg_resources as pkg
from os import path

from PyQt5.Qt import Qt
from PyQt5.QtWidgets import (
  QWidget,
  QMainWindow,
  QApplication,
  QAction,
  QVBoxLayout,
  QPushButton,
  QLabel,
)
from PyQt5.QtGui import QIcon, QCloseEvent, QDesktopServices
from PyQt5.QtCore import QUrl

from v4lcapture import util, error
from v4lcapture.ui.qtwidgets import (
  RgbPreview,
  RouteDialog,
  SimpleDialog,
  TabDialog,
)
from v4lcapture.capture import thread

class Window(QMainWindow):
  
  def __init__(self, conf, argv, v4l):
    '''
    Creates main window of the program.
    
    < conf: Config object
    < argv: command line options
    < v4l: v4l wrapper object
    '''
    # config and wrapper instance
    self.conf = conf
    self.v4l = v4l
    
    # thread: None for evaluating first execution
    self.params = thread.CaptureParams(self.conf)
    self.thread = None
    
    # application
    self.app = QApplication(argv)
    super(Window, self).__init__()
    
    ## title, icon and default size and position
    self.setWindowTitle('V4LCapture')
    iconpath = pkg.resource_filename('v4lcapture', 'v4lcapture.png')
    if path.isfile(iconpath):
      self.setWindowIcon(QIcon(iconpath))
    
    ## ui creation
    self._createUI()
    error.log('created main window')
    self._status(message=False)
    self._createPreview()
    self.capture()
    # route, device, preference (when thread is ready)
    self.thread.ready.connect(self._route)
    self.thread.ready.connect(self._device)
    self.thread.ready.connect(self._preferences)
    # visualize
    self.show()
  
  def _createUI(self):
    '''
    Create a menubar with file and about menu and a central widget with its
    own layout.
    Creates the preferences window opened by omonimous entry in file menu.
    '''
    # menubar
    self.menubar = self.menuBar()
    
    ## file
    file_menu = self.menubar.addMenu('&File')
    
    self.save_act = QAction(QIcon.fromTheme('document-save'),
        '&Save settings', self)
    self.save_act.setShortcut('Ctrl+S')
    self.save_act.triggered.connect(self._save_config)
    file_menu.addAction(self.save_act)
    
    self.rst_act = QAction(QIcon.fromTheme('document-revert'),
        '&Reset settings', self)
    self.rst_act.setShortcut('Ctrl+R')
    self.rst_act.triggered.connect(self._reset_config)
    file_menu.addAction(self.rst_act)
    
    sep_act1 = QAction(self)
    sep_act1.setSeparator(True)
    file_menu.addAction(sep_act1)
    
    self.enc_act = QAction(QIcon(), 'S&treaming', self)
    self.enc_act.setShortcut('Ctrl+T')
    self.enc_act.setCheckable(True)
    self.enc_act.setChecked(False)
    self.enc_act.toggled.connect(self._toggleEncoding)
    file_menu.addAction(self.enc_act)
    
    self.route_act = QAction(QIcon.fromTheme('configure'),
        'Add static rout&e', self)
    self.route_act.setShortcut('Ctrl+E')
    self.route_act.triggered.connect(self._show_route)
    file_menu.addAction(self.route_act)
    
    self.dev_act = QAction(QIcon.fromTheme('configure'),
        'Change capture &board', self)
    self.dev_act.setShortcut('Ctrl+Alt+B')
    self.dev_act.triggered.connect(self._show_device)
    file_menu.addAction(self.dev_act)
    
    self.pref_act = QAction(QIcon.fromTheme('configure'), '&Preferences', self)
    self.pref_act.setShortcut('Ctrl+Alt+P')
    self.pref_act.triggered.connect(self._show_preferences)
    file_menu.addAction(self.pref_act)
    
    sep_act2 = QAction(self)
    sep_act2.setSeparator(True)
    file_menu.addAction(sep_act2)
    
    close_act = QAction(QIcon.fromTheme('application-exit'), '&Quit', self)
    close_act.setShortcut('Ctrl+Q')
    close_act.triggered.connect(self.close)
    file_menu.addAction(close_act)
    
    ## about
    about_menu = self.menubar.addMenu('&About')
    
    wiki_act = QAction(QIcon.fromTheme('help-browser'),
        'Go to wiki...', self)
    wiki_act.triggered.connect(
      lambda:
        QDesktopServices.openUrl(QUrl('https://github.com/datasoftsrl/'
            'v4lcapture/wiki', QUrl.TolerantMode))
    )
    about_menu.addAction(wiki_act)
    
    sep_act3 = QAction(self)
    sep_act3.setSeparator(True)
    about_menu.addAction(sep_act3)
    
    ds_act = QAction(QIcon.fromTheme('applications-internet'),
        'DataSoft Srl', self)
    ds_act.triggered.connect(
      lambda:
        QDesktopServices.openUrl(QUrl('http://www.datasoftweb.com/',
            QUrl.TolerantMode))
    )
    about_menu.addAction(ds_act)
    
    qt_act = QAction(QIcon(':/qt-project.org/qmessagebox/images/qtlogo-64.png'),
        'About Qt', self)
    qt_act.triggered.connect(QApplication.aboutQt)
    about_menu.addAction(qt_act)
    
    self.sb_perm = QLabel()
    self.sb = self.statusBar()
    self.sb.setStyleSheet('''
      QStatusBar QLabel {
        margin-right: 5px;
      }
    ''')
    self.sb.addPermanentWidget(self.sb_perm)
    self.sb.setSizeGripEnabled(False)
    
    # central widget and layout
    self.central = QWidget()
    self.layout = QVBoxLayout()
    self.central.setLayout(self.layout)
    self.setCentralWidget(self.central)
  
  def _status(self, message=True):
    '''
    Sets statusbar text on streaming enabled/disabled.
    
    < message: flash a message if True
    '''
    label = 'address: <b>{}</b>'
    tooltip =  'codec: <b>{}</b>\nformat: <b>{}</b>\nsize: <b>{}</b>\n' \
        'framerate: <b>{}</b>'
    if self.params.encoding():
      (width, height) = self.v4l.getFormat()
      if message:
        self.sb.showMessage('Streaming enabled')
      self.sb_perm.setText(label.format(
        '<font color="green">{}</font>'.format(self.v4l.getFilename())
      ))
      self.sb_perm.setToolTip(
        tooltip.format(
          'H.264',
          self.v4l.getMuxer(),
          '{}x{}'.format(width, height),
          '{} fps'.format(self.v4l.getStrFps())
      ))
    else:
      if message:
        self.sb.showMessage('Streaming disabled')
      self.sb_perm.setText(label.format('n/a'))
      self.sb_perm.setToolTip(
        tooltip.format(
          'n/a',
          'n/a',
          'n/a',
          'n/a'
      ))
  
  '''
  Creates a preview of captured frames, occupying the window size.
  '''
  def _createPreview(self):
    self.preview = RgbPreview(self)
    self.layout.addWidget(self.preview)
  
  def _toggleEncoding(self):
    self.enc_act.toggled.disconnect(self._toggleEncoding)
    self.params.toggle_encoding()
    if self.params.encoding():
      error.log('streaming started at {}'.format(self.v4l.getFilename()))
    else:
      error.log('streaming stopped')
    self._status()
    self.capture()
    self.enc_act.toggled.connect(self._toggleEncoding)
  
  def _save_config(self):
    '''
    Write preferences into json object and prints a message in status bar.
    '''
    self.params.store()
    self.conf.write()
    self.sb.showMessage('Settings saved')
  
  def _reset_config(self):
    '''
    Resets prefereneces to line default and prints a message in status bar.
    '''
    self.params.default()
    self.capture()
    self.sb.showMessage('Settings reset')
  
  def _route(self):
    '''
    Create eth choosing window to add static route for streamings.
    '''
    self.route = RouteDialog(self.central)
    error.log('static route window created')
  
  def _show_route(self):
    '''
    Show eth choosing window.
    '''
    self.route_act.triggered.disconnect(self._show_route)
    self.route.show()
    self.route.setFixedSize(self.dev.sizeHint())
    self.route_act.triggered.connect(self._show_route)
  
  def _device(self):
    '''
    Create device choosing window.
    '''
    self.thread.ready.disconnect(self._device)
    self.dev = SimpleDialog(self.v4l, self.params, self.central)
    self.dev.setWindowTitle('Change capture board')
    self.dev.setWindowIcon(QIcon.fromTheme('preferences-desktop-multimedia'))
    error.log('device choosing window created')
  
  def _show_device(self):
    '''
    Show device choosing window.
    '''
    self.dev_act.triggered.disconnect(self._show_device)
    self.dev.show()
    self.dev.setFixedSize(self.dev.sizeHint())
    self.dev_act.triggered.connect(self._show_device)
  
  def _preferences(self):
    '''
    Create preference window.
    '''
    self.thread.ready.disconnect(self._preferences)
    self.pref = TabDialog(self.v4l, self.params, self.central)
    self.pref.setWindowTitle('Preferences')
    self.pref.setWindowIcon(QIcon.fromTheme('configure'))
    error.log('preferences window created')
  
  def _show_preferences(self):
    '''
    Show preference window.
    '''
    self.pref_act.triggered.disconnect(self._show_preferences)
    self.pref.show()
    self.pref.setFixedSize(self.pref.sizeHint())
    self.pref_act.triggered.connect(self._show_preferences)
  
  def _updatePreview(self, data):
    '''
    Updates the preview with a new frame. Called from capture QThread.
    
    < data: RGB data to display
    '''
    (width, height) = self.v4l.getFormat()
    if data is not None:
      self.preview.setPixmap(data, width, height)
      self.setFixedSize(self.sizeHint())
  
  def capture(self):
    '''
    Instantiate new QThread to capture frames, waiting for previous to stop.
    '''
    if self.thread is not None:
      self.thread.quit()
      self.thread.wait()
    self.thread = thread.CaptureThread(self.app, self.v4l, self.params,
        self._updatePreview)
    self.thread.start()
    # update only if initialized
    try:
      self.thread.ready.connect(self.pref.update)
      self.thread.ready.connect(lambda: self._status(message=False))
    except AttributeError:
      pass
    error.log('capture thread created and started')
  
  def closeEvent(self, event):
    '''
    Closes capture QThread when window is closed.
    Closes other windows open.
    Writes config.
    
    < event: closing event
    '''
    self.thread.quit()
    self.thread.wait()
    error.log('capture thread stopped')
    
    # close only if initialized
    try:
      self.dev.close()
      self.pref.close()
    except AttributeError:
      pass
    
    # close core log
    self.v4l.closeLog()
    error.log('core log closed')
  
  old_w = 0
  old_h = 0
  def resizeEvent(self, event):
    '''
    If current size is different from previous frame size, resize, else do nothing.
    old_w and old_h are defined outside to simulate C static keyword use.
    
    < event: resizing event
    '''
    size = self.sizeHint()
    w = size.width()
    h = size.height()
    if w != self.old_w or h != self.old_h:
      super().resizeEvent(event)
      self.old_w = w
      self.old_h = h
  
  def quit(self):
    '''
    Closes main window.
    '''
    sys.exit(self.app.exec_())

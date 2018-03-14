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

from setuptools import setup, find_packages

import v4lcapture as v4lc

setup(
  name = 'V4LCapture',
  version = v4lc.VERSION,
  url = 'https://github.com/datasoftsrl',
  author='Riccardo Macoratti',
  author_email = 'rmacoratti@datasoftweb.com',
  description = ('Control a v4l2 capture device,'
                 'with H.264 streaming capabilities.'),
  long_description = ('''
    A PyQt5 application that provides a control view to a Video4Linux2 \
    device. Adjustable and saveable settings with the optional H.264/mpegts \
    UDP multicast streaming capability (provided by ffmpeg library).
  '''),
  license = 'GPLv2, GPLv3',
  platforms = ['Linux'],
  classifiers = [
    'Development Status :: 4 - Beta',
    'Environment :: X11 Applications :: Qt',
    'Intended Audience :: Telecommunications Industry',
    'License :: OSI Approved :: GNU General Public License v2 or '
        'later (GPLv2+)',
    'Natural Language :: English',
    'Operating System :: POSIX :: Linux',
    'Programming Language :: Python :: 3 :: Only',
    'Topic :: Multimedia :: Video :: Capture'
  ],
  keywords = 'v4l2 capture device streaming ffmpeg uvcvideo',
  packages = find_packages(),
  scripts = ['v4lcapture/v4lcapture'],
  install_requires = [
    'setuptools'
  ],
  data_files = [
    ('v4lcapture', ['v4lcapture/v4lcapture.png'])
  ],
  zip_safe = False
)

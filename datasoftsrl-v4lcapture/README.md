# V4LCapture
A PyQt5 application that provides a control view to a Video4Linux2 device.
Adjustable and saveable settings with the optional H.264/mpegts UDP multicast
streaming capability (provided by ffmpeg library).

# libv4lcapture
Backend to V4LCapture. It is the C interface to communicate with v4l2 device,
by means of ioctl calls.

# Requirements
- ``python`` >= ``3.3``
  - ``setuptools`` >= ``3.3``
  - ``pyqt5`` >= ``5.2``
- ``ffmpeg`` >= ``2.5.8``
  - ``libavutil`` >= ``54.15.100``
  - ``libavcodec`` >= ``56.13.100``
  - ``libavformat`` >= ``56.15.102``
  - ``libswscale`` >= ``3.1.101``
- ``libpolkit`` >= ``0.105``
  - ``pkexec`` >= ``0.105``

# Installation

## V4LCapture and libv4lcapture
Execute in a terminal (`$` means normal user and `#` means root user):

    $ ./configure [-d] [-p PREFIX]
    $ make
    # make install

## `add_route`
To install (`$` means normal user and `#` means root user):

    $ ./configure [-d] [-p PREFIX]
    # make route

# Uninstallation and cleaning
Execute in a terminal (`$` means normal user and `#` means root user):

    # make uninstall
    # make clean

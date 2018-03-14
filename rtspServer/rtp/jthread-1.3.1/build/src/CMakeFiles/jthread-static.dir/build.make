# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.0

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = F:\soft\cmake-3.0.2-win32-x86\bin\cmake.exe

# The command to remove a file.
RM = F:\soft\cmake-3.0.2-win32-x86\bin\cmake.exe -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = E:\opencv\rtp\jthread-1.3.1

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = E:\opencv\rtp\jthread-1.3.1\build

# Include any dependencies generated for this target.
include src\CMakeFiles\jthread-static.dir\depend.make

# Include the progress variables for this target.
include src\CMakeFiles\jthread-static.dir\progress.make

# Include the compile flags for this target's objects.
include src\CMakeFiles\jthread-static.dir\flags.make

src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj: src\CMakeFiles\jthread-static.dir\flags.make
src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj: ..\src\win32\jmutex.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report E:\opencv\rtp\jthread-1.3.1\build\CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/CMakeFiles/jthread-static.dir/win32/jmutex.cpp.obj"
	cd E:\opencv\rtp\jthread-1.3.1\build\src
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj /FdCMakeFiles\jthread-static.dir/ -c E:\opencv\rtp\jthread-1.3.1\src\win32\jmutex.cpp
<<
	cd E:\opencv\rtp\jthread-1.3.1\build

src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/jthread-static.dir/win32/jmutex.cpp.i"
	cd E:\opencv\rtp\jthread-1.3.1\build\src
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  > CMakeFiles\jthread-static.dir\win32\jmutex.cpp.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E E:\opencv\rtp\jthread-1.3.1\src\win32\jmutex.cpp
<<
	cd E:\opencv\rtp\jthread-1.3.1\build

src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/jthread-static.dir/win32/jmutex.cpp.s"
	cd E:\opencv\rtp\jthread-1.3.1\build\src
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\jthread-static.dir\win32\jmutex.cpp.s /c E:\opencv\rtp\jthread-1.3.1\src\win32\jmutex.cpp
<<
	cd E:\opencv\rtp\jthread-1.3.1\build

src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj.requires:
.PHONY : src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj.requires

src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj.provides: src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj.requires
	$(MAKE) -f src\CMakeFiles\jthread-static.dir\build.make /nologo -$(MAKEFLAGS) src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj.provides.build
.PHONY : src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj.provides

src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj.provides.build: src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj

src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj: src\CMakeFiles\jthread-static.dir\flags.make
src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj: ..\src\win32\jthread.cpp
	$(CMAKE_COMMAND) -E cmake_progress_report E:\opencv\rtp\jthread-1.3.1\build\CMakeFiles $(CMAKE_PROGRESS_2)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building CXX object src/CMakeFiles/jthread-static.dir/win32/jthread.cpp.obj"
	cd E:\opencv\rtp\jthread-1.3.1\build\src
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoCMakeFiles\jthread-static.dir\win32\jthread.cpp.obj /FdCMakeFiles\jthread-static.dir/ -c E:\opencv\rtp\jthread-1.3.1\src\win32\jthread.cpp
<<
	cd E:\opencv\rtp\jthread-1.3.1\build

src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/jthread-static.dir/win32/jthread.cpp.i"
	cd E:\opencv\rtp\jthread-1.3.1\build\src
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  > CMakeFiles\jthread-static.dir\win32\jthread.cpp.i @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) -E E:\opencv\rtp\jthread-1.3.1\src\win32\jthread.cpp
<<
	cd E:\opencv\rtp\jthread-1.3.1\build

src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/jthread-static.dir/win32/jthread.cpp.s"
	cd E:\opencv\rtp\jthread-1.3.1\build\src
	C:\PROGRA~2\MICROS~2.0\VC\bin\cl.exe  @<<
 /nologo /TP $(CXX_FLAGS) $(CXX_DEFINES) /FoNUL /FAs /FaCMakeFiles\jthread-static.dir\win32\jthread.cpp.s /c E:\opencv\rtp\jthread-1.3.1\src\win32\jthread.cpp
<<
	cd E:\opencv\rtp\jthread-1.3.1\build

src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj.requires:
.PHONY : src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj.requires

src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj.provides: src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj.requires
	$(MAKE) -f src\CMakeFiles\jthread-static.dir\build.make /nologo -$(MAKEFLAGS) src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj.provides.build
.PHONY : src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj.provides

src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj.provides.build: src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj

# Object files for target jthread-static
jthread__static_OBJECTS = \
"CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj" \
"CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj"

# External object files for target jthread-static
jthread__static_EXTERNAL_OBJECTS =

src\jthread.lib: src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj
src\jthread.lib: src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj
src\jthread.lib: src\CMakeFiles\jthread-static.dir\build.make
src\jthread.lib: src\CMakeFiles\jthread-static.dir\objects1.rsp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking CXX static library jthread.lib"
	cd E:\opencv\rtp\jthread-1.3.1\build\src
	$(CMAKE_COMMAND) -P CMakeFiles\jthread-static.dir\cmake_clean_target.cmake
	cd E:\opencv\rtp\jthread-1.3.1\build
	cd E:\opencv\rtp\jthread-1.3.1\build\src
	C:\PROGRA~2\MICROS~2.0\VC\bin\link.exe /lib /nologo  /out:jthread.lib @CMakeFiles\jthread-static.dir\objects1.rsp 
	cd E:\opencv\rtp\jthread-1.3.1\build

# Rule to build all files generated by this target.
src\CMakeFiles\jthread-static.dir\build: src\jthread.lib
.PHONY : src\CMakeFiles\jthread-static.dir\build

src\CMakeFiles\jthread-static.dir\requires: src\CMakeFiles\jthread-static.dir\win32\jmutex.cpp.obj.requires
src\CMakeFiles\jthread-static.dir\requires: src\CMakeFiles\jthread-static.dir\win32\jthread.cpp.obj.requires
.PHONY : src\CMakeFiles\jthread-static.dir\requires

src\CMakeFiles\jthread-static.dir\clean:
	cd E:\opencv\rtp\jthread-1.3.1\build\src
	$(CMAKE_COMMAND) -P CMakeFiles\jthread-static.dir\cmake_clean.cmake
	cd E:\opencv\rtp\jthread-1.3.1\build
.PHONY : src\CMakeFiles\jthread-static.dir\clean

src\CMakeFiles\jthread-static.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" E:\opencv\rtp\jthread-1.3.1 E:\opencv\rtp\jthread-1.3.1\src E:\opencv\rtp\jthread-1.3.1\build E:\opencv\rtp\jthread-1.3.1\build\src E:\opencv\rtp\jthread-1.3.1\build\src\CMakeFiles\jthread-static.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : src\CMakeFiles\jthread-static.dir\depend


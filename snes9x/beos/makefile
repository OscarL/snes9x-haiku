## Haiku Generic Makefile v2.6 ##

## Fill in this file to specify the project being created, and the referenced
## Makefile-Engine will do all of the hard work for you. This handles any
## architecture of Haiku.
##
## For more information, see:
## file:///system/develop/documentation/makefile-engine.html

# The name of the binary.
NAME = BeS9x_1.41-1
TARGET_DIR = ..

# The type of binary, must be one of:
#	APP:	Application
#	SHARED:	Shared library or add-on
#	STATIC:	Static library archive
#	DRIVER: Kernel driver
TYPE = APP

# If you plan to use localization, specify the application's MIME signature.
APP_MIME_SIG =

#	The following lines tell Pe and Eddie where the SRCS, RDEFS, and RSRCS are
#	so that Pe and Eddie can fill them in for you.
#%{
# @src->@

#	Specify the source files to use. Full paths or paths relative to the
#	Makefile can be included. All files, regardless of directory, will have
#	their object files created in the common object directory. Note that this
#	means this Makefile will not work correctly if two source files with the
#	same name (source.c or source.cpp) are included from different directories.
#	Also note that spaces in folder names do not work well with this Makefile.
SRCS = \
	../2xsai.cpp \
	../apu.cpp \
	../apudebug.cpp \
	../c4alt.cpp \
	../cheats.cpp \
	../cheats2.cpp \
	../clip.cpp \
	../cpu.cpp \
	../data.cpp \
	../debug.cpp \
	../dma.cpp \
	../dsp1.cpp \
	../gfx.cpp \
	../globals.cpp \
	../i386/2xSaImmxMod.S \
	../i386/bilinear.asm \
	../i386/c4_.asm \
	../i386/cpuexec.S \
	../i386/cpuops.S \
	../i386/fxemu2.asm \
	../i386/fxemu2b.asm \
	../i386/fxemu2c.asm \
	../i386/fxtable.asm \
	../i386/sfxproc.asm \
	../i386/spc.asm \
	../i386/zsnes.asm \
	../i386/zsnesc4.asm \
	../loadzip.cpp \
	../memmap.cpp \
	../obc1.cpp \
	../ppu.cpp \
	../sa1.cpp \
	../sa1cpu.cpp \
	../screenshot.cpp \
	../sdd1.cpp \
	../seta.cpp \
	../seta010.cpp \
	../seta011.cpp \
	../seta018.cpp \
	../snaporig.cpp \
	../snapshot.cpp \
	../snes9x.cpp \
	../soundux.cpp \
	../spc700.cpp \
	../spc7110.cpp \
	../srtc.cpp \
	../tile.cpp \
	../unzip/explode.c \
	../unzip/unreduce.c \
	../unzip/unshrink.c \
	../unzip/unzip.c \
	atime.cpp \
	Cursors.cpp \
	direct_convert.cpp \
	gamestick.cpp \
	joystick.cpp \
	s9x_config_window.cpp \
	s9x_flags.cpp \
	s9x_input_view.cpp \
	s9x_key_names.cpp \
	s9x_list_view.cpp \
	s9x_main_window.cpp \
	s9x_main.cpp \
	s9x_refresh_window.cpp \
	s9x_rominfo.cpp \
	s9x_screenshot.cpp \
	s9x_settings.cpp \
	s9x_string_view.cpp \
	s9x_view.cpp \
	system_menu_bar.cpp \

#	../i386/sa1gs.S \
#	../i386/sa1ops.S \

#	Specify the resource definition files to use. Full or relative paths can be
#	used.
RDEFS =

#	Specify the resource files to use. Full or relative paths can be used.
#	Both RDEFS and RSRCS can be utilized in the same Makefile.
RSRCS = \
	Resource.rsrc

# End Pe/Eddie support.
# @<-src@
#%}

#	Specify libraries to link against.
#	There are two acceptable forms of library specifications:
#	-	if your library follows the naming pattern of libXXX.so or libXXX.a,
#		you can simply specify XXX for the library. (e.g. the entry for
#		"libtracker.so" would be "tracker")
#
#	-	for GCC-independent linking of standard C++ libraries, you can use
#		$(STDCPPLIBS) instead of the raw "stdc++[.r4] [supc++]" library names.
#
#	- 	if your library does not follow the standard library naming scheme,
#		you need to specify the path to the library and it's name.
#		(e.g. for mylib.a, specify "mylib.a" or "path/mylib.a")
LIBS = \
	be \
	device \
	game \
	media \
	root \
	tracker \
	translation \
	../../zlib/libz.a

#	Specify additional paths to directories following the standard libXXX.so
#	or libXXX.a naming scheme. You can specify full paths or paths relative
#	to the Makefile. The paths included are not parsed recursively, so
#	include all of the paths where libraries must be found. Directories where
#	source files were specified are	automatically included.
LIBPATHS =

#	Additional paths to look for system headers. These use the form
#	"#include <header>". Directories that contain the files in SRCS are
#	NOT auto-included here.
SYSTEM_INCLUDE_PATHS = \
	/boot/system/develop/os \
	/boot/system/develop/c++ \
	/boot/system/develop/headers/posix

#	Additional paths paths to look for local headers. These use the form
#	#include "header". Directories that contain the files in SRCS are
#	automatically included.
LOCAL_INCLUDE_PATHS = \
	.	\
	..	\
	beos_includes

#	Specify the level of optimization that you want. Specify either NONE (O0),
#	SOME (O1), FULL (O3), or leave blank (for the default optimization level).
OPTIMIZE := 

# 	Specify the codes for languages you are going to support in this
# 	application. The default "en" one must be provided too. "make catkeys"
# 	will recreate only the "locales/en.catkeys" file. Use it as a template
# 	for creating catkeys for other languages. All localization files must be
# 	placed in the "locales" subdirectory.
LOCALES =

#	Specify all the preprocessor symbols to be defined. The symbols will not
#	have their values set automatically; you must supply the value (if any) to
#	use. For example, setting DEFINES to "DEBUG=1" will cause the compiler
#	option "-DDEBUG=1" to be used. Setting DEFINES to "DEBUG" would pass
#	"-DDEBUG" on the compiler's command line.
DEFINES =

#	Specify the warning level. Either NONE (suppress all warnings),
#	ALL (enable all warnings), or leave blank (enable default warnings).
WARNINGS = 

#	With image symbols, stack crawls in the debugger are meaningful.
#	If set to "TRUE", symbols will be created.
SYMBOLS :=

#	Includes debug information, which allows the binary to be debugged easily.
#	If set to "TRUE", debug info will be created.
DEBUGGER :=

ASSEMBLER_FLAGS = -I../ -DZSNES_FX

#	Specify any additional compiler flags to be used.

# Flags for x86-32
#COMPILER_FLAGS = -Wall -Wno-multichar -Wno-ctor-dtor-privacy -Woverloaded-virtual -O3 -DZLIB -DJOYSTICK_SUPPORT -DSIDEWINDER_SUPPORT -DGRIP_SUPPORT -DSPC700_SHUTDOWN -DCPU_SHUTDOWN -DNO_INLINE_SET_GET -DVAR_CYCLES -DEXECUTE_SUPERFX_PER_LINE -DZSNES_FX -DEXECUTE_SUPERFX_PER_LINE -DZSNES_C4  -DUSE_THREADS -DSPC700_C  -DUNZIP_SUPPORT -DHAVE_3DNOW -DMMX -fomit-frame-pointer -march=pentium -pipe

# Flags for x86-64
COMPILER_FLAGS = -Wno-multichar -Wno-ctor-dtor-privacy -Woverloaded-virtual -O3 -DZLIB -DJOYSTICK_SUPPORT -DSIDEWINDER_SUPPORT -DGRIP_SUPPORT -DSPC700_SHUTDOWN -DCPU_SHUTDOWN -DNO_INLINE_SET_GET -DVAR_CYCLES -DEXECUTE_SUPERFX_PER_LINE -DZSNES_FX -DEXECUTE_SUPERFX_PER_LINE -DZSNES_C4  -DUSE_THREADS -DSPC700_C  -DUNZIP_SUPPORT -DMMX -fomit-frame-pointer \
	-fpermissive \
	-Wno-narrowing -Wno-register -Wno-maybe-uninitialized -Wno-write-strings -Wno-parentheses -Wno-misleading-indentation -Wno-restrict \
	-pipe -fPIC
	#--save-temps


#	Specify any additional linker flags to be used.
LINKER_FLAGS =

#	Specify the version of this binary. Example:
#		-app 3 4 0 d 0 -short 340 -long "340 "`echo -n -e '\302\251'`"1999 GNU GPL"
#	This may also be specified in a resource.
APP_VERSION :=

#	Specify the desired install location for "make install". Only used when "TYPE" is not "DRIVER".
#	Example:
#		INSTALL_DIR = $(shell finddir B_USER_NONPACKAGED_ADDONS_DIRECTORY)/input_server/filters
INSTALL_DIR =

#	Specify where to place the resulting binary (instead of leaving it under the "objects.* dir).
#	Example:
#		TARGET_DIR = .
TARGET_DIR =

#	(Only used when "TYPE" is "DRIVER"). Specify the desired driver install
#	location in the /dev hierarchy. Example:
#		DRIVER_PATH = video/usb
#	will instruct the "driverinstall" rule to place a symlink to your driver's
#	binary in ~/add-ons/kernel/drivers/dev/video/usb, so that your driver will
#	appear at /dev/video/usb when loaded. The default is "misc".
DRIVER_PATH =

## Include the Makefile-Engine
#DEVEL_DIRECTORY := \
#	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
#include $(DEVEL_DIRECTORY)/etc/makefile-engine
include makefile-engine


# xbattbar-pe version
VERSION = $(shell \
	  sed -ne 's/static char \*ReleaseVersion = "\(.*\)";/\1/p' xbattbar-se.c)

# Customize below to fit your system

# paths
PREFIX = /usr/local

# includes and libs
INCS =	$(shell pkg-config --cflags x11)
LIBS =	$(shell pkg-config --libs x11)

# flags
CFLAGS = -std=c89 -pedantic -Werror -Os ${INCS}
LDFLAGS = -s ${LIBS}

# compiler and linker
CC = cc

# End of file

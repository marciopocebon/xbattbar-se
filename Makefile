# xbattbar-se - yet another battery watcher for X11 (suckless edition)
# See LICENSE file for copyright and license details.

include config.mk

SRC = xbattbar-se.c
OBJ = ${SRC:.c=.o}

all: options xbattbar-se

options:
	@echo xbattbar-se build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.h config.mk

config.h:
	@echo creating $@ from config.def.h
	@cp config.def.h $@

xbattbar-se: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f xbattbar-se ${OBJ} xbattbar-se-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p xbattbar-se-${VERSION}
	@cp -R LICENSE Makefile README.md config.def.h config.mk \
		${SRC} xbattbar-se-${VERSION}
	@tar -cf xbattbar-se-${VERSION}.tar xbattbar-se-${VERSION}
	@gzip xbattbar-se-${VERSION}.tar
	@rm -rf xbattbar-se-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f xbattbar-se ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/xbattbar-se

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/xbattbar-se

.PHONY: all options clean dist install uninstall

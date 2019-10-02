# stt - simple time tracker
# See LICENSE file for copyright and license details.

include config.mk

SRC = stt.c
OBJ = ${SRC:.c=.o}

all: options stt

options:
	@echo stt build options:
	@echo "CFLAGS   = ${CFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"
	@echo "CC       = ${CC}"

.c.o:
	@echo CC $<
	@${CC} -c ${CFLAGS} $<

${OBJ}: config.mk

stt: ${OBJ}
	@echo CC -o $@
	@${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	@echo cleaning
	@rm -f stt ${OBJ} stt-${VERSION}.tar.gz

dist: clean
	@echo creating dist tarball
	@mkdir -p stt-${VERSION}
	@cp -R LICENSE Makefile config.mk ${SRC} stt.1 arg.h README.md stt-${VERSION}
	@tar -cf stt-${VERSION}.tar stt-${VERSION}
	@gzip stt-${VERSION}.tar
	@rm -rf stt-${VERSION}

install: all
	@echo installing executable file to ${DESTDIR}${PREFIX}/bin
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f stt ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/stt

uninstall:
	@echo removing executable file from ${DESTDIR}${PREFIX}/bin
	@rm -f ${DESTDIR}${PREFIX}/bin/stt

.PHONY: all options clean dist install uninstall

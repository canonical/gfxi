CFLAGS = -g -Wall

CFLAGS += `pkgconf --cflags libdrm`

LDFLAGS += `pkgconf --libs libdrm`

TARGET = gfxi

all: $(TARGET)

run: $(TARGET)
	./gfxi crtc ACTIVE:1
	./gfxi plane type:Primary CRTC_ID:`./gfxi crtc ACTIVE:1`
	./gfxi connector link-status:Good CRTC_ID:`./gfxi crtc ACTIVE:1`

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c $(LDFLAGS) -o $(TARGET)


DISTFILES=\
Makefile \
main.c \
gfxi.1 \
README.md \
LICENSE \
debian

clean:
	$(RM) $(TARGET)

install: gfxi
	install -d ${DESTDIR}/usr/bin
	install -m 755 gfxi ${DESTDIR}/usr/bin/

uninstall:
	rm -f ${DESTDIR}/usr/bin/gfxi

tarball:
	tar cvzf ../gfxi_1.3.orig.tar.gz $(DISTFILES)

packageupload:
	debuild -S
	debsign ../gfxi_1.3-1_source.changes
	dput ppa:b-stolk/ppa ../gfxi_1.3-1_source.changes


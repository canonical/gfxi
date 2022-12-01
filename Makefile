CFLAGS = -g -Wall

CFLAGS += `pkgconf --cflags libdrm`

LDFLAGS += `pkgconf --libs libdrm`

run: gfxi
	./gfxi connector link-status:Good
	./gfxi crtc ACTIVE:1
	./gfxi plane type:Primary CRTC_ID:`./gfxi crtc ACTIVE:1`

gfxi: main.c
	$(CC) $(CFLAGS) main.c $(LDFLAGS) -o gfxi



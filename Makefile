CFLAGS = -g -Wall

CFLAGS += `pkgconf --cflags libdrm`

LDFLAGS += `pkgconf --libs libdrm`

run: gfxi
	#./gfxi class=crtc ACTIVE:1
	./gfxi class=plane type:Overlay
	#./gfxi class=connector connected=1
	#./gfxi class=plane
	#./gfxi class=crtc
	#./gfxi class=framebuffer


gfxi: main.c
	$(CC) $(CFLAGS) main.c $(LDFLAGS) -o gfxi



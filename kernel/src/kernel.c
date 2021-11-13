
typedef unsigned long long size_t;

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;

typedef struct {
	PSF1_HEADER *psf1_header;
	void *glyphBuffer;
} PSF1_FONT;

typedef struct {
	void *BaseAddress;
	size_t BufferSize;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanline;
} Framebuffer;

Framebuffer *framebuffer;
PSF1_FONT *font;

void putChar(unsigned int colour, char chr, unsigned int xOff, unsigned int yOff) {
	unsigned int *pxPtr = (unsigned int *) framebuffer->BaseAddress;
	char * fontPtr = font->glyphBuffer + (chr * font->psf1_header->charsize);
	for (unsigned long y = yOff; y < yOff + 16; y++) {
		for (unsigned long x = xOff; x < xOff + 8; x++) {
			if ((*fontPtr & (0b10000000 >> (x - xOff))) > 0) {
				*(unsigned int *)(pxPtr + x + (y * framebuffer->PixelsPerScanline)) = colour;
			}
		}
		fontPtr++;
	}
}

void print(unsigned int colour, char* str) {
	unsigned int x = 0;
	char *chr = str;
	putChar(0xffffffff, *chr, 0, 0);
	while (*chr != 0) {
		putChar(colour, *chr, x, 0);
		x += 8;
		chr++;
	}
}

void _start(Framebuffer *fb, PSF1_FONT *ft) {

	framebuffer = fb;
	font = ft;

    print(0xffffffff, "Hello Kernel!");
	// putChar(0xffffffff, 'G', 0, 0);
	// putChar(0xffffffff, 'A', 8, 0);

    return;
}
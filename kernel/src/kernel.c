
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

typedef struct {
	unsigned int x;
	unsigned int y;
} Point;

enum COLOR {
	COLOR_WHITE = 0x00ffffff,
	COLOR_RED = 0x00ff0000,
	COLOR_BLUE = 0x0000ff00,
	COLOR_GREEN = 0x000000ff
};

Framebuffer *framebuffer;
PSF1_FONT *font;
Point cursorPosition;

// put the given character with the given color at the given offset in the framebuffer
void putChar(unsigned int color, char chr, unsigned int xOff, unsigned int yOff) {
	unsigned int *pxPtr = (unsigned int *) framebuffer->BaseAddress;
	char * fontPtr = font->glyphBuffer + (chr * font->psf1_header->charsize);
	for (unsigned long y = yOff; y < yOff + 16; y++) {
		for (unsigned long x = xOff; x < xOff + 8; x++) {
			if ((*fontPtr & (0b10000000 >> (x - xOff))) > 0) {
				*(unsigned int *)(pxPtr + x + (y * framebuffer->PixelsPerScanline)) = color;
			}
		}
		fontPtr++;
	}
}

// print the given string at the cursor position
void print(unsigned int color, char* str) {
	char *chr = str;
	while (*chr != 0) {
		if (*chr == '\n') {
			cursorPosition.x = 0;
			cursorPosition.y += 16;
			chr++;
			continue;
		}
		putChar(color, *chr, cursorPosition.x, cursorPosition.y);
		cursorPosition.x += 8;
		if (cursorPosition.x + 8 > framebuffer->Width) {
			cursorPosition.x = 0;
			cursorPosition.y += 16;
		}
		chr++;
	}
}

void _start(Framebuffer *fb, PSF1_FONT *ft) {

	// Set global variables
	framebuffer = fb;
	font = ft;

	// Initialize cursor
	cursorPosition.x = 0;
	cursorPosition.y = 0;

	char str[] = "Hello Kernel!\n";
	for (int t = 0; t < 30; t++) {
		print(COLOR_RED, str);
	}

    return;
}
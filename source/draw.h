#include "color.h"
#include "point.h"

#pragma once

typedef void(*fnevent)();

enum dweventf : unsigned {
	KeyBackspace = 8, KeyTab = 9, KeyEnter = 10, KeyEscape = 0x1B, KeySpace = 0x20, KeyDelete = 0x7F,
	// input events
	InputSymbol = 0x80, InputTimer, InputKeyUp, InputIdle, InputNeedUpdate, InputUpdate, InputNoUpdate,
	// Keyboard and mouse input (can be overrided by flags)
	MouseLeft, MouseLeftDBL, MouseRight,
	MouseMove, MouseWheelUp, MouseWheelDown,
	KeyLeft, KeyRight, KeyUp, KeyDown, KeyPageUp, KeyPageDown, KeyHome, KeyEnd,
	F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
	// support
	CommandMask = 0x000000FF,
	Ctrl = 0x00000100,
	Alt = 0x00000200,
	Shift = 0x00000400,
};
enum dwimagef {
	ImageMirrorV = 0x0001,
	ImageMirrorH = 0x0002,
	ImageGrayscale = 0x0004,
	ImageNoOffset = 0x0008,
	ImageTransparent = 0x0010,
	ImageColor = 0x0020,
	ImagePallette = 0x0040,
	TextStroke = 0x0080,
	TextItalic = 0x0100,
	TextBold = 0x0200,
	TextUscope = 0x0400,
	TextSingleLine = 0x0800,
	AlignLeft = 0x0000,
	AlignCenter = 0x1000,
	AlignRight = 0x2000,
	AlignLeftCenter = 0x3000,
	AlignCenterCenter = 0x4000,
	AlignRightCenter = 0x5000,
	AlignLeftBottom = 0x6000,
	AlignCenterBottom = 0x7000,
	AlignRightBottom = 0x8000,
	AlignWidth = 0xE000,
	AlignMask = 0xF000,
};
enum dwwindowf {
	WFResize = 0x0010,
	WFMinmax = 0x0020,
	WFMaximized = 0x0040,
	WFAbsolutePos = 0x0080,
};
enum class cursor : unsigned char {
	Arrow, Hand, LeftRight, UpDown, All, No, Edit, Wait,
};

namespace colors {
extern color button, form, window;
extern color border, active;
extern color text, header, special;
namespace tips {
extern color back;
extern color text;
}
}

struct pma {
	char name[4]; // Identifier of current block
	int	size; // Size of all block
	int	count; // Count of records in this block
	const pma* getheader(const char* name) const;
	const char* getstring(int id) const;
};

struct sprite : pma {
	enum encodes { Auto, RAW, RLE, ALC, RAW8, RLE8, ALC1, ALC8 };
	struct frame {
		short sx, sy;
		short ox, oy;
		encodes encode;
		unsigned pallette;
		unsigned offset;
		rect getrect(int x, int y, unsigned flags) const;
	};
	struct cicle {
		short unsigned	start;
		short unsigned	count;
	};
	short int			width, height; // common size of all frames (if applicable)
	short int			ascend, descend; // top or down ascend
	short unsigned		flags; // must be zero
	unsigned			cicles; // count of anim structure
	unsigned			cicles_offset;
	frame				frames[1];
	int					esize() const { return frames[0].offset - (sizeof(sprite) + sizeof(frame) * (count - 1)); }
	const unsigned char* edata() const { return (const unsigned char*)this + sizeof(sprite) + sizeof(frame) * (count - 1); }
	int					ganim(int index, int tick);
	const frame&		get(int id) const { return frames[(id >= count) ? 0 : id]; }
	cicle*				gcicle(int index) { return (cicle*)ptr(cicles_offset) + index; }
	int					glyph(unsigned sym) const;
	const unsigned char* ptr(unsigned o) const { return (unsigned char*)this + o; }
};

namespace metrics {
extern sprite* small;
extern sprite* font;
extern sprite* h1;
extern sprite* h2;
extern sprite* h3;
extern sprite* icons;
extern int border, padding;
}

struct surface {
	struct plugin {
		const char* name;
		const char*	filter;
		plugin*	next;
		static plugin* first;
		plugin(const char* name, const char* filter);
		virtual bool decode(unsigned char* output, int output_bpp, const unsigned char* input, unsigned input_size) = 0;
		virtual bool inspect(int& w, int& h, int& bpp, const unsigned char* input, unsigned size) = 0;
	};
	int	width, height, scanline, bpp;
	unsigned char* bits;
	surface();
	surface(int width, int height, int bpp);
	surface(const char* url, color* pallette = 0);
	~surface();
	constexpr explicit operator bool() const { return bits != 0; }
	static unsigned char* allocator(unsigned char* bits, unsigned size);
	void blend(const surface& source, int alpha);
	void clear() { resize(0, 0, 0, true); }
	void convert(int bpp, color* pallette);
	void flipv();
	unsigned char*		ptr(int x, int y);
	bool read(const char* url, color* pallette = 0, int need_bpp = 0);
	void resize(int width, int height, int bpp, bool alloc_memory);
	void rotate();
	void write(const char* url, color* pallette);
};

extern long	hparam, hparam2; // Command context or parameters
extern const void* hobject; // Command object

extern color fore, fore_stroke;
extern unsigned char alpha; // Current global alpha value
extern point text_next; // After each text output contain position next glyph.
extern point hmouse, dragmouse, caret, camera;
extern int width, height, fsize, dialog_width;
extern unsigned	hkey; // if pressed key or mouse this field has key
extern surface*	canvas; // Current output surface
extern rect	clipping; // Clipping area
extern const sprite* font; // Currently selected font
extern fnevent domodal, ptips, pbeforemodal, pleavemodal;
extern rect	hilite;
extern const void* hilite_object;
extern cursor hcursor; // set this mouse cursor
extern bool	hpressed; // flag if any of mouse keys is pressed
extern bool	text_clipped;
extern bool button_pressed, button_executed, button_hilited, input_disabled;
extern double linw;
extern unsigned text_flags; // textf() function flags
extern long	text_params[16];
extern color* palt;
extern int tab_pixels;
extern fnevent last_scene;
extern char tips_text[2048];
extern point tips_pos;

struct pushfore {
	color fore;
	pushfore() : fore(::fore) {}
	pushfore(color v) : pushfore() { ::fore = v; }
	~pushfore() { ::fore = fore; }
};
struct pushstroke {
	color fore;
	pushstroke() : fore(::fore_stroke) {}
	pushstroke(color v) : fore(::fore_stroke) { ::fore_stroke = v; }
	~pushstroke() { ::fore_stroke = fore; }
};
struct pushfont {
	const sprite* font;
	pushfont() : font(::font) {}
	pushfont(const sprite* v) : font(::font) { ::font = v; }
	~pushfont() { ::font = font; }
};
struct pushrect {
	point caret;
	int	width, height;
	pushrect() : caret(::caret), width(::width), height(::height) {}
	~pushrect() { ::caret = caret; ::width = width; ::height = height; }
};
struct pushscene {
	fnevent scene;
	pushscene(fnevent v) : scene(last_scene) { last_scene = v; }
	~pushscene() { last_scene = scene; }
};

int	aligned(int x, int width, unsigned state, int string_width);
int	alignedh(const rect& rc, const char* string, unsigned state);
int	getbpp();
int getheight();
int	getwidth();
int	textbc(const char* string, int width);
int	texth();
int	texth(const char* string, int width);
int	textw(int sym);
int	textw(const char* string, int count = -1);

void bitmap_write(const char* url, unsigned char* bits, int width, int height, int bpp, int scanline, color* pallette);
void blit(surface& dest, int x, int y, int width, int height, unsigned flags, const surface& source, int x_source, int y_source);
void blit(surface& dest, int x, int y, int width, int height, unsigned flags, const surface& source, int x_source, int y_source, int width_source, int height_source);
void button_clear();
void button_check(unsigned key, bool execute_by_press = false);
void caretright();
void circle(int size);
void circlef(int size);
void dragdrop(fnevent proc);
void fillform();
void fillwindow();
void fire(fnevent proc, long param = 0, long param2 = 0, const void* object = 0);
void fhexagon();
void glyph(int sym, unsigned flags);
void gradv(const color c1, const color c2, int skip = 0);
void gradh(const color c1, const color c2, int skip = 0);
void hexagon();
void image(int x, int y, const sprite* e, int id, int flags);
bool ishilite(const rect& rc);
void line(int x, int y); // Draw line
void linedown();
void lineright();
void lineup();
void linet(int x, int y);
void paint_active_color(fnevent proc);
void paint_border_color(fnevent proc);
void pixel(int x, int y);
void pixel(int x, int y, unsigned char alpha);
void rectb(); // Draw rectangle border
void rectf(); // Draw rectangle area. Right and bottom side is one pixel less.
void rectx();
void rectfocus();
void set(int x, int y);
void setclip(rect v);
void setoffset(int x, int y);
void setpos(int x, int y);
void setpos(int x, int y, int width, int height);
void stroke(int x, int y, const sprite* e, int id, int flags, unsigned char thin = 1, unsigned char* koeff = 0);
void strokeactive();
void strokeborder();
void strokeout(fnevent proc, int dx = 0);
void text(const char* string, int count = -1, unsigned flags = 0);
void texta(const char* string, unsigned state = 0);
void textas(const char* string);
void textc(const char* string, int count = -1, unsigned flags = 0);
void textf(const char* string);
void textf(const char* string, const char*& cashe_string, int& cashe_origin);
void textfs(const char* string);

unsigned char* ptr(int x, int y);
const char* skiptr(const char* string);

inline rect	getrect() { return {caret.x, caret.y, caret.x + width, caret.y + height}; }
inline bool	ishilite() { return ishilite({caret.x, caret.y, caret.x + width, caret.y + height}); }
inline bool	ishilite(int size) { return ishilite({caret.x - size, caret.y - size, caret.x + size, caret.y + size}); }
inline void image(const sprite* e, int id, int flags) { image(caret.x, caret.y, e, id, flags); }
inline void	setclip() { clipping.set(0, 0, getwidth(), getheight()); }
inline void	setclipall() { setclip({caret.x, caret.y, caret.x + width, caret.y + height}); }
inline void	setpos(const rect& v) { setpos(v.x1, v.y1, v.width(), v.height()); }

void breakmodal(long result);
void buttoncancel();
void buttonok();
void buttonparam();
void execute(fnevent proc, long value = 0, long value2 = 0, const void* object = 0);
long getresult();
bool ismodal();
bool running_scene();
long scene(fnevent proc);
void set_need_update();
void next_scene(fnevent v);
void start_scene();

void set_dark_theme();
void set_light_theme();

void sys_caption(const char* value);
void sys_create_window(int width, int height);
void sys_cursor(bool enable);
void sys_input();
void sys_redraw();
void sys_update_window();

void cbsetuc();
void cbsetsht();
void cbsetint();
void cbsetptr();
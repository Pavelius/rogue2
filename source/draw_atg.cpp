#include "answers.h"
#include "draw.h"
#include "draw_atg.h"
#include "io_stream.h"
#include "print.h"
#include "pushvalue.h"
#include "stringbuilder.h"

// #define NOART

static char sb_console[4096];
static const char* answer_title;
static const char* answer_cancel_text;
static int answer_columns;
static void* current_tab;
static point last_separator;

stringbuilder sb(sb_console);
fnevent atg_menu;
void* current_avatar;
long current_avatar_post;

static void atg_hotkey_text(const char* title) {
	pushfore push;
	fore = fore.mix(colors::form, 192);
	if(title)
		text(title);
	auto dx = textw('0') * 3;
	caret.x += dx; width -= dx;
}

static void atg_hotkey(unsigned key) {
	char temp[4];
	switch(key) {
	case KeyEscape: atg_hotkey_text("X)"); break;
	default: temp[0] = key; temp[1] = ')'; temp[2] = 0; atg_hotkey_text(temp); break;
	}
}

void paint_hilite() {
	if(!button_hilited && !button_pressed)
		return;
	pushfore push(colors::active);
	auto push_alpha = alpha;
	auto push_height = height;
	if(button_pressed)
		alpha = 32;
	else
		alpha = 64;
	rectf();
	height = push_height;
	alpha = push_alpha;
}

static void paint_header() {
	if(!answers::header || !answers::header[0])
		return;
	pushfont push_font(metrics::h1);
	pushfore push_fore(colors::header);
	char temp[260]; stringbuilder sb(temp);
	sb.add(answers::header);
	texta(temp, AlignCenter);
	caret.y += texth() + metrics::padding;
}

static void paint_console() {
	if(!answers::string || !answers::string[0])
		return;
	textf(answers::string);
	caret.y += metrics::padding;
}

static void paint_ask() {
	if(!answer_title || !answer_title[0])
		return;
	pushfore push(colors::header);
	char temp[512]; stringbuilder sb(temp);
	sb.add(answer_title);
	textf(temp);
	caret.y += metrics::padding;
}

static void atg_paintcell(int index, long value, const char* title) {
	if(!title)
		return;
	auto push = caret;
	if(index == -1)
		index = answers::last->getcount();
	unsigned key = anhotkey(index);
	pushfore push_fore;
	auto push_width = width;
	atg_hotkey(key);
	textfs(title);
	button_check(key);
	if(button_hilited) {
		fore = fore.mix(colors::button, button_pressed ? 128 : 216);
		// hilite_object = value;
		tips_pos.x = caret.x - metrics::border * 2 - metrics::padding;
		tips_pos.y = caret.y;
	} else if(button_pressed)
		fore = fore.mix(colors::button, 128);
	textf(title);
	caret = push;
	width = push_width;
	height += metrics::padding;
}

static void paint_picture() {
#ifndef NOART
	if(!answers::resid || !answers::resid[0])
		return;
	auto p = gres(answers::resid, "images");
	if(!p)
		return;
	image(caret.x, caret.y, p, 0, 0);
	caret.y += p->get(0).sy + metrics::border + metrics::padding;
#endif // NOART
}

static void stroke_bar() {
	auto push_caret = caret;
	line(caret.x, caret.y + height);
	line(caret.x + width, caret.y);
	line(caret.x, caret.y - height);
	caret = push_caret;
}

static void bar_border() {
	auto push_fore = fore;
	fore = colors::border;
	stroke_bar();
	fore = push_fore;
}

static void bar_text_selected(const char* format) {
	auto push_fore = fore;
	fore = colors::active;
	texta(format, AlignCenterCenter);
	fore = push_fore;
}

void paint_bar(const char* name, fnevent proc) {
	auto push_width = width;
	auto push_height = height;
	auto push_caret = caret;
	width = textw(name) + metrics::border * 2;
	if(!current_tab)
		current_tab = (void*)proc;
	button_check(0, true);
	if(button_hilited)
		hilite_object = (void*)proc;
	caret.y += 1; height -= 1;
	if(current_tab == proc) {
		fillwindow();
		bar_border();
		bar_text_selected(name);
	} else
		texta(name, AlignCenterCenter);
	fire(cbsetptr, (long)proc, 0, &current_tab);
	caret = push_caret;
	caret.x += width;
	height = push_height;
	width = push_width;
}

static void line_horizontal() {
	auto push_caret = caret;
	caret.x -= metrics::border;
	line(caret.x + width + metrics::border * 2, caret.y);
	caret = push_caret;
}

void paint_separator() {
	height = 1;
	if(last_separator == caret)
		return;
	pushfore push(colors::border);
	line_horizontal();
	last_separator = caret;
}

void paint_window_center(const char* format) {
	pushrect push;
	pushfore push_fore(colors::window);
	auto push_alpha = alpha;
	textfs(format);
	caret.x -= width / 2;
	setoffset(-metrics::border, -metrics::border);
	alpha = 128;
	rectf();
	alpha = push_alpha;
	rectb();
	setoffset(metrics::border, metrics::border);
	fore = colors::text;
	textf(format);
}

void paint_window_info(const char* format) {
	auto push_caret = caret;
	caret.x = getwidth() / 2;
	caret.y = metrics::border * 2 + metrics::padding;
	paint_window_center(format);
	caret = push_caret;
}

void paint_status_text() {
	if(tips_text[0] == 0)
		return;
	pushrect push;
	caret.x = metrics::padding + metrics::border;
	caret.y = getheight() - texth() - metrics::border - 1;
	width = getwidth() - caret.x * 2;
	auto push_width = width;
	textfs(tips_text);
	caret.x += (push_width - width) / 2;
	textf(tips_text);
}

void paint_status_bar() {
	auto push_caret = caret;
	auto push_height = height;
	auto dy = texth() + metrics::border * 2; height = dy;
	caret.y = getheight() - dy;
	gradv(colors::form, colors::window, 0);
	pushfore push_fore(colors::border);
	line_horizontal();
	caret = push_caret;
	height = push_height - dy;
}

static void paint_menu() {
	pushrect push;
	auto py = texth() + metrics::border + metrics::padding;
	caret.y = metrics::border + height - py;
	height = py;
	line_horizontal();
	if(atg_menu)
		atg_menu();
}

bool allow_paint() {
	return caret.in(clipping);
}

void paint_button(const char* format, const void* object, bool choose) {
	if(!format || format[0] == 0)
		return;
	if(!allow_paint())
		return;
	auto push_width = width;
	textfs(format);
	width = push_width;
	height += metrics::padding * 2;
	button_check(0);
	paint_hilite();
	caret.y += metrics::padding;
	textf(format);
	if(button_hilited)
		hilite_object = object;
	if(button_executed && choose)
		execute(buttonparam, (long)object);
	caret.y += metrics::padding;
}

static int find_avatars(void** source, int count, void* current) {
	for(auto i = 0; i < count; i++) {
		if(source[i] == current)
			return i;
	}
	return -1;
}

static void change_avatar() {
	current_avatar = (void*)hparam;
	if(current_avatar_post)
		breakmodal(current_avatar_post);
}

static void paint_avatar(const sprite* ps, const void* player, unsigned key, bool mark_player, int hit_percent) {
#ifndef NOART
	if(ps)
		image(ps, 0, 0);
#endif // NOART
	if(hit_percent != 100) {
		pushrect push;
		auto push_alpha = alpha; alpha = 192;
		fore = color(76, 12, 28);
		if(!hit_percent)
			fore = color(32, 32, 32);
		height = (100 - hit_percent) * 64 / 100;
		caret.y = caret.y + 64 - height;
		rectf();
		alpha = push_alpha;
	}
	button_clear();
	button_check(key);
	if(mark_player) {
		fore = colors::red.mix(colors::active);
		rectb();
	} else if(player == current_avatar) {
		fore = colors::active;
		rectb();
	}
	if(button_hilited) {
		hilite_object = player;
		fore = button_pressed ? colors::active.mix(colors::form) : colors::active;
		pushrect push;
		setoffset(1, 1);
		rectb();
	}
	fire(change_avatar, (long)player, 0, &current_avatar);
}

void paint_avatars(void** source, int count, fnstatus getavatar, void* current_player, fnvalue gethits) {
	char temp[260]; stringbuilder sb(temp);
	if(!current_avatar || (find_avatars(source, count, current_avatar) == -1))
		current_avatar = source[0];
	pushrect push;
	pushfore push_fore;
	width = 64; height = 64;
	for(auto i = 0; i < count; i++) {
		auto p = source[i];
		if(!p)
			continue;
		sb.clear(); getavatar(p, sb);
		if(temp[0] == 0)
			continue;
		paint_avatar(gres(temp, "avatars"), p, F1 + i, p == current_player, gethits(p));
		caret.x += width + 1;
	}
	push.caret.y += height + 1 + metrics::padding;
}

static void paint_page() {
	if(!current_tab)
		return;
	pushrect push;
	pushfore push_fore(colors::text);
	auto dy = texth() + metrics::border * 2;
	auto py = texth() + metrics::border + metrics::padding + 4;
	auto push_clip = clipping; setclip({caret.x, caret.y, caret.x + width, getheight() - dy - py});
	height = dy;
	last_separator = caret;
	auto p = (fnevent)current_tab;
	if(p)
		p();
	clipping = push_clip;
}

static void paint_panel() {
	pushfore push(colors::border);
	auto push_caret = caret;
	width = 320;
	setpos(getwidth() - metrics::border * 2 - width, 0);
	line(caret.x, caret.y + height + metrics::border * 2);
	setpos(getwidth() - metrics::border - width, metrics::border);
	paint_picture();
	paint_menu();
	paint_page();
	width = getwidth() - metrics::border * 4 - width;
	caret = push_caret;
}

static void print_file_error(const char* format, const char* format_param) {
	static io::file file("errors.txt", StreamWrite | StreamText);
	if(!file)
		return;
	if(format_param) {
		char temp[4096]; stringbuilder sb(temp);
		sb.addv(format, format_param);
		file << temp;
	} else
		file << format;
}

static void paint_background() {
	fillform();
	paint_status_bar();
	setoffset(metrics::border, metrics::border);
}

static void choose_answers_scene() {
	pushvalue push_text(text_flags, (unsigned)TextBold);
	paint_background();
	paint_panel();
	paint_header();
	paint_console();
	paint_ask();
	answers::last->paintanswers(atg_paintcell, answer_columns, answer_cancel_text);
}

long choose_answers(const char* title, const char* cancel_text, int columns) {
	answer_title = title;
	answer_cancel_text = cancel_text;
	answer_columns = columns;
	scene(choose_answers_scene); // Run as scene allow correct dragging api.
	return getresult();
}

void fixclear() {
	sb.clear();
}

void fixmsg(messagen id) {
	sb.addsep('\n');
	sb.addv(getname(id), 0);
}

static int atg_initialize() {
	set_dark_theme();
	metrics::h1 = (sprite*)loadb("fonts/h1.pma");
	metrics::h2 = (sprite*)loadb("fonts/h2.pma");
	metrics::h3 = (sprite*)loadb("fonts/h3.pma");
	metrics::font = (sprite*)loadb("fonts/font.pma");
	metrics::small = (sprite*)loadb("fonts/small.pma");
	metrics::icons = (sprite*)loadb("fonts/icons.pma");
	font = metrics::font;
	fore = colors::text;
	fore_stroke = colors::border;
	print_proc = print_file_error;
	ptips = paint_status_text;
	answers::string = sb_console;
	sys_create_window(800, 600);
	sys_caption(getname((messagen)0));
	return 0;
}

int main(int argc, char* argv[]) {
	auto r = atg_initialize();
	if(r)
		return r;
	next_scene(game_run);
	start_scene();
	return 0;
}

#ifndef __GNUC__
int _stdcall WinMain(void* ci, void* pi, char* cmd, int sw) {
	return main(0, 0);
}
#endif // _DEBUG
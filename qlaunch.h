/*
 * qlaunch.h - qlaunch declarations
 *
 * This file contains declarations for global variables and functions used in qlaunch.
 * It is part of the qlaunch project, released under the MIT License.
 */

#ifndef QLAUNCH_H
#define QLAUNCH_H

#include <X11/Xlib.h> /* For Display, Window, GC, XFontStruct, XKeyEvent */

/* Global variables. */
extern Display *dpy;
extern Window root, win;
extern GC gc;
extern XFontStruct *font;
extern int screen;
extern int running;

/* Colors */
extern unsigned long bg, fg, hl_bg, hl_fg;

/* Input and items */
extern char input_buffer[];
extern int input_len;
extern char *items[];
extern int num_items;
extern char **matches;
extern int num_matches;

/* Function declarations. */
void
setup_x(void);

void
setup_colors_and_font(void);

void
grab_keyboard(void);

int
is_executable(const char *path);

void
populate_items(void);

void
cleanup(void);

void
run(void);

void
draw(void);

void
handle_keypress(XKeyEvent *ev);

void
execute_command(void);

void
filter_items(void);

#endif /* QLAUNCH_H */

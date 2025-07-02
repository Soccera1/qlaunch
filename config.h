/* config.h - qlaunch configuration
*/
/*
*/
/* To change settings, edit this file and then recompile qlaunch with:
*/
/* gcc -o qlaunch qlaunch.c -lX11
*/
/*

*/
/* --- Appearance ---

*/
/* Font to use. Use 'xfontsel' to find a font you like.
*/
static const char *font_name = "9x15";

/* Colors (see https://www.color-hex.com/ for color codes)
*/
static const char *color_fg      = "#f8f8f2"; /* Foreground (text) */
static const char *color_bg      = "#282a36"; /* Background */
static const char *color_hl_fg   = "#50fa7b"; /* Highlighted Foreground */
static const char *color_hl_bg   = "#44475a"; /* Highlighted Background */

/* The prompt to display before the input box.
*/
static const char *prompt = ">";


/* --- Geometry ---

*/
/* Height of the bar in pixels.
*/
#define WINDOW_HEIGHT 24

/* Horizontal padding on the left and between items.
*/
#define HORZ_PADDING 10

/* Width of the text input cursor in pixels.
*/
#define CURSOR_WIDTH 2


/* --- Internal Limits ---

*/
/* Maximum length of the user input string.
*/
#define MAX_INPUT_LEN 256

/* Maximum number of executables to read from PATH.
*/
/* Increase if you have a very large number of commands.
*/
#define MAX_ITEMS 4096
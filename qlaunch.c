#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <sys/wait.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

// Include the configuration file
#include "config.h"

// --- Globals ---
static Display *dpy;
static Window root, win;
static GC gc;
static XFontStruct *font;
static int screen;
static int running = 1;

// Colors
static unsigned long bg, fg, hl_bg, hl_fg;

// Input and items
static char input_buffer[MAX_INPUT_LEN] = {0};
static int input_len = 0;
static char *items[MAX_ITEMS];
static int num_items = 0;
static char **matches = NULL;
static int num_matches = 0;

// --- Function Prototypes ---
void setup_x();
void setup_colors_and_font();
void grab_keyboard();
void populate_items();
void cleanup();
void run();
void draw();
void handle_keypress(XKeyEvent *ev);
void execute_command();
void filter_items();

// --- Implementation ---

void setup_colors_and_font() {
    XColor color;
    Colormap colormap = DefaultColormap(dpy, screen);

    // Allocate colors from config.h
    XAllocNamedColor(dpy, colormap, color_bg, &color, &color);
    bg = color.pixel;
    XAllocNamedColor(dpy, colormap, color_fg, &color, &color);
    fg = color.pixel;
    XAllocNamedColor(dpy, colormap, color_hl_bg, &color, &color);
    hl_bg = color.pixel;
    XAllocNamedColor(dpy, colormap, color_hl_fg, &color, &color);
    hl_fg = color.pixel;

    // Load font from config.h
    font = XLoadQueryFont(dpy, font_name);
    if (!font) {
        fprintf(stderr, "qlaunch: cannot load font '%s', falling back to 'fixed'.\n", font_name);
        font = XLoadQueryFont(dpy, "fixed");
    }
    if (!font) {
        fprintf(stderr, "qlaunch: cannot load font 'fixed'.\n");
        exit(1);
    }
}

void setup_x() {
    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "qlaunch: cannot open display.\n");
        exit(1);
    }

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    setup_colors_and_font();

    // Create window
    win = XCreateSimpleWindow(dpy, root, 0, 0, DisplayWidth(dpy, screen), WINDOW_HEIGHT,
                              0, fg, bg);

    // Set window properties to be like a dock/dmenu
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    XChangeWindowAttributes(dpy, win, CWOverrideRedirect, &attrs);

    // Hint to the window manager that this is a dock window
    Atom window_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    Atom window_type_dock = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(dpy, win, window_type, XA_ATOM, 32, PropModeReplace, (unsigned char *)&window_type_dock, 1);

    // Create graphics context
    gc = XCreateGC(dpy, win, 0, NULL);
    XSetFont(dpy, gc, font->fid);

    // Select events to listen for
    XSelectInput(dpy, win, ExposureMask | KeyPressMask);

    // Map window to screen
    XMapRaised(dpy, win);
}

void grab_keyboard() {
    for (int i = 0; i < 1000; i++) {
        if (XGrabKeyboard(dpy, root, True, GrabModeAsync, GrabModeAsync, CurrentTime) == GrabSuccess) {
            return;
        }
        usleep(500);
    }
    fprintf(stderr, "qlaunch: cannot grab keyboard.\n");
    cleanup();
    exit(1);
}

int is_executable(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        return (st.st_mode & S_IXUSR) && S_ISREG(st.st_mode);
    }
    return 0;
}

void populate_items() {
    char *path_env = getenv("PATH");
    if (!path_env) return;

    char *path = strdup(path_env);
    char *token = strtok(path, ":");

    while (token != NULL) {
        DIR *dir = opendir(token);
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
                    char full_path[1024];
                    snprintf(full_path, sizeof(full_path), "%s/%s", token, entry->d_name);
                    
                    int found = 0;
                    for(int i = 0; i < num_items; i++) {
                        if(strcmp(items[i], entry->d_name) == 0) {
                            found = 1;
                            break;
                        }
                    }

                    if (!found && num_items < MAX_ITEMS - 1 && is_executable(full_path)) {
                        items[num_items++] = strdup(entry->d_name);
                    }
                }
            }
            closedir(dir);
        }
        token = strtok(NULL, ":");
    }
    free(path);
    filter_items();
}

void filter_items() {
    free(matches);
    matches = NULL;
    num_matches = 0;

    if (input_len == 0) {
        return;
    }
    
    matches = malloc(sizeof(char*) * num_items);
    if (!matches) return;

    for (int i = 0; i < num_items; i++) {
        if (strstr(items[i], input_buffer) == items[i]) {
            matches[num_matches++] = items[i];
        }
    }
}

void draw() {
    int text_y = WINDOW_HEIGHT - ((WINDOW_HEIGHT - font->ascent - font->descent) / 2);
    int x_offset = HORZ_PADDING;
    
    XSetForeground(dpy, gc, bg);
    XFillRectangle(dpy, win, gc, 0, 0, DisplayWidth(dpy, screen), WINDOW_HEIGHT);

    // Draw prompt
    XSetForeground(dpy, gc, fg);
    if (prompt && strlen(prompt) > 0) {
        XDrawString(dpy, win, gc, x_offset, text_y, prompt, strlen(prompt));
        x_offset += XTextWidth(font, prompt, strlen(prompt)) + font->min_bounds.width;
    }

    // Draw input buffer
    XDrawString(dpy, win, gc, x_offset, text_y, input_buffer, input_len);
    x_offset += XTextWidth(font, input_buffer, input_len);
    
    // Draw cursor
    XSetForeground(dpy, gc, fg);
    XFillRectangle(dpy, win, gc, x_offset, (WINDOW_HEIGHT - font->ascent)/2, CURSOR_WIDTH, font->ascent);

    x_offset += HORZ_PADDING;

    // Draw matches
    for (int i = 0; i < num_matches; i++) {
        int text_width = XTextWidth(font, matches[i], strlen(matches[i]));
        if (x_offset + text_width + HORZ_PADDING > DisplayWidth(dpy, screen)) break;

        if (i == 0) {
            XSetForeground(dpy, gc, hl_bg);
            XFillRectangle(dpy, win, gc, x_offset - (HORZ_PADDING/2), 0, text_width + HORZ_PADDING, WINDOW_HEIGHT);
            XSetForeground(dpy, gc, hl_fg);
        } else {
            XSetForeground(dpy, gc, fg);
        }
        
        XDrawString(dpy, win, gc, x_offset, text_y, matches[i], strlen(matches[i]));
        x_offset += text_width + HORZ_PADDING;
    }

    XFlush(dpy);
}

void handle_keypress(XKeyEvent *ev) {
    char buf[32];
    KeySym ksym;
    int len = XLookupString(ev, buf, sizeof(buf), &ksym, NULL);

    if (ksym == XK_Escape) {
        running = 0;
    } else if (ksym == XK_Return) {
        execute_command();
        running = 0;
    } else if (ksym == XK_BackSpace) {
        if (input_len > 0) {
            input_buffer[--input_len] = '\0';
        }
    } else if (len > 0 && input_len < MAX_INPUT_LEN - 1) {
        input_buffer[input_len] = buf[0];
        input_buffer[input_len + 1] = '\0';
        input_len++;
    }
    
    filter_items();
    draw();
}

void execute_command() {
    char *command_to_run = input_buffer;
    if (input_len == 0 && num_matches > 0) {
        command_to_run = matches[0];
    }
    
    if (strlen(command_to_run) == 0) return;

    // --- The Double Fork ---
    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        perror("fork");
        return;
    }

    if (pid == 0) {
        // --- We are Child 1 ---
        // Fork again to create the grandchild
        pid_t grandchild_pid = fork();
        
        if (grandchild_pid < 0) {
            perror("fork (grandchild)");
            exit(1); // Exit child 1 on error
        }

        if (grandchild_pid == 0) {
            // --- We are Child 2 (the Grandchild) ---
            // This is the process that will actually run the command.
            
            // Detach from the controlling terminal
            if (setsid() < 0) {
                perror("setsid");
                exit(1);
            }
            
            // The parent qlaunch will clean up the main display connection.
            // We can safely close our copy of it here.
            XCloseDisplay(dpy);
            
            // Execute the command via the shell
            execlp("/bin/sh", "sh", "-c", command_to_run, NULL);
            
            // If execlp returns, an error occurred
            perror("execlp");
            exit(1); // Exit grandchild
        }
        
        // --- We are still Child 1 ---
        // Our only job is to exit immediately, orphaning the grandchild.
        exit(0);
    }
    
    // --- We are the Original Parent (qlaunch) ---
    // Wait for Child 1 to exit. This happens almost instantly.
    // This prevents Child 1 from becoming a zombie.
    int status;
    waitpid(pid, &status, 0);
}

void cleanup() {
    for (int i = 0; i < num_items; i++) {
        free(items[i]);
    }
    free(matches);
    
    if (font) XFreeFont(dpy, font);
    if (dpy) {
        XFreeGC(dpy, gc);
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);
    }
}

void run() {
    XEvent ev;
    while (running) {
        XNextEvent(dpy, &ev);
        if (ev.type == Expose) {
            draw();
        } else if (ev.type == KeyPress) {
            handle_keypress(&ev.xkey);
        }
    }
}

int main(void) {
    setup_x();
    grab_keyboard();
    populate_items();
    draw();
    run();
    cleanup();
    return 0;
}

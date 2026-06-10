 /*
 * main.c - ncurses-based 2D Graphics Editor
 *
 * Menu-driven editor that lets you add, delete, and modify
 * geometric shapes (circle, rectangle, line, triangle) on a
 * character-based canvas.
 *
 * Build:
 *   Linux/macOS:  gcc -o editor main.c shapes.c -lncurses -lm
 *   Windows MSYS2: gcc -o editor main.c shapes.c -lncurses -lm
 *   Windows PDCurses: gcc -o editor main.c shapes.c -lpdcurses -lm
 */

#ifdef _WIN32
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "shapes.h"

/* ─── Color Pair IDs ─── */
#define CP_TITLE     1
#define CP_MENU      2
#define CP_MENU_HL   3
#define CP_CANVAS_BG 4
#define CP_DRAW      5
#define CP_STATUS    6
#define CP_BORDER    7
#define CP_INPUT     8
#define CP_SHAPE_LIST 9
#define CP_HEADER    10

/* ─── Layout Constants ─── */
#define MENU_WIDTH   32
#define STATUS_HEIGHT 3

/* ─── Global Editor ─── */
static Editor g_editor;



/* ═══════════════════════════════════════════════════════════════════
 *  Draw the Title Bar
 * ═══════════════════════════════════════════════════════════════════ */

static void draw_title_bar(void)
{
    attron(COLOR_PAIR(CP_TITLE) | A_BOLD);
    for (int x = 0; x < COLS; x++)
        mvaddch(0, x, ' ');
    mvprintw(0, (COLS - 26) / 2, " 2D GRAPHICS EDITOR [C] ");
    attroff(COLOR_PAIR(CP_TITLE) | A_BOLD);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Draw the Status Bar
 * ═══════════════════════════════════════════════════════════════════ */

static void draw_status_bar(const char *msg)
{
    attron(COLOR_PAIR(CP_STATUS));
    for (int x = 0; x < COLS; x++)
        mvaddch(LINES - 1, x, ' ');
    mvprintw(LINES - 1, 1, " %s", msg);
    char info[64];
    snprintf(info, sizeof(info), "Canvas: %dx%d | Shapes: %d/%d ",
             CANVAS_WIDTH, CANVAS_HEIGHT,
             g_editor.shape_count, MAX_SHAPES);
    mvprintw(LINES - 1, COLS - (int)strlen(info) - 1, "%s", info);
    attroff(COLOR_PAIR(CP_STATUS));
}

/* ═══════════════════════════════════════════════════════════════════
 *  Display the Canvas
 * ═══════════════════════════════════════════════════════════════════ */

static void display_canvas(WINDOW *win)
{
    int win_h, win_w;
    getmaxyx(win, win_h, win_w);

    /* Available drawing area inside the border */
    int area_w = win_w - 2;
    int area_h = win_h - 2;

    werase(win);
    wattron(win, COLOR_PAIR(CP_BORDER));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(CP_BORDER));

    /* Title */
    wattron(win, COLOR_PAIR(CP_HEADER) | A_BOLD);
    mvwprintw(win, 0, 2, " Canvas ");
    wattroff(win, COLOR_PAIR(CP_HEADER) | A_BOLD);

    for (int y = 0; y < CANVAS_HEIGHT && y < area_h; y++) {
        for (int x = 0; x < CANVAS_WIDTH && x < area_w; x++) {
            char ch = g_editor.canvas.grid[y][x];
            if (ch == DRAW_CHAR) {
                wattron(win, COLOR_PAIR(CP_DRAW) | A_BOLD);
                mvwaddch(win, y + 1, x + 1, ch);
                wattroff(win, COLOR_PAIR(CP_DRAW) | A_BOLD);
            } else {
                wattron(win, COLOR_PAIR(CP_CANVAS_BG));
                mvwaddch(win, y + 1, x + 1, ch);
                wattroff(win, COLOR_PAIR(CP_CANVAS_BG));
            }
        }
    }
    wrefresh(win);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Input Dialog  – prompts for an integer value
 * ═══════════════════════════════════════════════════════════════════ */

static int input_int(const char *prompt, int *result)
{
    int dlg_w = 50;
    int dlg_h = 5;
    int start_y = (LINES - dlg_h) / 2;
    int start_x = (COLS  - dlg_w) / 2;

    WINDOW *dlg = newwin(dlg_h, dlg_w, start_y, start_x);
    wattron(dlg, COLOR_PAIR(CP_INPUT));
    werase(dlg);
    box(dlg, 0, 0);

    wattron(dlg, A_BOLD);
    mvwprintw(dlg, 0, 2, " Input ");
    wattroff(dlg, A_BOLD);

    mvwprintw(dlg, 1, 2, "%s", prompt);
    mvwprintw(dlg, 3, 2, "[Enter value, ESC to cancel]");
    wrefresh(dlg);

    echo();
    curs_set(1);

    char buf[32] = {0};
    int ch;
    int pos = 0;
    wmove(dlg, 2, 2);
    wattron(dlg, A_UNDERLINE);
    for (int i = 0; i < 20; i++) mvwaddch(dlg, 2, 2 + i, ' ');
    wattroff(dlg, A_UNDERLINE);
    wmove(dlg, 2, 2);
    wrefresh(dlg);

    noecho();
    while (1) {
        ch = wgetch(dlg);
        if (ch == 27) { /* ESC */
            delwin(dlg);
            curs_set(0);
            return -1;
        }
        if (ch == '\n' || ch == '\r') {
            break;
        }
        if ((ch == KEY_BACKSPACE || ch == 127 || ch == 8) && pos > 0) {
            pos--;
            buf[pos] = '\0';
            mvwaddch(dlg, 2, 2 + pos, ' ');
            wmove(dlg, 2, 2 + pos);
            wrefresh(dlg);
        } else if ((isdigit(ch) || (ch == '-' && pos == 0)) && pos < 20) {
            buf[pos++] = ch;
            buf[pos]   = '\0';
            mvwaddch(dlg, 2, 2 + pos - 1, ch);
            wrefresh(dlg);
        }
    }

    curs_set(0);
    wattroff(dlg, COLOR_PAIR(CP_INPUT));
    delwin(dlg);

    if (pos == 0) return -1;
    *result = atoi(buf);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════
 *  Message Box
 * ═══════════════════════════════════════════════════════════════════ */

static void show_message(const char *title, const char *msg)
{
    int dlg_w = 54;
    int dlg_h = 5;
    int start_y = (LINES - dlg_h) / 2;
    int start_x = (COLS  - dlg_w) / 2;

    WINDOW *dlg = newwin(dlg_h, dlg_w, start_y, start_x);
    wattron(dlg, COLOR_PAIR(CP_INPUT));
    werase(dlg);
    box(dlg, 0, 0);

    wattron(dlg, A_BOLD);
    mvwprintw(dlg, 0, 2, " %s ", title);
    wattroff(dlg, A_BOLD);

    mvwprintw(dlg, 2, 2, "%s", msg);
    mvwprintw(dlg, 3, 2, "Press any key to continue...");
    wattroff(dlg, COLOR_PAIR(CP_INPUT));
    wrefresh(dlg);

    wgetch(dlg);
    delwin(dlg);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Shape Selection Dialog (for delete/modify)
 * ═══════════════════════════════════════════════════════════════════ */

static int select_shape_dialog(const char *title)
{
    if (g_editor.shape_count == 0) {
        show_message("Info", "No shapes on the canvas.");
        return -1;
    }

    int dlg_w = 56;
    int dlg_h = g_editor.shape_count + 6;
    if (dlg_h > LINES - 4) dlg_h = LINES - 4;
    int visible = dlg_h - 6;
    int start_y = (LINES - dlg_h) / 2;
    int start_x = (COLS  - dlg_w) / 2;

    WINDOW *dlg = newwin(dlg_h, dlg_w, start_y, start_x);
    keypad(dlg, TRUE);

    int highlight = 0;
    int scroll_off = 0;

    while (1) {
        werase(dlg);
        wattron(dlg, COLOR_PAIR(CP_INPUT));
        box(dlg, 0, 0);

        wattron(dlg, A_BOLD);
        mvwprintw(dlg, 0, 2, " %s ", title);
        wattroff(dlg, A_BOLD);

        mvwprintw(dlg, 1, 2, "Use UP/DOWN to select, ENTER to confirm, ESC cancel");
        mvwprintw(dlg, 2, 2, "%-52s", "----------------------------------------------------");

        for (int i = 0; i < visible && (i + scroll_off) < g_editor.shape_count; i++) {
            char desc[128];
            shape_description(&g_editor.shapes[i + scroll_off], desc, sizeof(desc));
            if (i + scroll_off == highlight) {
                wattron(dlg, COLOR_PAIR(CP_MENU_HL) | A_BOLD);
                mvwprintw(dlg, 3 + i, 2, " > %-49s", desc);
                wattroff(dlg, COLOR_PAIR(CP_MENU_HL) | A_BOLD);
            } else {
                mvwprintw(dlg, 3 + i, 2, "   %-49s", desc);
            }
        }

        /* Scroll indicators */
        if (scroll_off > 0)
            mvwprintw(dlg, 3, dlg_w - 4, "^");
        if (scroll_off + visible < g_editor.shape_count)
            mvwprintw(dlg, 3 + visible - 1, dlg_w - 4, "v");

        wattroff(dlg, COLOR_PAIR(CP_INPUT));
        wrefresh(dlg);

        int ch = wgetch(dlg);
        switch (ch) {
        case KEY_UP:
            if (highlight > 0) {
                highlight--;
                if (highlight < scroll_off) scroll_off = highlight;
            }
            break;
        case KEY_DOWN:
            if (highlight < g_editor.shape_count - 1) {
                highlight++;
                if (highlight >= scroll_off + visible) scroll_off = highlight - visible + 1;
            }
            break;
        case '\n': case '\r':
            delwin(dlg);
            return g_editor.shapes[highlight].id;
        case 27: /* ESC */
            delwin(dlg);
            return -1;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Add Shape Dialogs
 * ═══════════════════════════════════════════════════════════════════ */

static void add_circle(void)
{
    Shape s;
    s.type = SHAPE_CIRCLE;
    if (input_int("Center X (0-%d):", &s.params.circle.cx) < 0) return;
    if (input_int("Center Y (0-%d):", &s.params.circle.cy) < 0) return;
    if (input_int("Radius:",          &s.params.circle.radius) < 0) return;
    if (s.params.circle.radius < 0) {
        show_message("Error", "Radius must be non-negative.");
        return;
    }
    int id = editor_add_shape(&g_editor, s);
    if (id > 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Circle added with ID %d.", id);
        show_message("Success", msg);
    } else {
        show_message("Error", "Shape limit reached!");
    }
}

static void add_rectangle(void)
{
    Shape s;
    s.type = SHAPE_RECTANGLE;
    if (input_int("Top-Left X:", &s.params.rect.x)      < 0) return;
    if (input_int("Top-Left Y:", &s.params.rect.y)      < 0) return;
    if (input_int("Width:",      &s.params.rect.width)   < 0) return;
    if (input_int("Height:",     &s.params.rect.height)  < 0) return;
    if (s.params.rect.width <= 0 || s.params.rect.height <= 0) {
        show_message("Error", "Width and Height must be positive.");
        return;
    }
    int id = editor_add_shape(&g_editor, s);
    if (id > 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Rectangle added with ID %d.", id);
        show_message("Success", msg);
    } else {
        show_message("Error", "Shape limit reached!");
    }
}

static void add_line(void)
{
    Shape s;
    s.type = SHAPE_LINE;
    if (input_int("Start X:", &s.params.line.x1) < 0) return;
    if (input_int("Start Y:", &s.params.line.y1) < 0) return;
    if (input_int("End X:",   &s.params.line.x2) < 0) return;
    if (input_int("End Y:",   &s.params.line.y2) < 0) return;
    int id = editor_add_shape(&g_editor, s);
    if (id > 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Line added with ID %d.", id);
        show_message("Success", msg);
    } else {
        show_message("Error", "Shape limit reached!");
    }
}

static void add_triangle(void)
{
    Shape s;
    s.type = SHAPE_TRIANGLE;
    if (input_int("Vertex 1 X:", &s.params.triangle.x1) < 0) return;
    if (input_int("Vertex 1 Y:", &s.params.triangle.y1) < 0) return;
    if (input_int("Vertex 2 X:", &s.params.triangle.x2) < 0) return;
    if (input_int("Vertex 2 Y:", &s.params.triangle.y2) < 0) return;
    if (input_int("Vertex 3 X:", &s.params.triangle.x3) < 0) return;
    if (input_int("Vertex 3 Y:", &s.params.triangle.y3) < 0) return;
    int id = editor_add_shape(&g_editor, s);
    if (id > 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Triangle added with ID %d.", id);
        show_message("Success", msg);
    } else {
        show_message("Error", "Shape limit reached!");
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Add Shape Sub-Menu
 * ═══════════════════════════════════════════════════════════════════ */

static void menu_add_shape(void)
{
    const char *items[] = {
        " Circle    ",
        " Rectangle ",
        " Line      ",
        " Triangle  ",
        " Back      "
    };
    int n = 5;

    int dlg_w = 30;
    int dlg_h = n + 4;
    int start_y = (LINES - dlg_h) / 2;
    int start_x = (COLS  - dlg_w) / 2;

    WINDOW *dlg = newwin(dlg_h, dlg_w, start_y, start_x);
    keypad(dlg, TRUE);
    int highlight = 0;

    while (1) {
        werase(dlg);
        wattron(dlg, COLOR_PAIR(CP_INPUT));
        box(dlg, 0, 0);
        wattron(dlg, A_BOLD);
        mvwprintw(dlg, 0, 2, " Add Shape ");
        wattroff(dlg, A_BOLD);

        mvwprintw(dlg, 1, 2, "Select shape type:");

        for (int i = 0; i < n; i++) {
            if (i == highlight) {
                wattron(dlg, COLOR_PAIR(CP_MENU_HL) | A_BOLD);
                mvwprintw(dlg, 3 + i, 3, "> %s", items[i]);
                wattroff(dlg, COLOR_PAIR(CP_MENU_HL) | A_BOLD);
            } else {
                mvwprintw(dlg, 3 + i, 3, "  %s", items[i]);
            }
        }
        wattroff(dlg, COLOR_PAIR(CP_INPUT));
        wrefresh(dlg);

        int ch = wgetch(dlg);
        switch (ch) {
        case KEY_UP:
            highlight = (highlight > 0) ? highlight - 1 : n - 1;
            break;
        case KEY_DOWN:
            highlight = (highlight < n - 1) ? highlight + 1 : 0;
            break;
        case '\n': case '\r':
            delwin(dlg);
            switch (highlight) {
            case 0: add_circle();    return;
            case 1: add_rectangle(); return;
            case 2: add_line();      return;
            case 3: add_triangle();  return;
            case 4: return;
            }
            return;
        case 27:
            delwin(dlg);
            return;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Delete Shape
 * ═══════════════════════════════════════════════════════════════════ */

static void menu_delete_shape(void)
{
    int shape_id = select_shape_dialog("Delete Shape");
    if (shape_id < 0) return;

    if (editor_delete_shape(&g_editor, shape_id) == 0) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Shape ID %d deleted.", shape_id);
        show_message("Deleted", msg);
    } else {
        show_message("Error", "Could not delete shape.");
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Modify Shape
 * ═══════════════════════════════════════════════════════════════════ */

static void menu_modify_shape(void)
{
    int shape_id = select_shape_dialog("Modify Shape");
    if (shape_id < 0) return;

    Shape *s = editor_find_shape(&g_editor, shape_id);
    if (!s) {
        show_message("Error", "Shape not found.");
        return;
    }

    char desc[128];
    shape_description(s, desc, sizeof(desc));

    /* Show current values and allow re-entry */
    switch (s->type) {
    case SHAPE_CIRCLE: {
        int val;
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "Center X (cur=%d):", s->params.circle.cx);
        if (input_int(prompt, &val) == 0) s->params.circle.cx = val;
        snprintf(prompt, sizeof(prompt), "Center Y (cur=%d):", s->params.circle.cy);
        if (input_int(prompt, &val) == 0) s->params.circle.cy = val;
        snprintf(prompt, sizeof(prompt), "Radius   (cur=%d):", s->params.circle.radius);
        if (input_int(prompt, &val) == 0) {
            if (val >= 0) s->params.circle.radius = val;
        }
        break;
    }
    case SHAPE_RECTANGLE: {
        int val;
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "Top-Left X (cur=%d):", s->params.rect.x);
        if (input_int(prompt, &val) == 0) s->params.rect.x = val;
        snprintf(prompt, sizeof(prompt), "Top-Left Y (cur=%d):", s->params.rect.y);
        if (input_int(prompt, &val) == 0) s->params.rect.y = val;
        snprintf(prompt, sizeof(prompt), "Width      (cur=%d):", s->params.rect.width);
        if (input_int(prompt, &val) == 0 && val > 0) s->params.rect.width = val;
        snprintf(prompt, sizeof(prompt), "Height     (cur=%d):", s->params.rect.height);
        if (input_int(prompt, &val) == 0 && val > 0) s->params.rect.height = val;
        break;
    }
    case SHAPE_LINE: {
        int val;
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "Start X (cur=%d):", s->params.line.x1);
        if (input_int(prompt, &val) == 0) s->params.line.x1 = val;
        snprintf(prompt, sizeof(prompt), "Start Y (cur=%d):", s->params.line.y1);
        if (input_int(prompt, &val) == 0) s->params.line.y1 = val;
        snprintf(prompt, sizeof(prompt), "End X   (cur=%d):", s->params.line.x2);
        if (input_int(prompt, &val) == 0) s->params.line.x2 = val;
        snprintf(prompt, sizeof(prompt), "End Y   (cur=%d):", s->params.line.y2);
        if (input_int(prompt, &val) == 0) s->params.line.y2 = val;
        break;
    }
    case SHAPE_TRIANGLE: {
        int val;
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "Vertex 1 X (cur=%d):", s->params.triangle.x1);
        if (input_int(prompt, &val) == 0) s->params.triangle.x1 = val;
        snprintf(prompt, sizeof(prompt), "Vertex 1 Y (cur=%d):", s->params.triangle.y1);
        if (input_int(prompt, &val) == 0) s->params.triangle.y1 = val;
        snprintf(prompt, sizeof(prompt), "Vertex 2 X (cur=%d):", s->params.triangle.x2);
        if (input_int(prompt, &val) == 0) s->params.triangle.x2 = val;
        snprintf(prompt, sizeof(prompt), "Vertex 2 Y (cur=%d):", s->params.triangle.y2);
        if (input_int(prompt, &val) == 0) s->params.triangle.y2 = val;
        snprintf(prompt, sizeof(prompt), "Vertex 3 X (cur=%d):", s->params.triangle.x3);
        if (input_int(prompt, &val) == 0) s->params.triangle.x3 = val;
        snprintf(prompt, sizeof(prompt), "Vertex 3 Y (cur=%d):", s->params.triangle.y3);
        if (input_int(prompt, &val) == 0) s->params.triangle.y3 = val;
        break;
    }
    }

    editor_rebuild_canvas(&g_editor);
    show_message("Modified", "Shape updated successfully.");
}

/* ═══════════════════════════════════════════════════════════════════
 *  List All Shapes
 * ═══════════════════════════════════════════════════════════════════ */

static void menu_list_shapes(void)
{
    if (g_editor.shape_count == 0) {
        show_message("Shape List", "No shapes on the canvas.");
        return;
    }

    int dlg_w = 56;
    int dlg_h = g_editor.shape_count + 5;
    if (dlg_h > LINES - 4) dlg_h = LINES - 4;
    int visible = dlg_h - 5;
    int start_y = (LINES - dlg_h) / 2;
    int start_x = (COLS  - dlg_w) / 2;

    WINDOW *dlg = newwin(dlg_h, dlg_w, start_y, start_x);
    keypad(dlg, TRUE);
    int scroll_off = 0;

    while (1) {
        werase(dlg);
        wattron(dlg, COLOR_PAIR(CP_SHAPE_LIST));
        box(dlg, 0, 0);
        wattron(dlg, A_BOLD);
        mvwprintw(dlg, 0, 2, " Shape List (%d shapes) ", g_editor.shape_count);
        wattroff(dlg, A_BOLD);

        mvwprintw(dlg, 1, 2, "%-52s", "----------------------------------------------------");

        for (int i = 0; i < visible && (i + scroll_off) < g_editor.shape_count; i++) {
            char desc[128];
            shape_description(&g_editor.shapes[i + scroll_off], desc, sizeof(desc));
            mvwprintw(dlg, 2 + i, 2, " %-50s", desc);
        }

        mvwprintw(dlg, dlg_h - 2, 2, "UP/DOWN to scroll, any other key to close");

        if (scroll_off > 0)
            mvwprintw(dlg, 2, dlg_w - 4, "^");
        if (scroll_off + visible < g_editor.shape_count)
            mvwprintw(dlg, 2 + visible - 1, dlg_w - 4, "v");

        wattroff(dlg, COLOR_PAIR(CP_SHAPE_LIST));
        wrefresh(dlg);

        int ch = wgetch(dlg);
        if (ch == KEY_UP && scroll_off > 0) {
            scroll_off--;
        } else if (ch == KEY_DOWN && scroll_off + visible < g_editor.shape_count) {
            scroll_off++;
        } else if (ch != KEY_UP && ch != KEY_DOWN) {
            break;
        }
    }
    delwin(dlg);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Clear Canvas (remove all shapes)
 * ═══════════════════════════════════════════════════════════════════ */

static void menu_clear_canvas(void)
{
    if (g_editor.shape_count == 0) {
        show_message("Info", "Canvas is already empty.");
        return;
    }

    /* Confirmation dialog */
    int dlg_w = 44;
    int dlg_h = 5;
    int start_y = (LINES - dlg_h) / 2;
    int start_x = (COLS  - dlg_w) / 2;

    WINDOW *dlg = newwin(dlg_h, dlg_w, start_y, start_x);
    wattron(dlg, COLOR_PAIR(CP_INPUT));
    werase(dlg);
    box(dlg, 0, 0);
    wattron(dlg, A_BOLD);
    mvwprintw(dlg, 0, 2, " Confirm Clear ");
    wattroff(dlg, A_BOLD);
    mvwprintw(dlg, 2, 2, "Clear all %d shapes? (y/n)", g_editor.shape_count);
    wattroff(dlg, COLOR_PAIR(CP_INPUT));
    wrefresh(dlg);

    int ch = wgetch(dlg);
    delwin(dlg);

    if (ch == 'y' || ch == 'Y') {
        editor_init(&g_editor);
        show_message("Cleared", "All shapes removed.");
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Main Menu
 * ═══════════════════════════════════════════════════════════════════ */

static const char *main_menu_items[] = {
    "  Add Shape       ",
    "  Delete Shape    ",
    "  Modify Shape    ",
    "  List Shapes     ",
    "  Clear Canvas    ",
    "  Refresh Display ",
    "  Exit            "
};
static const int main_menu_count = 7;

static void draw_menu(WINDOW *win, int highlight)
{
    int win_h, win_w;
    getmaxyx(win, win_h, win_w);
    (void)win_h;

    werase(win);
    wattron(win, COLOR_PAIR(CP_BORDER));
    box(win, 0, 0);
    wattroff(win, COLOR_PAIR(CP_BORDER));

    wattron(win, COLOR_PAIR(CP_HEADER) | A_BOLD);
    mvwprintw(win, 0, 2, " Main Menu ");
    wattroff(win, COLOR_PAIR(CP_HEADER) | A_BOLD);

    /* Draw decorative line */
    wattron(win, COLOR_PAIR(CP_MENU));
    mvwprintw(win, 1, 1, "%-*s", win_w - 2, "");
    for (int x = 1; x < win_w - 1; x++)
        mvwaddch(win, 1, x, ACS_HLINE);
    wattroff(win, COLOR_PAIR(CP_MENU));

    for (int i = 0; i < main_menu_count; i++) {
        if (i == highlight) {
            wattron(win, COLOR_PAIR(CP_MENU_HL) | A_BOLD);
            mvwprintw(win, 3 + i * 2, 1, " > %s", main_menu_items[i]);
            wattroff(win, COLOR_PAIR(CP_MENU_HL) | A_BOLD);
        } else {
            wattron(win, COLOR_PAIR(CP_MENU));
            mvwprintw(win, 3 + i * 2, 1, "   %s", main_menu_items[i]);
            wattroff(win, COLOR_PAIR(CP_MENU));
        }
    }

    /* Key hints at bottom */
    wattron(win, COLOR_PAIR(CP_STATUS));
    mvwprintw(win, win_h - 3, 2, "UP/DOWN: Navigate");
    mvwprintw(win, win_h - 2, 2, "ENTER:   Select");
    wattroff(win, COLOR_PAIR(CP_STATUS));

    wrefresh(win);
}

/* ═══════════════════════════════════════════════════════════════════
 *  MAIN
 * ═══════════════════════════════════════════════════════════════════ */

int main(void)
{
    /* ── Initialize ncurses ── */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    /* ── Initialize colors ── */
    if (has_colors()) {
        start_color();
        use_default_colors();

        init_pair(CP_TITLE,      COLOR_WHITE,  COLOR_BLUE);
        init_pair(CP_MENU,       COLOR_WHITE,  -1);
        init_pair(CP_MENU_HL,    COLOR_BLACK,  COLOR_CYAN);
        init_pair(CP_CANVAS_BG,  COLOR_WHITE,  -1);
        init_pair(CP_DRAW,       COLOR_YELLOW, -1);
        init_pair(CP_STATUS,     COLOR_WHITE,  COLOR_BLUE);
        init_pair(CP_BORDER,     COLOR_CYAN,   -1);
        init_pair(CP_INPUT,      COLOR_WHITE,  COLOR_BLUE);
        init_pair(CP_SHAPE_LIST, COLOR_WHITE,  COLOR_BLUE);
        init_pair(CP_HEADER,     COLOR_CYAN,   -1);
    }

    /* ── Check minimum terminal size ── */
    if (LINES < 30 || COLS < 80) {
        endwin();
        fprintf(stderr, "Terminal too small. Need at least 80x30, got %dx%d.\n",
                COLS, LINES);
        return 1;
    }

    /* ── Initialize editor ── */
    editor_init(&g_editor);

    /* ── Create windows ── */
    int canvas_w = COLS - MENU_WIDTH;
    int canvas_h = LINES - 2;  /* minus title + status bars */

    WINDOW *menu_win   = newwin(canvas_h, MENU_WIDTH, 1, 0);
    WINDOW *canvas_win = newwin(canvas_h, canvas_w, 1, MENU_WIDTH);
    keypad(menu_win, TRUE);

    int highlight = 0;
    int running   = 1;

    while (running) {
        /* Draw the UI */
        draw_title_bar();
        draw_status_bar("Ready | Use arrow keys to navigate, ENTER to select");
        draw_menu(menu_win, highlight);
        display_canvas(canvas_win);
        refresh();

        int ch = wgetch(menu_win);
        switch (ch) {
        case KEY_UP:
            highlight = (highlight > 0) ? highlight - 1 : main_menu_count - 1;
            break;
        case KEY_DOWN:
            highlight = (highlight < main_menu_count - 1) ? highlight + 1 : 0;
            break;
        case '\n':
        case '\r':
            switch (highlight) {
            case 0: menu_add_shape();    break;
            case 1: menu_delete_shape(); break;
            case 2: menu_modify_shape(); break;
            case 3: menu_list_shapes();  break;
            case 4: menu_clear_canvas(); break;
            case 5: /* refresh */        break;
            case 6: running = 0;         break;
            }
            /* Rebuild windows in case terminal was resized */
            getmaxyx(stdscr, canvas_h, canvas_w);
            canvas_h -= 2;
            canvas_w -= MENU_WIDTH;
            wresize(menu_win,   canvas_h, MENU_WIDTH);
            wresize(canvas_win, canvas_h, canvas_w);
            mvwin(canvas_win, 1, MENU_WIDTH);
            break;
        case 'q':
        case 'Q':
            running = 0;
            break;
        }
    }

    /* ── Cleanup ── */
    delwin(menu_win);
    delwin(canvas_win);
    endwin();

    printf("2D Graphics Editor terminated.\n");
    return 0;
}

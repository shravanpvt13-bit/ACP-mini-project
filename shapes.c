/*
 * shapes.c - Shape drawing algorithms and editor logic
 * 2D Graphics Editor
 *
 * Drawing algorithms:
 *   - Line:      Bresenham's line algorithm
 *   - Circle:    Midpoint circle algorithm
 *   - Rectangle: Four lines
 *   - Triangle:  Three lines connecting vertices
 */

#include "shapes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════════
 *  Canvas Operations
 * ═══════════════════════════════════════════════════════════════════ */

void canvas_init(Canvas *c)
{
    for (int y = 0; y < CANVAS_HEIGHT; y++)
        for (int x = 0; x < CANVAS_WIDTH; x++)
            c->grid[y][x] = FILL_CHAR;
}

void canvas_set_pixel(Canvas *c, int x, int y, char ch)
{
    if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT)
        c->grid[y][x] = ch;
}

char canvas_get_pixel(Canvas *c, int x, int y)
{
    if (x >= 0 && x < CANVAS_WIDTH && y >= 0 && y < CANVAS_HEIGHT)
        return c->grid[y][x];
    return '\0';
}

/* ═══════════════════════════════════════════════════════════════════
 *  Bresenham's Line Algorithm
 * ═══════════════════════════════════════════════════════════════════ */

void draw_line_on_canvas(Canvas *c, int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx + dy;
    int e2;

    for (;;) {
        canvas_set_pixel(c, x1, y1, DRAW_CHAR);
        if (x1 == x2 && y1 == y2)
            break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1  += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1  += sy;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Midpoint Circle Algorithm
 * ═══════════════════════════════════════════════════════════════════ */

void draw_circle_on_canvas(Canvas *c, int cx, int cy, int radius)
{
    if (radius <= 0) {
        canvas_set_pixel(c, cx, cy, DRAW_CHAR);
        return;
    }

    int x = radius;
    int y = 0;
    int err = 1 - radius;

    while (x >= y) {
        /* Plot all 8 octants */
        canvas_set_pixel(c, cx + x, cy + y, DRAW_CHAR);
        canvas_set_pixel(c, cx - x, cy + y, DRAW_CHAR);
        canvas_set_pixel(c, cx + x, cy - y, DRAW_CHAR);
        canvas_set_pixel(c, cx - x, cy - y, DRAW_CHAR);
        canvas_set_pixel(c, cx + y, cy + x, DRAW_CHAR);
        canvas_set_pixel(c, cx - y, cy + x, DRAW_CHAR);
        canvas_set_pixel(c, cx + y, cy - x, DRAW_CHAR);
        canvas_set_pixel(c, cx - y, cy - x, DRAW_CHAR);

        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Rectangle Drawing (four lines)
 * ═══════════════════════════════════════════════════════════════════ */

void draw_rect_on_canvas(Canvas *c, int x, int y, int w, int h)
{
    /* Top edge */
    draw_line_on_canvas(c, x, y, x + w - 1, y);
    /* Bottom edge */
    draw_line_on_canvas(c, x, y + h - 1, x + w - 1, y + h - 1);
    /* Left edge */
    draw_line_on_canvas(c, x, y, x, y + h - 1);
    /* Right edge */
    draw_line_on_canvas(c, x + w - 1, y, x + w - 1, y + h - 1);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Triangle Drawing (three lines)
 * ═══════════════════════════════════════════════════════════════════ */

void draw_triangle_on_canvas(Canvas *c, int x1, int y1,
                             int x2, int y2, int x3, int y3)
{
    draw_line_on_canvas(c, x1, y1, x2, y2);
    draw_line_on_canvas(c, x2, y2, x3, y3);
    draw_line_on_canvas(c, x3, y3, x1, y1);
}

/* ═══════════════════════════════════════════════════════════════════
 *  Shape Helper Functions
 * ═══════════════════════════════════════════════════════════════════ */

void draw_shape(Canvas *c, const Shape *s)
{
    switch (s->type) {
    case SHAPE_CIRCLE:
        draw_circle_on_canvas(c, s->params.circle.cx,
                              s->params.circle.cy,
                              s->params.circle.radius);
        break;
    case SHAPE_RECTANGLE:
        draw_rect_on_canvas(c, s->params.rect.x,
                            s->params.rect.y,
                            s->params.rect.width,
                            s->params.rect.height);
        break;
    case SHAPE_LINE:
        draw_line_on_canvas(c, s->params.line.x1,
                            s->params.line.y1,
                            s->params.line.x2,
                            s->params.line.y2);
        break;
    case SHAPE_TRIANGLE:
        draw_triangle_on_canvas(c, s->params.triangle.x1,
                                s->params.triangle.y1,
                                s->params.triangle.x2,
                                s->params.triangle.y2,
                                s->params.triangle.x3,
                                s->params.triangle.y3);
        break;
    }
}

const char *shape_type_name(ShapeType type)
{
    switch (type) {
    case SHAPE_CIRCLE:    return "Circle";
    case SHAPE_RECTANGLE: return "Rectangle";
    case SHAPE_LINE:      return "Line";
    case SHAPE_TRIANGLE:  return "Triangle";
    default:              return "Unknown";
    }
}

void shape_description(const Shape *s, char *buf, int bufsize)
{
    switch (s->type) {
    case SHAPE_CIRCLE:
        snprintf(buf, bufsize, "[ID %d] Circle: center=(%d,%d) r=%d",
                 s->id, s->params.circle.cx, s->params.circle.cy,
                 s->params.circle.radius);
        break;
    case SHAPE_RECTANGLE:
        snprintf(buf, bufsize, "[ID %d] Rect: pos=(%d,%d) %dx%d",
                 s->id, s->params.rect.x, s->params.rect.y,
                 s->params.rect.width, s->params.rect.height);
        break;
    case SHAPE_LINE:
        snprintf(buf, bufsize, "[ID %d] Line: (%d,%d)->(%d,%d)",
                 s->id, s->params.line.x1, s->params.line.y1,
                 s->params.line.x2, s->params.line.y2);
        break;
    case SHAPE_TRIANGLE:
        snprintf(buf, bufsize, "[ID %d] Tri: (%d,%d),(%d,%d),(%d,%d)",
                 s->id, s->params.triangle.x1, s->params.triangle.y1,
                 s->params.triangle.x2, s->params.triangle.y2,
                 s->params.triangle.x3, s->params.triangle.y3);
        break;
    default:
        snprintf(buf, bufsize, "[ID %d] Unknown shape", s->id);
        break;
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  Editor Operations
 * ═══════════════════════════════════════════════════════════════════ */

void editor_init(Editor *e)
{
    canvas_init(&e->canvas);
    e->shape_count = 0;
    e->next_id     = 1;
}

int editor_add_shape(Editor *e, Shape s)
{
    if (e->shape_count >= MAX_SHAPES)
        return -1;  /* no room */

    s.id = e->next_id++;
    e->shapes[e->shape_count++] = s;

    /* Rebuild the entire canvas to ensure consistency */
    editor_rebuild_canvas(e);
    return s.id;
}

int editor_delete_shape(Editor *e, int shape_id)
{
    for (int i = 0; i < e->shape_count; i++) {
        if (e->shapes[i].id == shape_id) {
            /* Shift remaining shapes down */
            for (int j = i; j < e->shape_count - 1; j++)
                e->shapes[j] = e->shapes[j + 1];
            e->shape_count--;
            editor_rebuild_canvas(e);
            return 0;  /* success */
        }
    }
    return -1;  /* not found */
}

Shape *editor_find_shape(Editor *e, int shape_id)
{
    for (int i = 0; i < e->shape_count; i++) {
        if (e->shapes[i].id == shape_id)
            return &e->shapes[i];
    }
    return NULL;
}

void editor_rebuild_canvas(Editor *e)
{
    canvas_init(&e->canvas);
    for (int i = 0; i < e->shape_count; i++)
        draw_shape(&e->canvas, &e->shapes[i]);
}

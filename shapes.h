/*
 * shapes.h - Shape definitions and drawing algorithms
 * 2D Graphics Editor
 */

#ifndef SHAPES_H
#define SHAPES_H

/* ─── Canvas Dimensions ─── */
#define CANVAS_WIDTH  120
#define CANVAS_HEIGHT  40
#define MAX_SHAPES     64
#define FILL_CHAR     '_'
#define DRAW_CHAR     '*'

/* ─── Shape Type Enumeration ─── */
typedef enum {
    SHAPE_CIRCLE,
    SHAPE_RECTANGLE,
    SHAPE_LINE,
    SHAPE_TRIANGLE
} ShapeType;

/* ─── Shape Parameter Structures ─── */
typedef struct {
    int cx, cy, radius;
} CircleParams;

typedef struct {
    int x, y, width, height;
} RectParams;

typedef struct {
    int x1, y1, x2, y2;
} LineParams;

typedef struct {
    int x1, y1;
    int x2, y2;
    int x3, y3;
} TriangleParams;

/* ─── Shape Union ─── */
typedef struct {
    ShapeType type;
    int       id;       /* unique identifier for this shape */
    union {
        CircleParams   circle;
        RectParams     rect;
        LineParams     line;
        TriangleParams triangle;
    } params;
} Shape;

/* ─── Canvas ─── */
typedef struct {
    char grid[CANVAS_HEIGHT][CANVAS_WIDTH];
} Canvas;

/* ─── Editor State ─── */
typedef struct {
    Canvas canvas;
    Shape  shapes[MAX_SHAPES];
    int    shape_count;
    int    next_id;
} Editor;

/* ─── Function Prototypes ─── */

/* Canvas operations */
void canvas_init(Canvas *c);
void canvas_set_pixel(Canvas *c, int x, int y, char ch);
char canvas_get_pixel(Canvas *c, int x, int y);

/* Drawing primitives */
void draw_line_on_canvas(Canvas *c, int x1, int y1, int x2, int y2);
void draw_circle_on_canvas(Canvas *c, int cx, int cy, int radius);
void draw_rect_on_canvas(Canvas *c, int x, int y, int w, int h);
void draw_triangle_on_canvas(Canvas *c, int x1, int y1, int x2, int y2, int x3, int y3);

/* Shape operations */
void draw_shape(Canvas *c, const Shape *s);
const char *shape_type_name(ShapeType type);
void shape_description(const Shape *s, char *buf, int bufsize);

/* Editor operations */
void editor_init(Editor *e);
int  editor_add_shape(Editor *e, Shape s);
int  editor_delete_shape(Editor *e, int shape_id);
Shape *editor_find_shape(Editor *e, int shape_id);
void editor_rebuild_canvas(Editor *e);

#endif /* SHAPES_H */

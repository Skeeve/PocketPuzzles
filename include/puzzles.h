/*
 * puzzles.h: header file for my puzzle collection
 */

#ifndef PUZZLES_PUZZLES_H
#define PUZZLES_PUZZLES_H

#include <stdio.h>  /* for FILE */
#include <stdlib.h> /* for size_t */
#include <limits.h> /* for UINT_MAX */
#include <stdbool.h>

#define PI 3.141592653589793238462643383279502884197169399
#define ROOT2 1.414213562373095048801688724209698078569672

#define lenof(array) ( sizeof(array) / sizeof(*(array)) )

#define STR_INT(x) #x
#define STR(x) STR_INT(x)

/* An upper bound on the length of sprintf'ed integers (signed or unsigned). */
#define MAX_DIGITS(x) (sizeof(x) * CHAR_BIT / 3 + 2)

/* NB not perfect because they evaluate arguments multiple times. */
#ifndef max
#define max(x,y) ( (x)>(y) ? (x) : (y) )
#endif /* max */
#ifndef min
#define min(x,y) ( (x)<(y) ? (x) : (y) )
#endif /* min */

enum {
    LEFT_BUTTON = 0x0200,
    MIDDLE_BUTTON,
    RIGHT_BUTTON,
    LEFT_DRAG,
    MIDDLE_DRAG,
    RIGHT_DRAG,
    LEFT_RELEASE,
    MIDDLE_RELEASE,
    RIGHT_RELEASE,
    CURSOR_UP,
    CURSOR_DOWN,
    CURSOR_LEFT,
    CURSOR_RIGHT,
    CURSOR_SELECT,
    CURSOR_SELECT2,
    /* UI_* are special keystrokes generated by front ends in response
     * to menu actions, never passed to back ends */
    UI_LOWER_BOUND,
    UI_QUIT,
    UI_NEWGAME,
    UI_SOLVE,
    UI_UNDO,
    UI_REDO,
    UI_UPPER_BOUND,
    
    /* made smaller because of 'limited range of datatype' errors. */
    MOD_CTRL       = 0x1000,
    MOD_SHFT       = 0x2000,
    MOD_NUM_KEYPAD = 0x4000,
    MOD_MASK       = 0x7000 /* mask for all modifiers */
};

#define IS_MOUSE_DOWN(m) ( (unsigned)((m) - LEFT_BUTTON) <= \
                               (unsigned)(RIGHT_BUTTON - LEFT_BUTTON))
#define IS_MOUSE_DRAG(m) ( (unsigned)((m) - LEFT_DRAG) <= \
                               (unsigned)(RIGHT_DRAG - LEFT_DRAG))
#define IS_MOUSE_RELEASE(m) ( (unsigned)((m) - LEFT_RELEASE) <= \
                               (unsigned)(RIGHT_RELEASE - LEFT_RELEASE))
#define IS_CURSOR_MOVE(m) ( (m) == CURSOR_UP || (m) == CURSOR_DOWN || \
                            (m) == CURSOR_RIGHT || (m) == CURSOR_LEFT )
#define IS_CURSOR_SELECT(m) ( (m) == CURSOR_SELECT || (m) == CURSOR_SELECT2)
#define IS_UI_FAKE_KEY(m) ( (m) > UI_LOWER_BOUND && (m) < UI_UPPER_BOUND )

/*
 * Flags in the back end's `flags' word.
 */
/* Bit flags indicating mouse button priorities */
#define BUTTON_BEATS(x,y) ( 1 << (((x)-LEFT_BUTTON)*3+(y)-LEFT_BUTTON) )
/* Flag indicating that Solve operations should be animated */
#define SOLVE_ANIMATES ( 1 << 9 )
/* Pocket PC: Game requires right mouse button emulation */
#define REQUIRE_RBUTTON ( 1 << 10 )
/* Pocket PC: Game requires numeric input */
#define REQUIRE_NUMPAD ( 1 << 11 )
/* end of `flags' word definitions */

#define IGNOREARG(x) ( (x) = (x) )

typedef struct frontend frontend;
typedef struct config_item config_item;
typedef struct midend midend;
typedef struct random_state random_state;
typedef struct game_params game_params;
typedef struct game_state game_state;
typedef struct game_ui game_ui;
typedef struct game_drawstate game_drawstate;
typedef struct game game;
typedef struct blitter blitter;
typedef struct document document;
typedef struct drawing_api drawing_api;
typedef struct drawing drawing;
typedef struct psdata psdata;

#define ALIGN_VNORMAL 0x000
#define ALIGN_VCENTRE 0x100

#define ALIGN_HLEFT   0x000
#define ALIGN_HCENTRE 0x001
#define ALIGN_HRIGHT  0x002

#define FONT_FIXED    0
#define FONT_VARIABLE 1
#define FONT_FIXED_NORMAL 2
#define FONT_VARIABLE_NORMAL 3

/* For printing colours */
#define HATCH_SLASH     1
#define HATCH_BACKSLASH 2
#define HATCH_HORIZ     3
#define HATCH_VERT      4
#define HATCH_PLUS      5
#define HATCH_X         6

/*
 * Structure used to pass configuration data between frontend and
 * game
 */
enum { C_STRING, C_CHOICES, C_BOOLEAN, C_END };
struct config_item {
    /* Not dynamically allocated: the GUI display name for the option */
    const char *name;
    /* Not dynamically allocated: the keyword identifier for the
     * option. Only examined in the case where this structure is being
     * used for options that appear in config files, i.e. the
     * get_prefs method fills this in but configure does not. */
    const char *kw;
    /* Value from the above C_* enum */
    int type;
    union {
        struct { /* if type == C_STRING */
            /* Always dynamically allocated and non-NULL */
            char *sval;
        } string;
        struct { /* if type == C_CHOICES */
            /*
             * choicenames is non-NULL, not dynamically allocated, and
             * contains a set of option strings separated by a
             * delimiter. The delimiter is also the first character in
             * the string, so for example ":Foo:Bar:Baz" gives three
             * options `Foo', `Bar' and `Baz'.
             */
            const char *choicenames;
            /*
             * choicekws is non-NULL, not dynamically allocated, and
             * contains a parallel list of keyword strings used to
             * represent the enumeration in config files. As with 'kw'
             * above, this is only expected to be set by get_prefs.
             */
            const char *choicekws;
            /*
             * Indicates the chosen index from the options in
             * choicenames. In the above example, 0==Foo, 1==Bar and
             * 2==Baz.
             */
            int selected;
        } choices;
        struct {
            bool bval;
        } boolean;
    } u;
};

/*
 * Structure used to communicate the presets menu from midend to
 * frontend. In principle, it's also used to pass the same information
 * from game to midend, though games that don't specify a menu
 * hierarchy (i.e. most of them) will use the simpler fetch_preset()
 * function to return an unstructured list.
 *
 * A tree of these structures always belongs to the midend, and only
 * the midend should ever need to free it. The front end should treat
 * them as read-only.
 */
struct preset_menu_entry {
    char *title;
    /* Exactly one of the next two fields is NULL, depending on
     * whether this entry is a submenu title or an actual preset */
    game_params *params;
    struct preset_menu *submenu;
    /* Every preset menu entry has a number allocated by the mid-end,
     * so that midend_which_preset() can return a value that
     * identifies an entry anywhere in the menu hierarchy. The values
     * will be allocated reasonably densely from 1 upwards (so it's
     * reasonable for the front end to use them as array indices if it
     * needs to store GUI state per menu entry), but no other
     * guarantee is given about their ordering.
     *
     * Entries containing submenus have ids too - not only the actual
     * presets are numbered. */
    int id;
};
struct preset_menu {
    int n_entries;             /* number of entries actually in use */
    int entries_size;          /* space currently allocated in this array */
    struct preset_menu_entry *entries;
};
/* For games which do want to directly return a tree of these, here
 * are convenience routines (in midend.c) for constructing one. These
 * assume that 'title' and 'encoded_params' are already dynamically
 * allocated by the caller; the resulting preset_menu tree takes
 * ownership of them. */
struct preset_menu *preset_menu_new(void);
struct preset_menu *preset_menu_add_submenu(struct preset_menu *parent,
                                            char *title);
void preset_menu_add_preset(struct preset_menu *menu,
                            char *title, game_params *params);
/* Helper routine front ends can use for one of the ways they might
 * want to organise their preset menu usage */
game_params *preset_menu_lookup_by_id(struct preset_menu *menu, int id);

/*
 * Small structure specifying a UI button in a keyboardless front
 * end. The button will have the text of "label" written on it, and
 * pressing it causes the value "button" to be passed to
 * midend_process_key() as if typed at the keyboard.
 *
 * If `label' is NULL (which it likely will be), a generic label can
 * be generated with the button2label() function.
 */
typedef struct key_label {
    /* What should be displayed to the user by the frontend. Backends
     * can set this field to NULL and have it filled in by the midend
     * with an empty string.
     PocketPuzzles: label is ignored by frontend, so just don't bother.
      */
    const char *label;
    int button; /* passed to midend_process_key when button is pressed */
} key_label;

/*
 * Platform routines
 */

/* We can't use #ifdef DEBUG, because Cygwin defines it by default. */
#ifdef DEBUGGING
#define debug(x) (debug_printf x)
void debug_printf(const char *fmt, ...);
#else
#define debug(x)
#endif

void fatal(const char *fmt, ...);
void frontend_default_colour(frontend *fe, float *output);
void deactivate_timer(frontend *fe);
void activate_timer(frontend *fe);
void get_random_seed(void **randseed, int *randseedsize);

/*
 * drawing.c
 */
drawing *drawing_new(const drawing_api *api, midend *me, void *handle);
void drawing_free(drawing *dr);
void draw_text(drawing *dr, int x, int y, int fonttype, int fontsize,
               int align, int colour, const char *text);
void draw_rect(drawing *dr, int x, int y, int w, int h, int colour);
void draw_line(drawing *dr, int x1, int y1, int x2, int y2, int colour);
void draw_polygon(drawing *dr, const int *coords, int npoints,
                  int fillcolour, int outlinecolour);
void draw_circle(drawing *dr, int cx, int cy, int radius,
                 int fillcolour, int outlinecolour);
void draw_thick_line(drawing *dr, float thickness,
             float x1, float y1, float x2, float y2, int colour);
void clip(drawing *dr, int x, int y, int w, int h);
void unclip(drawing *dr);
void start_draw(drawing *dr);
void draw_update(drawing *dr, int x, int y, int w, int h);
void end_draw(drawing *dr);
char *text_fallback(drawing *dr, const char *const *strings, int nstrings);
void status_bar(drawing *dr, const char *text);
char *get_statustext(drawing *dr);
blitter *blitter_new(drawing *dr, int w, int h);
void blitter_free(drawing *dr, blitter *bl);
/* save puts the portion of the current display with top-left corner
 * (x,y) to the blitter. load puts it back again to the specified
 * coords, or else wherever it was saved from
 * (if x = y = BLITTER_FROMSAVED). */
void blitter_save(drawing *dr, blitter *bl, int x, int y);
#define BLITTER_FROMSAVED (-1)
void blitter_load(drawing *dr, blitter *bl, int x, int y);
void print_begin_doc(drawing *dr, int pages);
void print_begin_page(drawing *dr, int number);
void print_begin_puzzle(drawing *dr, float xm, float xc,
            float ym, float yc, int pw, int ph, float wmm,
            float scale);
void print_end_puzzle(drawing *dr);
void print_end_page(drawing *dr, int number);
void print_end_doc(drawing *dr);
void print_get_colour(drawing *dr, int colour, bool printing_in_colour,
              int *hatch, float *r, float *g, float *b);
int print_mono_colour(drawing *dr, int grey); /* 0==black, 1==white */
int print_grey_colour(drawing *dr, float grey);
int print_hatched_colour(drawing *dr, int hatch);
int print_rgb_mono_colour(drawing *dr, float r, float g, float b, int mono);
int print_rgb_grey_colour(drawing *dr, float r, float g, float b, float grey);
int print_rgb_hatched_colour(drawing *dr, float r, float g, float b,
                 int hatch);
void print_line_width(drawing *dr, int width);
void print_line_dotted(drawing *dr, bool dotted);

/*
 * midend.c
 */
midend *midend_new(frontend *fe, const game *ourgame,
           const drawing_api *drapi, void *drhandle);
void midend_free(midend *me);
const game *midend_which_game(midend *me);
void midend_set_params(midend *me, game_params *params);
game_params *midend_get_params(midend *me);
void midend_size(midend *me, int *x, int *y, bool user_size,
                 double device_pixel_ratio);
void midend_reset_tilesize(midend *me);
void midend_new_game(midend *me);
void midend_restart_game(midend *me);
void midend_stop_anim(midend *me);
enum { PKR_QUIT = 0, PKR_SOME_EFFECT, PKR_NO_EFFECT, PKR_UNUSED };
int midend_process_key(midend *me, int x, int y, int button, bool swapped);
key_label *midend_request_keys(midend *me, int *nkeys);
const char *midend_current_key_label(midend *me, int button);
void midend_force_redraw(midend *me);
void midend_redraw(midend *me);
float *midend_colours(midend *me, int *ncolours);
void midend_freeze_timer(midend *me, float tprop);
void midend_timer(midend *me, float tplus);
struct preset_menu *midend_get_presets(midend *me, int *id_limit);
int midend_which_preset(midend *me);
bool midend_wants_statusbar(midend *me);
enum { CFG_SETTINGS, CFG_SEED, CFG_DESC, CFG_PREFS, CFG_FRONTEND_SPECIFIC };
config_item *midend_get_config(midend *me, int which, char **wintitle);
const char *midend_set_config(midend *me, int which, config_item *cfg);
const char *midend_game_id(midend *me, const char *id);
char *midend_get_game_id(midend *me);
char *midend_get_random_seed(midend *me);
bool midend_can_format_as_text_now(midend *me);
char *midend_text_format(midend *me);
const char *midend_solve(midend *me);
int midend_status(midend *me);
bool midend_can_undo(midend *me);
bool midend_can_redo(midend *me);
void midend_supersede_game_desc(midend *me, const char *desc,
                                const char *privdesc);
char *midend_rewrite_statusbar(midend *me, const char *text);
void midend_serialise(midend *me,
                      void (*write)(void *ctx, const void *buf, int len),
                      void *wctx);
const char *midend_deserialise(midend *me,
                               bool (*read)(void *ctx, void *buf, int len),
                               void *rctx);
const char *midend_load_prefs(
    midend *me, bool (*read)(void *ctx, void *buf, int len), void *rctx);
void midend_save_prefs(midend *me,
                       void (*write)(void *ctx, const void *buf, int len),
                       void *wctx);
const char *identify_game(char **name,
                          bool (*read)(void *ctx, void *buf, int len),
                          void *rctx);
void midend_request_id_changes(midend *me, void (*notify)(void *), void *ctx);
bool midend_get_cursor_location(midend *me, int *x, int *y, int *w, int *h);
char *midend_get_statustext(midend *me);

/* Printing functions supplied by the mid-end */
const char *midend_print_puzzle(midend *me, document *doc, bool with_soln);
int midend_tilesize(midend *me);

/*
 * malloc.c
 */
void *smalloc(size_t size);
void *srealloc(void *p, size_t size);
void sfree(void *p);
char *dupstr(const char *s);
#define snew(type) \
    ( (type *) smalloc (sizeof (type)) )
#define snewn(number, type) \
    ( (type *) smalloc ((number) * sizeof (type)) )
#define sresize(array, number, type) \
    ( (type *) srealloc ((array), (number) * sizeof (type)) )

/*
 * misc.c
 */
void free_cfg(config_item *cfg);
void free_keys(key_label *keys, int nkeys);
void obfuscate_bitmap(unsigned char *bmp, int bits, bool decode);
char *fgetline(FILE *fp);
char *make_prefs_path(const char *dir, const char *sep,
                      const game *game, const char *suffix);
int n_times_root_k(int n, int k);

/* allocates output each time. len is always in bytes of binary data.
 * May assert (or just go wrong) if lengths are unchecked. */
char *bin2hex(const unsigned char *in, int inlen);
unsigned char *hex2bin(const char *in, int outlen);

/* Returns 0 or 1 if the environment variable is set, or dflt if not.
 * dflt may be a third value if it needs to be. */
int getenv_bool(const char *name, int dflt);

/* Mixes two colours in specified proportions. */
void colour_mix(const float src1[3], const float src2[3], float p,
                float dst[3]);
/* Sets (and possibly dims) background from frontend default colour,
 * and auto-generates highlight and lowlight colours too. */
void game_mkhighlight(frontend *fe, float *ret,
                      int background, int highlight, int lowlight);
/* As above, but starts from a provided background colour rather
 * than the frontend default. */
void game_mkhighlight_specific(frontend *fe, float *ret,
                   int background, int highlight, int lowlight);

/* Randomly shuffles an array of items. */
void shuffle(void *array, int nelts, int eltsize, random_state *rs);

/* Draw a rectangle outline, using the drawing API's draw_polygon. */
void draw_rect_outline(drawing *dr, int x, int y, int w, int h,
                       int colour);

/* Draw a set of rectangle corners (e.g. for a cursor display). */
void draw_rect_corners(drawing *dr, int cx, int cy, int r, int col);

char *move_cursor(int button, int *x, int *y, int maxw, int maxh, bool wrap,
                  bool *visible);

/* Used in netslide.c and sixteen.c for cursor movement around edge. */
int c2pos(int w, int h, int cx, int cy);
int c2diff(int w, int h, int cx, int cy, int button);
void pos2c(int w, int h, int pos, int *cx, int *cy);

/* Draws text with an 'outline' formed by offsetting the text
 * by one pixel; useful for highlighting. Outline is omitted if -1. */
void draw_text_outline(drawing *dr, int x, int y, int fonttype,
                       int fontsize, int align,
                       int text_colour, int outline_colour, const char *text);

/* Copies text left-justified with spaces. Length of string must be
 * less than buffer size. */
void copy_left_justified(char *buf, size_t sz, const char *str);

/* Returns a generic label based on the value of `button.' To be used
   whenever a `label' field returned by the request_keys() game
   function is NULL. Dynamically allocated, to be freed by caller. */
char *button2label(int button);

/*
 * dsf.c
 */
typedef struct DSF DSF;
DSF *dsf_new(int size);
void dsf_free(DSF *dsf);

void dsf_copy(DSF *to, DSF *from);

/* Basic dsf operations, return the canonical element of a class,
 * check if two elements are in the same class, and return the size of
 * a class. These work on all types of dsf. */
int dsf_canonify(DSF *dsf, int n);
bool dsf_equivalent(DSF *dsf, int n1, int n2);
int dsf_size(DSF *dsf, int n);

/* Merge two elements and their classes. Not legal on a flip dsf. */
void dsf_merge(DSF *dsf, int n1, int n2);

/* Special dsf that tracks the minimal element of every equivalence
 * class, and a function to query it. */
DSF *dsf_new_min(int size);
int dsf_minimal(DSF *dsf, int n);

/* Special dsf that tracks whether pairs of elements in the same class
 * have flipped sense relative to each other. Merge function takes an
 * argument saying whether n1 and n2 are opposite to each other;
 * canonify function will report whether n is opposite to the returned
 * element. */
DSF *dsf_new_flip(int size);
void dsf_merge_flip(DSF *dsf, int n1, int n2, bool flip);
int dsf_canonify_flip(DSF *dsf, int n, bool *flip);

/* Reinitialise a dsf to the starting 'all elements distinct' state. */
void dsf_reinit(DSF *dsf);


/*
 * tdq.c
 */

/*
 * Data structure implementing a 'to-do queue', a simple
 * de-duplicating to-do list mechanism.
 *
 * Specification: a tdq is a queue which can hold integers from 0 to
 * n-1, where n was some constant specified at tdq creation time. No
 * integer may appear in the queue's current contents more than once;
 * an attempt to add an already-present integer again will do nothing,
 * so that that integer is removed from the queue at the position
 * where it was _first_ inserted. The add and remove operations take
 * constant time.
 *
 * The idea is that you might use this in applications like solvers:
 * keep a tdq listing the indices of grid squares that you currently
 * need to process in some way. Whenever you modify a square in a way
 * that will require you to re-scan its neighbours, add them to the
 * list with tdq_add; meanwhile you're constantly taking elements off
 * the list when you need another square to process. In solvers where
 * deductions are mostly localised, this should prevent repeated
 * O(N^2) loops over the whole grid looking for something to do. (But
 * if only _most_ of the deductions are localised, then you should
 * respond to an empty to-do list by re-adding everything using
 * tdq_fill, so _then_ you rescan the whole grid looking for newly
 * enabled non-local deductions. Only if you've done that and emptied
 * the list again finding nothing new to do are you actually done.)
 */
typedef struct tdq tdq;
tdq *tdq_new(int n);
void tdq_free(tdq *tdq);
void tdq_add(tdq *tdq, int k);
int tdq_remove(tdq *tdq);        /* returns -1 if nothing available */
void tdq_fill(tdq *tdq);         /* add everything to the tdq at once */

/*
 * laydomino.c
 */
int *domino_layout(int w, int h, random_state *rs);
void domino_layout_prealloc(int w, int h, random_state *rs,
                            int *grid, int *grid2, int *list);
/*
 * version.c
 */
extern char ver[];

/*
 * random.c
 */
random_state *random_new(const char *seed, int len);
random_state *random_copy(random_state *tocopy);
unsigned long random_bits(random_state *state, int bits);
unsigned long random_upto(random_state *state, unsigned long limit);
void random_free(random_state *state);
char *random_state_encode(random_state *state);
random_state *random_state_decode(const char *input);
/* random.c also exports SHA, which occasionally comes in useful. */
#if HAVE_STDINT_H
#include <stdint.h>
typedef uint32_t uint32;
#elif UINT_MAX >= 4294967295L
typedef unsigned int uint32;
#else
typedef unsigned long uint32;
#endif
typedef struct {
    uint32 h[5];
    unsigned char block[64];
    int blkused;
    uint32 lenhi, lenlo;
} SHA_State;
void SHA_Init(SHA_State *s);
void SHA_Bytes(SHA_State *s, const void *p, int len);
void SHA_Final(SHA_State *s, unsigned char *output);
void SHA_Simple(const void *p, int len, unsigned char *output);

/*
 * printing.c
 */
document *document_new(int pw, int ph, float userscale);
void document_free(document *doc);
void document_add_puzzle(document *doc, const game *game, game_params *par,
             game_ui *ui, game_state *st, game_state *st2);
int document_npages(const document *doc);
void document_begin(const document *doc, drawing *dr);
void document_end(const document *doc, drawing *dr);
void document_print_page(const document *doc, drawing *dr, int page_nr);
void document_print(const document *doc, drawing *dr);

/*
 * ps.c
 */
psdata *ps_init(FILE *outfile, bool colour);
void ps_free(psdata *ps);
drawing *ps_drawing_api(psdata *ps);

/*
 * combi.c: provides a structure and functions for iterating over
 * combinations (i.e. choosing r things out of n).
 */
typedef struct _combi_ctx {
  int r, n, nleft, total;
  int *a;
} combi_ctx;

combi_ctx *new_combi(int r, int n);
void reset_combi(combi_ctx *combi);
combi_ctx *next_combi(combi_ctx *combi); /* returns NULL for end */
void free_combi(combi_ctx *combi);

/*
 * divvy.c
 */
/* divides w*h rectangle into pieces of size k. Returns w*h dsf. */
DSF *divvy_rectangle(int w, int h, int k, random_state *rs);
/* Same, but only tries once, and may fail. (Exposed for test program.) */
DSF *divvy_rectangle_attempt(int w, int h, int k, random_state *rs);

/*
 * findloop.c
 */
struct findloopstate;
struct findloopstate *findloop_new_state(int nvertices);
void findloop_free_state(struct findloopstate *);
/*
 * Callback provided by the client code to enumerate the graph
 * vertices joined directly to a given vertex.
 *
 * Semantics: if vertex >= 0, return one of its neighbours; if vertex
 * < 0, return a previously unmentioned neighbour of whatever vertex
 * was last passed as input. Write to 'ctx' as necessary to store
 * state. In either case, return < 0 if no such vertex can be found.
 */
typedef int (*neighbour_fn_t)(int vertex, void *ctx);
/*
 * Actual function to find loops. 'ctx' will be passed unchanged to
 * the 'neighbour' function to query graph edges. Returns false if no
 * loop was found, or true if one was.
 */
bool findloop_run(struct findloopstate *state, int nvertices,
                  neighbour_fn_t neighbour, void *ctx);
/*
 * Query whether an edge is part of a loop, in the output of
 * find_loops.
 *
 * Due to the internal storage format, if you pass u,v which are not
 * connected at all, the output will be true. (The algorithm actually
 * stores an exhaustive list of *non*-loop edges, because there are
 * fewer of those, so really it's querying whether the edge is on that
 * list.)
 */
bool findloop_is_loop_edge(struct findloopstate *state, int u, int v);

/*
 * Alternative query function, which returns true if the u-v edge is a
 * _bridge_, i.e. a non-loop edge, i.e. an edge whose removal would
 * disconnect a currently connected component of the graph.
 *
 * If the return value is true, then the numbers of vertices that
 * would be in the new components containing u and v are written into
 * u_vertices and v_vertices respectively.
 */
bool findloop_is_bridge(
    struct findloopstate *pv, int u, int v, int *u_vertices, int *v_vertices);

/*
 * Helper function to sort an array. Differs from standard qsort in
 * that it takes a context parameter that is passed to the compare
 * function.
 *
 * I wrap it in a macro so that you only need to give the element
 * count of the array. The element size is determined by sizeof.
 */
typedef int (*arraysort_cmpfn_t)(const void *av, const void *bv, void *ctx);
void arraysort_fn(void *array, size_t nmemb, size_t size,
                  arraysort_cmpfn_t cmp, void *ctx);
#define arraysort(array, nmemb, cmp, ctx) \
    arraysort_fn(array, nmemb, sizeof(*(array)), cmp, ctx)

/*
 * Data structure containing the function calls and data specific
 * to a particular game. This is enclosed in a data structure so
 * that a particular platform can choose, if it wishes, to compile
 * all the games into a single combined executable rather than
 * having lots of little ones.
 */
struct game {
    const char *name;
    const char *winhelp_topic, *htmlhelp_topic;
    const char *rules;
    game_params *(*default_params)(void);
    bool (*fetch_preset)(int i, char **name, game_params **params);
    struct preset_menu *(*preset_menu)(void);
    void (*decode_params)(game_params *, char const *string);
    char *(*encode_params)(const game_params *, bool full);
    void (*free_params)(game_params *params);
    game_params *(*dup_params)(const game_params *params);
    bool can_configure;
    config_item *(*configure)(const game_params *params);
    game_params *(*custom_params)(const config_item *cfg);
    const char *(*validate_params)(const game_params *params, bool full);
    char *(*new_desc)(const game_params *params, random_state *rs,
              char **aux, bool interactive);
    const char *(*validate_desc)(const game_params *params, const char *desc);
    game_state *(*new_game)(midend *me, const game_params *params,
                            const char *desc);
    game_state *(*dup_game)(const game_state *state);
    void (*free_game)(game_state *state);
    bool can_solve;
    char *(*solve)(const game_state *orig, const game_state *curr,
                   const char *aux, const char **error);
    bool can_format_as_text_ever;
    bool (*can_format_as_text_now)(const game_params *params);
    char *(*text_format)(const game_state *state);
    bool has_preferences;
    config_item *(*get_prefs)(game_ui *ui);
    void (*set_prefs)(game_ui *ui, const config_item *cfg);
    game_ui *(*new_ui)(const game_state *state);
    void (*free_ui)(game_ui *ui);
    char *(*encode_ui)(const game_ui *ui);
    void (*decode_ui)(game_ui *ui, const char *encoding,
                      const game_state *state);
    key_label *(*request_keys)(const game_params *params, const game_ui *ui, int *nkeys);
    void (*changed_state)(game_ui *ui, const game_state *oldstate,
                          const game_state *newstate);
    const char *(*current_key_label)(const game_ui *ui,
                                     const game_state *state, int button);
    char *(*interpret_move)(const game_state *state, game_ui *ui,
                            const game_drawstate *ds, int x, int y, int button, bool swapped);
    game_state *(*execute_move)(const game_state *state, const game_ui *ui, const char *move);
    int preferred_tilesize;
    void (*compute_size)(const game_params *params, int tilesize,
                         const game_ui *ui, int *x, int *y);
    void (*set_size)(drawing *dr, game_drawstate *ds,
             const game_params *params, int tilesize);
    float *(*colours)(frontend *fe, int *ncolours);
    game_drawstate *(*new_drawstate)(drawing *dr, const game_state *state);
    void (*free_drawstate)(drawing *dr, game_drawstate *ds);
    void (*redraw)(drawing *dr, game_drawstate *ds, const game_state *oldstate,
           const game_state *newstate, int dir, const game_ui *ui,
                   float anim_time, float flash_time);
    float (*anim_length)(const game_state *oldstate,
                         const game_state *newstate, int dir, game_ui *ui);
    float (*flash_length)(const game_state *oldstate,
                          const game_state *newstate, int dir, game_ui *ui);
    void (*get_cursor_location)(const game_ui *ui,
                                const game_drawstate *ds,
                                const game_state *state,
                                const game_params *params,
                                int *x, int *y, int *w, int *h);
    int (*status)(const game_state *state);
    bool can_print, can_print_in_colour;
    void (*print_size)(const game_params *params, const game_ui *ui,
                       float *x, float *y);
    void (*print)(drawing *dr, const game_state *state, const game_ui *ui,
                  int tilesize);
    bool wants_statusbar;
    bool is_timed;
    bool (*timing_state)(const game_state *state, game_ui *ui);
    int flags;
};

/*
 * Data structure containing the drawing API implemented by the
 * front end and also by cross-platform printing modules such as
 * PostScript.
 */
struct drawing_api {
    void (*draw_text)(void *handle, int x, int y, int fonttype, int fontsize,
              int align, int colour, const char *text);
    void (*draw_rect)(void *handle, int x, int y, int w, int h, int colour);
    void (*draw_line)(void *handle, int x1, int y1, int x2, int y2,
              int colour);
    void (*draw_polygon)(void *handle, const int *coords, int npoints,
             int fillcolour, int outlinecolour);
    void (*draw_circle)(void *handle, int cx, int cy, int radius,
            int fillcolour, int outlinecolour);
    void (*draw_update)(void *handle, int x, int y, int w, int h);
    void (*clip)(void *handle, int x, int y, int w, int h);
    void (*unclip)(void *handle);
    void (*start_draw)(void *handle);
    void (*end_draw)(void *handle);
    void (*status_bar)(void *handle, const char *text);
    blitter *(*blitter_new)(void *handle, int w, int h);
    void (*blitter_free)(void *handle, blitter *bl);
    void (*blitter_save)(void *handle, blitter *bl, int x, int y);
    void (*blitter_load)(void *handle, blitter *bl, int x, int y);
    void (*begin_doc)(void *handle, int pages);
    void (*begin_page)(void *handle, int number);
    void (*begin_puzzle)(void *handle, float xm, float xc,
             float ym, float yc, int pw, int ph, float wmm);
    void (*end_puzzle)(void *handle);
    void (*end_page)(void *handle, int number);
    void (*end_doc)(void *handle);
    void (*line_width)(void *handle, float width);
    void (*line_dotted)(void *handle, bool dotted);
    char *(*text_fallback)(void *handle, const char *const *strings,
               int nstrings);
    void (*draw_thick_line)(void *handle, float thickness,
                float x1, float y1, float x2, float y2,
                int colour);
};

/*
 * For one-game-at-a-time platforms, there's a single structure
 * like the above, under a fixed name. For all-at-once platforms,
 * there's a list of all available puzzles in array form.
 */
#ifdef COMBINED
extern const game *gamelist[];
extern const int gamecount;
#else
extern const game thegame;
#endif

/*
 * Special string values to return from interpret_move.
 *
 * MOVE_UI_UPDATE is for the case where the game UI has been updated
 * but no actual move is being appended to the undo chain.
 *
 * MOVE_NO_EFFECT is for when the key was understood by the puzzle,
 * but it happens that there isn't effect, not even a UI change.
 *
 * MOVE_UNUSED is for keys that the puzzle has no use for at all.
 *
 * Each must be declared as a non-const char, but should never
 * actually be modified by anyone.
 */
extern char MOVE_UI_UPDATE[], MOVE_NO_EFFECT[], MOVE_UNUSED[];

/* A little bit of help to lazy developers */
#define DEFAULT_STATUSBAR_TEXT "Use status_bar() to fill this in."

#endif /* PUZZLES_PUZZLES_H */

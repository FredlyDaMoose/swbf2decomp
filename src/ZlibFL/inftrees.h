typedef struct {
    unsigned char op;
    unsigned char bits;
    unsigned short val;
} code;

#define ENOUGH 1440
#define MAXD 154

typedef enum {
    CODES,
    LENS,
    DISTS
} codetype;

extern int inflate_table OF((codetype type, unsigned short FAR *lens,
                             unsigned codes, code FAR * FAR *table,
                             unsigned FAR *bits, unsigned short FAR *work));

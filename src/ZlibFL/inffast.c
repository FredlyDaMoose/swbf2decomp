#include "zutil.h"
#include "inftrees.h"
#include "inflate.h"
#include "inffast.h"

#ifndef ASMINF

#ifdef POSTINC
#  define OFF 0
#  define PUP(a) *(a)++
#else
#  define OFF 1
#  define PUP(a) *++(a)
#endif

// LIBRARY: SWBF2 0x006989d0
// inflate_fast
void inflate_fast(strm, start)
z_streamp strm;
unsigned start;
{
    struct inflate_state FAR *state;
    unsigned char FAR *in;
    unsigned char FAR *last;
    unsigned char FAR *out;
    unsigned char FAR *beg;
    unsigned char FAR *end;
    unsigned wsize;
    unsigned whave;
    unsigned write;
    unsigned char FAR *window;
    unsigned long hold;
    unsigned bits;
    code const FAR *lcode;
    code const FAR *dcode;
    unsigned lmask;
    unsigned dmask;
    code this;
    unsigned op;

    unsigned len;
    unsigned dist;
    unsigned char FAR *from;

    state = (struct inflate_state FAR *)strm->state;
    in = strm->next_in - OFF;
    last = in + (strm->avail_in - 5);
    out = strm->next_out - OFF;
    beg = out - (start - strm->avail_out);
    end = out + (strm->avail_out - 257);
    wsize = state->wsize;
    whave = state->whave;
    write = state->write;
    window = state->window;
    hold = state->hold;
    bits = state->bits;
    lcode = state->lencode;
    dcode = state->distcode;
    lmask = (1U << state->lenbits) - 1;
    dmask = (1U << state->distbits) - 1;

    do {
        if (bits < 15) {
            hold += (unsigned long)(PUP(in)) << bits;
            bits += 8;
            hold += (unsigned long)(PUP(in)) << bits;
            bits += 8;
        }
        this = lcode[hold & lmask];
      dolen:
        op = (unsigned)(this.bits);
        hold >>= op;
        bits -= op;
        op = (unsigned)(this.op);
        if (op == 0) {
            Tracevv((stderr, this.val >= 0x20 && this.val < 0x7f ?
                    "inflate:         literal '%c'\n" :
                    "inflate:         literal 0x%02x\n", this.val));
            PUP(out) = (unsigned char)(this.val);
        }
        else if (op & 16) {
            len = (unsigned)(this.val);
            op &= 15;
            if (op) {
                if (bits < op) {
                    hold += (unsigned long)(PUP(in)) << bits;
                    bits += 8;
                }
                len += (unsigned)hold & ((1U << op) - 1);
                hold >>= op;
                bits -= op;
            }
            Tracevv((stderr, "inflate:         length %u\n", len));
            if (bits < 15) {
                hold += (unsigned long)(PUP(in)) << bits;
                bits += 8;
                hold += (unsigned long)(PUP(in)) << bits;
                bits += 8;
            }
            this = dcode[hold & dmask];
          dodist:
            op = (unsigned)(this.bits);
            hold >>= op;
            bits -= op;
            op = (unsigned)(this.op);
            if (op & 16) {
                dist = (unsigned)(this.val);
                op &= 15;
                if (bits < op) {
                    hold += (unsigned long)(PUP(in)) << bits;
                    bits += 8;
                    if (bits < op) {
                        hold += (unsigned long)(PUP(in)) << bits;
                        bits += 8;
                    }
                }
                dist += (unsigned)hold & ((1U << op) - 1);
                hold >>= op;
                bits -= op;
                Tracevv((stderr, "inflate:         distance %u\n", dist));
                op = (unsigned)(out - beg);
                if (dist > op) {
                    op = dist - op;
                    if (op > whave) {
                        strm->msg = (char *)"invalid distance too far back";
                        state->mode = BAD;
                        break;
                    }
                    from = window - OFF;
                    if (write == 0) {
                        from += wsize - op;
                        if (op < len) {
                            len -= op;
                            do {
                                PUP(out) = PUP(from);
                            } while (--op);
                            from = out - dist;
                        }
                    }
                    else if (write < op) {
                        from += wsize + write - op;
                        op -= write;
                        if (op < len) {
                            len -= op;
                            do {
                                PUP(out) = PUP(from);
                            } while (--op);
                            from = window - OFF;
                            if (write < len) {
                                op = write;
                                len -= op;
                                do {
                                    PUP(out) = PUP(from);
                                } while (--op);
                                from = out - dist;
                            }
                        }
                    }
                    else {
                        from += write - op;
                        if (op < len) {
                            len -= op;
                            do {
                                PUP(out) = PUP(from);
                            } while (--op);
                            from = out - dist;
                        }
                    }
                    while (len > 2) {
                        PUP(out) = PUP(from);
                        PUP(out) = PUP(from);
                        PUP(out) = PUP(from);
                        len -= 3;
                    }
                    if (len) {
                        PUP(out) = PUP(from);
                        if (len > 1)
                            PUP(out) = PUP(from);
                    }
                }
                else {
                    from = out - dist;
                    do {
                        PUP(out) = PUP(from);
                        PUP(out) = PUP(from);
                        PUP(out) = PUP(from);
                        len -= 3;
                    } while (len > 2);
                    if (len) {
                        PUP(out) = PUP(from);
                        if (len > 1)
                            PUP(out) = PUP(from);
                    }
                }
            }
            else if ((op & 64) == 0) {
                this = dcode[this.val + (hold & ((1U << op) - 1))];
                goto dodist;
            }
            else {
                strm->msg = (char *)"invalid distance code";
                state->mode = BAD;
                break;
            }
        }
        else if ((op & 64) == 0) {
            this = lcode[this.val + (hold & ((1U << op) - 1))];
            goto dolen;
        }
        else if (op & 32) {
            Tracevv((stderr, "inflate:         end of block\n"));
            state->mode = TYPE;
            break;
        }
        else {
            strm->msg = (char *)"invalid literal/length code";
            state->mode = BAD;
            break;
        }
    } while (in < last && out < end);

    len = bits >> 3;
    in -= len;
    bits -= len << 3;
    hold &= (1U << bits) - 1;

    strm->next_in = in + OFF;
    strm->next_out = out + OFF;
    strm->avail_in = (unsigned)(in < last ? 5 + (last - in) : 5 - (in - last));
    strm->avail_out = (unsigned)(out < end ?
                                 257 + (end - out) : 257 - (out - end));
    state->hold = hold;
    state->bits = bits;
    return;
}

#endif

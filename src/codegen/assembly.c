#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "globals/globals.h"
#include "palm/dbug.h"
#include "print/print.h"
#include "table/consttable.h"
#include "table/funtable.h"
#include "table/vartable.h"
#include "utils.h"

void CGAinit(void) {
    if (globals.output_file) {
        DATA_CGA_GET()->output = fopen(globals.output_file, "w");
        if (!DATA_CGA_GET()->output) {
            emit_message(L_WARNING, "couldn't open '%s': %s (os error %d)",
                         globals.input_file, strerror(errno), errno);
            DATA_CGA_GET()->output = stdout;
        }
    } else {
        DATA_CGA_GET()->output = stdout;
    }
}
void CGAfini(void) {
    if (DATA_CGA_GET()->output != stdout) {
        fclose(DATA_CGA_GET()->output);
    }
}

static int oprintf(const char *f, ...) __attribute__((format(printf, 1, 2)));
static int oprintf(const char *f, ...) {
    va_list ap;

    va_start(ap, f);
    int res = vfprintf(DATA_CGA_GET()->output, f, ap);
    va_end(ap);

    return res;
}

static void short_basic_ty(enum BasicType ty) {
    switch (ty) {
    case TY_int:
        oprintf("i");
        break;
    case TY_float:
        oprintf("f");
        break;
    case TY_bool:
        oprintf("b");
        break;
    default:
        DBUG_ASSERT(false, "unknown output ty %s", fmt_BasicType(ty));
    }
}

static void short_ty(thin_vartype ty) {
    if (ty.dims > 0) {
        oprintf("a");
    } else {
        short_basic_ty(ty.ty);
    }
}

static void long_basic_ty(enum BasicType ty) {
    switch (ty) {
    case TY_void:
        oprintf("void");
        break;
    case TY_int:
        oprintf("int");
        break;
    case TY_float:
        oprintf("float");
        break;
    case TY_bool:
        oprintf("bool");
        break;
    default:
        DBUG_ASSERT(false, "unknown output ty %s", fmt_BasicType(ty));
    }
}

static void long_ty(thin_vartype ty) {
    long_basic_ty(ty.ty);

    if (ty.dims > 0) {
        oprintf("[]");
    }
}

static void constvalue(consttable_entry e) {
    switch (e.ty) {
    case TY_int:
        oprintf("%d", e.intval);
        break;
    case TY_float:
        oprintf("%lg", e.floatval);
        break;
    default:
        DBUG_ASSERT(false, "unknown constvalue ty %s", fmt_BasicType(e.ty));
    }
}

node_st *CGAprogram(node_st *node) {

    consttable *ct = PROGRAM_CONSTTABLE(node);
    DATA_CGA_GET()->consttable = ct;

    for (int ct_i = 0; ct_i < ct->len; ct_i++) {
        consttable_entry e = ct->buf[ct_i];
        oprintf(".const ");
        long_basic_ty(e.ty);
        oprintf(" ");
        constvalue(e);
        oprintf(" ; %d\n", ct_i);
    }
    oprintf("\n");

    funtable *ft = PROGRAM_FUNTABLE(node);
    DATA_CGA_GET()->funtable = ft;

    int import_fun = 0;
    for (int ft_i = 0; ft_i < ft->len; ft_i++) {
        funtable_entry e = ft->buf[ft_i];

        if (e.exported) {
            oprintf(".exportfun");
        } else if (e.external) {
            oprintf(".importfun");
        } else {
            continue;
        }

        bool has_array = false;
        for (int ty_i = 0; ty_i < e.ty.len; ty_i++) {
            thin_vartype ty = e.ty.buf[ty_i];
            if (ty.dims > 0) {
                has_array = true;
                break;
            }
        }

        oprintf(" \"%s\" ", has_array ? e.mangled_name : e.name);

        long_basic_ty(e.ty.ty);
        for (int ty_i = 0; ty_i < e.ty.len; ty_i++) {
            thin_vartype ty = e.ty.buf[ty_i];
            for (int dim_i = 0; dim_i < ty.dims; dim_i++) {
                oprintf(" int");
            }
            oprintf(" ");
            long_ty(ty);
        }

        if (e.exported) {
            oprintf(" %s", e.mangled_name);
        } else if (e.external) {
            int l = ft->buf[ft_i].new_l = import_fun++;
            oprintf(" ; %d", l);
        }
        oprintf("\n");
    }
    oprintf("\n");

    vartable *vt = PROGRAM_VARTABLE(node);
    DATA_CGA_GET()->vartable = vt;

    int global_var = 0;
    int import_var = 0;
    for (int vt_i = 0; vt_i < vt->len; vt_i++) {
        vartable_entry e = vt->buf[vt_i];

        thin_vartype ty = {e.ty.ty, e.ty.len};

        if (e.external) {
            oprintf(".importvar \"%s\" ", e.name);
            long_ty(ty);
            oprintf(" ; %d\n", import_var);

            vt->buf[vt_i].new_l = import_var++;
        } else {
            oprintf(".global ");
            long_ty(ty);
            oprintf(" ; %s %d\n", e.name, global_var);

            if (e.exported) {
                oprintf(".exportvar \"%s\" %d\n", e.name, global_var);
            }

            vt->buf[vt_i].new_l = global_var++;
        }
    }

    TRAVchildren(node);
    oprintf("\n");

    return node;
}

node_st *CGAfundecl(node_st *node) {
    if (FUNDECL_EXTERNAL(node)) {
        return node;
    }

    funtable *parent_ft = DATA_CGA_GET()->funtable;
    DATA_CGA_GET()->funtable = FUNBODY_FUNTABLE(FUNDECL_BODY(node));

    vartable *parent_vt = DATA_CGA_GET()->vartable;
    vartable *vt = FUNDECL_VARTABLE(node);
    DATA_CGA_GET()->vartable = vt;

    oprintf("\n%s:\n", parent_ft->buf[FUNDECL_L(node)].mangled_name);

    int locals = 0;
    for (int i = 0; i < vt->len; i++) {
        if (!vt->buf[i].param) {
            locals++;
        }
    }
    if (locals > 0) {
        oprintf("\tesr %d\n", locals);
    }

    node_st *body = FUNDECL_BODY(node);

    DATA_CGA_GET()->nesting++;
    TRAVstmts(body);
    TRAVdecls(body);
    DATA_CGA_GET()->nesting--;

    DATA_CGA_GET()->funtable = parent_ft;
    DATA_CGA_GET()->vartable = parent_vt;

    return node;
}

static int create_label(void) {
    return DATA_CGA_GET()->label++;
}

static void branch_on(node_st *cond, bool val) {
    if (NODE_TYPE(cond) == NT_BOOL && BOOL_VAL(cond) == val) {
        oprintf("\tjump ");
        return;
    }

    bool swap = NODE_TYPE(cond) == NT_MONOP && MONOP_OP(cond) == MO_not;
    if (swap) {
        cond = MONOP_EXPR(cond);
    }

    TRAVdo(cond);

    oprintf("\tbranch_");
    if (swap == val) {
        oprintf("f ");
    } else {
        oprintf("t ");
    }
}

node_st *CGAifelse(node_st *node) {
    node_st *els = IFELSE_ELSE_BLOCK(node);

    int else_label;
    if (els) {
        else_label = create_label();
    }

    branch_on(IFELSE_EXPR(node), false);

    int end_label = create_label();
    if (els) {
        oprintf("%d_else\n", else_label);
    } else {
        oprintf("%d_end\n", end_label);
    }

    TRAVif_block(node);

    if (els) {
        oprintf("\tjump %d_end\n", end_label);
        oprintf("%d_else:\n", else_label);
        TRAVelse_block(node);
    }

    oprintf("%d_end:\n", end_label);

    return node;
}

node_st *CGAdowhile(node_st *node) {
    int head_label = create_label();
    oprintf("%d_head:\n", head_label);

    TRAVstmts(node);

    branch_on(DOWHILE_EXPR(node), true);
    oprintf("%d_head\n", head_label);

    return node;
}

bool inc(node_st *node) {
    node_st *ref = ASSIGN_REF(node);
    if (VARREF_N(ref) != 0 || VARREF_RESOLVED_TY(ref) != TY_int) {
        return false;
    }

    node_st *expr = ASSIGN_EXPR(node);
    if (NODE_TYPE(expr) != NT_BINOP || BINOP_OP(expr) != BO_add) {
        return false;
    }

    node_st *base = BINOP_RIGHT(expr);
    if (NODE_TYPE(base) != NT_VARREF || VARREF_N(base) != 0) {
        return false;
    }

    int l = VARREF_L(ref);
    vartable_entry e = DATA_CGA_GET()->vartable->buf[l];

    node_st *constant = BINOP_LEFT(expr);
    switch (NODE_TYPE(constant)) {
    case NT_CONSTREF: {
        int cl = CONSTREF_L(constant);
        oprintf("\tiinc %d %d ; %s + ", l, cl, e.name);
        constvalue(DATA_CGA_GET()->consttable->buf[cl]);
        oprintf("\n");
        return true;
    }
    case NT_INT: {
        int val = INT_VAL(constant);
        switch (val) {
        case 1:
            oprintf("\tiinc_1 %d ; %s + 1\n", l, e.name);
            return true;
        case -1:
            oprintf("\tidec_1 %d ; %s - 1\n", l, e.name);
            return true;
        default:
            return false;
        }
    }
    default:
        return false;
    }
}

node_st *CGAassign(node_st *node) {
    if (!inc(node)) {
        TRAVexpr(node);
        TRAVref(node);
    }

    return node;
}

node_st *CGAcall(node_st *node) {
    funtable_entry *e = funtable_get(
        DATA_CGA_GET()->funtable, (funtable_ref){CALL_N(node), CALL_L(node)});

    int n = CALL_N(node) - 1;
    if (n == DATA_CGA_GET()->nesting || e->external) {
        oprintf("\tisrg\n");
    } else {
        switch (n) {
        case -1:
            oprintf("\tisrl\n");
            break;
        case 0:
            oprintf("\tisr\n");
            break;
        default:
            oprintf("\tisrn %d\n", n);
            break;
        }
    }

    TRAVchildren(node);

    int a = 0;
    for (node_st *expr = CALL_EXPRS(node); expr != NULL;
         expr = EXPRS_NEXT(expr)) {
        a++;
    }

    if (e->external) {
        oprintf("\tjsre %d\n", e->new_l);
    } else {
        oprintf("\tjsr %d %s\n", a, e->mangled_name);
    }

    return node;
}

node_st *CGAstmts(node_st *node) {
    node_st *stmt = STMTS_STMT(node);
    TRAVdo(stmt);

    if (NODE_TYPE(stmt) == NT_CALL && CALL_RESOLVED_TY(stmt) != TY_void) {
        oprintf("\t");
        short_ty(TYPE(stmt));
        oprintf("pop\n");
    }

    if (STMTS_NEXT(node) != NULL) {
        TRAVdo(STMTS_NEXT(node));
    }

    return node;
}

node_st *CGAreturn(node_st *node) {
    TRAVchildren(node);

    oprintf("\t");
    node_st *expr = RETURN_EXPR(node);
    if (expr != NULL) {
        short_ty(TYPE(expr));
    }
    oprintf("return\n");

    return node;
}

node_st *CGAmalloc(node_st *node) {
    node_st *exprs = MALLOC_EXPRS(node);
    DBUG_ASSERT(EXPRS_NEXT(exprs) == NULL, "malloc has multiple dimensions");
    node_st *size = EXPRS_EXPR(exprs);

    TRAVdo(size);

    oprintf("\t");
    short_ty(TYPE(node));
    oprintf("newa\n");

    return node;
}

node_st *CGAbinop(node_st *node) {
    enum BinOpKind op = BINOP_OP(node);

    if (op == BO_and || op == BO_or) {
        bool short_if = op == BO_or;
        branch_on(BINOP_LEFT(node), short_if);

        int short_label = create_label();
        int end_label = create_label();
        oprintf("%d_short\n", short_label);

        TRAVright(node);
        oprintf("\tjump %d_end\n", end_label);

        oprintf("%d_short:\n", short_label);
        oprintf("\tbloadc_");
        if (short_if) {
            oprintf("t");
        } else {
            oprintf("f");
        }
        oprintf("\n");

        oprintf("%d_end:\n", end_label);

        return node;
    }

    TRAVchildren(node);

    oprintf("\t");
    short_ty(TYPE(BINOP_LEFT(node)));

    switch (BINOP_OP(node)) {
    case BO_add:
        oprintf("add\n");
        break;
    case BO_sub:
        oprintf("sub\n");
        break;
    case BO_mul:
        oprintf("mul\n");
        break;
    case BO_div:
        oprintf("div\n");
        break;
    case BO_mod:
        oprintf("rem\n");
        break;

    case BO_eq:
        oprintf("eq\n");
        break;
    case BO_ne:
        oprintf("ne\n");
        break;
    case BO_lt:
        oprintf("lt\n");
        break;
    case BO_le:
        oprintf("le\n");
        break;
    case BO_gt:
        oprintf("gt\n");
        break;
    case BO_ge:
        oprintf("ge\n");
        break;

    default:
        DBUG_ASSERT(false, "unknown binop");
        break;
    }

    return node;
}

node_st *CGAmonop(node_st *node) {
    TRAVchildren(node);

    switch (MONOP_OP(node)) {
    case MO_not:
        oprintf("\t");
        short_ty(TYPE(MONOP_EXPR(node)));
        oprintf("not\n");
        break;
    case MO_pos:
        break;
    case MO_neg:
        oprintf("\t");
        short_ty(TYPE(MONOP_EXPR(node)));
        oprintf("neg\n");
        break;
    default:
        DBUG_ASSERT(false, "unknown monop");
        break;
    }

    return node;
}

node_st *CGAcast(node_st *node) {
    TRAVchildren(node);

    enum BasicType to = CAST_RESOLVED_TY(node);
    enum BasicType from = EXPR_RESOLVED_TY(CAST_EXPR(node));

    if (from == TY_bool) {
        int zero_label = create_label();
        int end_label = create_label();

        oprintf("\tbranch_f %d_zero\n", zero_label);

        oprintf("\t");
        short_basic_ty(to);
        oprintf("loadc_1\n");

        oprintf("\tjump %d_end\n", end_label);

        oprintf("%d_zero:\n", zero_label);

        oprintf("\t");
        short_basic_ty(to);
        oprintf("loadc_0\n");

        oprintf("%d_end:\n", end_label);
    } else if (from == TY_int && to == TY_float) {
        oprintf("\ti2f\n");
    } else if (from == TY_float && to == TY_int) {
        oprintf("\tf2i\n");
    } else if (from != to) {
        DBUG_ASSERT(false, "unexpected cast %s -> %s", fmt_BasicType(from),
                    fmt_BasicType(to));
    }

    return node;
}

node_st *CGAvarref(node_st *node) {
    TRAVexprs(node);

    bool array_access = VARREF_EXPRS(node) != NULL;
    bool array = array_access || VARREF_RESOLVED_DIMS(node) > 0;

    oprintf("\t");
    if (array) {
        oprintf("a");
    } else {
        short_basic_ty(VARREF_RESOLVED_TY(node));
    }

    int l = VARREF_L(node);
    int n = VARREF_N(node);

    bool write = VARREF_WRITE(node);
    bool read_first = !write || array_access;

    if (read_first) {
        oprintf("load");
    } else {
        oprintf("store");
    }

    if (n == DATA_CGA_GET()->nesting) {
        vartable_entry *e =
            vartable_get(DATA_CGA_GET()->vartable, (vartable_ref){n, l});
        if (e->external) {
            oprintf("e %d", e->new_l);
        } else {
            oprintf("g %d", e->new_l);
        }
    } else if (n == 0) {
        if (read_first && l <= 3) {
            oprintf("_%d", l);
        } else {
            oprintf(" %d", l);
        }
    } else {
        oprintf("n %d %d", n, l);
    }

    oprintf(" ; %s\n", ID_VAL(VARREF_ID(node)));

    if (array_access) {
        oprintf("\t");
        short_basic_ty(VARREF_RESOLVED_TY(node));
        if (write) {
            oprintf("storea\n");
        } else {
            oprintf("loada\n");
        }
    }

    return node;
}

node_st *CGAint(node_st *node) {
    int val = INT_VAL(node);
    switch (val) {
    case 1:
        oprintf("\tiloadc_1 ; 1\n");
        break;
    case 0:
        oprintf("\tiloadc_0 ; 0\n");
        break;
    case -1:
        oprintf("\tiloadc_m1 ; -1\n");
        break;
    default:
        DBUG_ASSERT(false, "int constant not 1, 0 or -1");
    }
    return node;
}

node_st *CGAfloat(node_st *node) {
    float val = FLOAT_VAL(node);
    if (val == 1.0) {
        oprintf("\tfloadc_1 ; 1\n");
    } else if (val == 0.0) {
        oprintf("\tfloadc_0 ; 0\n");
    } else {
        DBUG_ASSERT(false, "float constant not 1 or 0");
    }
    return node;
}

node_st *CGAbool(node_st *node) {
    if (BOOL_VAL(node)) {
        oprintf("\tbloadc_t\n");
    } else {
        oprintf("\tbloadc_f\n");
    }
    return node;
}

node_st *CGAconstref(node_st *node) {
    oprintf("\t");
    short_ty(TYPE(node));

    int l = CONSTREF_L(node);
    oprintf("loadc %d ; ", l);
    constvalue(DATA_CGA_GET()->consttable->buf[l]);
    oprintf("\n");

    return node;
}

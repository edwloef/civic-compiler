#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "palm/ctinfo.h"
#include "palm/dbug.h"
#include "print/print.h"

vartype RESOLVED_TY(node_st *node) {
    vartype ty = {ARREXPR_RESOLVED_TY(node), ARREXPR_RESOLVED_DIMS(node)};
    return ty;
}

void ARcheckassign(vartype from, vartype to) {
    if (to.dims != 0 && from.dims != to.dims) {
        if (from.dims == 0) {
            CTI(CTI_ERROR, true,
                "can't assign %d-dimensional array of type '%s' to value of "
                "type '%s'",
                from.dims, fmt_BasicType(from.ty), fmt_BasicType(to.ty));
        } else {
            CTI(CTI_ERROR, true,
                "can't assign %d-dimensional array of type '%s' to "
                "%d-dimensional array of type '%s'",
                from.dims, fmt_BasicType(from.ty), to.dims,
                fmt_BasicType(to.ty));
        }
    } else if (from.ty != to.ty) {
        if (from.dims == 0) {
            if (to.dims == 0) {
                CTI(CTI_ERROR, true,
                    "can't assign value of type '%s' to value of type '%s'",
                    fmt_BasicType(from.ty), fmt_BasicType(to.ty));
            } else {
                CTI(CTI_ERROR, true,
                    "can't spread value of type '%s' into %d-dimensional array "
                    "of type '%s'",
                    fmt_BasicType(from.ty), to.dims, fmt_BasicType(to.ty));
            }
        } else {
            CTI(CTI_ERROR, true,
                "can't assign %d-dimensional array of type '%s' to "
                "%d-dimensional array "
                "of type '%s'",
                from.dims, fmt_BasicType(from.ty), to.dims,
                fmt_BasicType(to.ty));
        }
    }
    CTIabortOnError();
}

void ARinit(void) { DATA_AR_GET()->vartable = vartable_new(NULL); }

void ARfini(void) {}

node_st *ARprogram(node_st *node) {
    DATA_AR_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    PROGRAM_VARTABLE(node) = DATA_AR_GET()->vartable;

    return node;
}

node_st *ARvardecl(node_st *node) {
    TRAVchildren(node);

    int dims = 0;
    node_st *expr = TYPE_EXPRS(VARDECL_TY(node));
    while (expr) {
        dims++;
        expr = EXPRS_NEXT(expr);
    }

    vartype ty = {TYPE_TY(VARDECL_TY(node)), dims};
    vartable_entry e = {ID_VAL(VARDECL_ID(node)), ty, true};
    vartable_insert(DATA_AR_GET()->vartable, e);

    if (VARDECL_EXPR(node))
        ARcheckassign(RESOLVED_TY(VARDECL_EXPR(node)), ty);

    return node;
}

node_st *ARparam(node_st *node) {
    int dims = 0;
    node_st *id = PARAM_IDS(node);
    while (id) {
        dims++;
        vartype ty = {TY_int, 0};
        vartable_entry e = {ID_VAL(IDS_ID(id)), ty, true};
        vartable_insert(DATA_AR_GET()->vartable, e);
        id = IDS_NEXT(id);
    }

    vartype ty = {PARAM_TY(node), dims};
    vartable_entry e = {ID_VAL(PARAM_ID(node)), ty, true};
    vartable_insert(DATA_AR_GET()->vartable, e);

    return node;
}

node_st *ARfundecl(node_st *node) {
    enum BasicType prev = DATA_AR_GET()->ret_ty;
    DATA_AR_GET()->ret_ty = FUNDECL_TY(node);
    DATA_AR_GET()->vartable = vartable_new(DATA_AR_GET()->vartable);

    TRAVchildren(node);
    FUNDECL_VARTABLE(node) = DATA_AR_GET()->vartable;

    DATA_AR_GET()->ret_ty = prev;
    DATA_AR_GET()->vartable = DATA_AR_GET()->vartable->parent;

    return node;
}

node_st *ARfunbody(node_st *node) {
    DATA_AR_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AR_GET()->funtable = DATA_AR_GET()->funtable->parent;

    return node;
}

node_st *ARfor(node_st *node) {
    TRAVloop_start(node);
    TRAVloop_end(node);
    TRAVloop_step(node);

    vartype ty = {TY_int, 0};
    vartable_entry e = {ID_VAL(FOR_ID(node)), ty, true};
    vartable_push(DATA_AR_GET()->vartable, e);

    int idx = DATA_AR_GET()->vartable->len - 1;

    TRAVstmts(node);

    DATA_AR_GET()->vartable->buf[idx].valid = false;

    return node;
}

node_st *ARassign(node_st *node) {
    TRAVchildren(node);

    ARcheckassign(RESOLVED_TY(ASSIGN_EXPR(node)),
                  RESOLVED_TY(ASSIGN_REF(node)));

    return node;
}

node_st *ARreturn(node_st *node) {
    TRAVchildren(node);

    if (RETURN_EXPR(node)) {
        vartype resolved_ty = RESOLVED_TY(RETURN_EXPR(node));
        if (resolved_ty.dims != 0) {
            CTI(CTI_ERROR, true,
                "can't return %d-dimensional array of '%s' from function "
                "returning value of "
                "'%s'",
                resolved_ty.dims, fmt_BasicType(resolved_ty.ty),
                fmt_BasicType(DATA_AR_GET()->ret_ty));
        } else if (DATA_AR_GET()->ret_ty != resolved_ty.ty) {
            CTI(CTI_ERROR, true,
                "can't return value of '%s' from function returning value of "
                "'%s'",
                fmt_BasicType(resolved_ty.ty),
                fmt_BasicType(DATA_AR_GET()->ret_ty));
        }
    } else {
        if (DATA_AR_GET()->ret_ty != TY_void) {
            CTI(CTI_ERROR, true,
                "can't return from function returning type '%s' without "
                "providing a value of that type",
                fmt_BasicType(DATA_AR_GET()->ret_ty));
        }
    }
    CTIabortOnError();

    return node;
}

node_st *ARarrexprs(node_st *node) {
    TRAVchildren(node);

    vartype ty = RESOLVED_TY(ARREXPRS_EXPR(node));
    ty.dims++;

    node_st *inner = ARREXPRS_NEXT(node);
    while (inner) {
        vartype resolved_ty = RESOLVED_TY(ARREXPRS_EXPR(inner));
        if (ty.dims != resolved_ty.dims) {
            CTI(CTI_ERROR, true,
                "encountered inconsistent dimensions in array expression");
        } else if (ty.ty != resolved_ty.ty) {
            CTI(CTI_ERROR, true,
                "encountered inconsistent types in array expression");
        }
        CTIabortOnError();
    }

    ARREXPRS_RESOLVED_TY(node) = ty.ty;
    ARREXPRS_RESOLVED_DIMS(node) = ty.dims;

    return node;
}

node_st *ARmonop(node_st *node) {
    TRAVchildren(node);

    vartype resolved_ty = RESOLVED_TY(MONOP_EXPR(node));

    if (resolved_ty.dims != 0) {
        CTI(CTI_ERROR, true,
            "can't apply monop '%s' to %d-dimensional array of type '%s'",
            fmt_MonOpKind(MONOP_OP(node)), resolved_ty.dims,
            fmt_BasicType(resolved_ty.ty));
        CTIabortOnError();
    }

    switch (MONOP_OP(node)) {
    case MO_pos:
    case MO_neg:
        if (resolved_ty.ty != TY_int && resolved_ty.ty != TY_float) {
            CTI(CTI_ERROR, true, "can't apply monop '%s' to value of type '%s'",
                fmt_MonOpKind(MONOP_OP(node)), fmt_BasicType(resolved_ty.ty));
        }
        break;
    case MO_not:
        if (resolved_ty.ty != TY_bool) {
            CTI(CTI_ERROR, true, "can't apply monop '%s' to value of type '%s'",
                fmt_MonOpKind(MONOP_OP(node)), fmt_BasicType(resolved_ty.ty));
        }
        break;
    default:
        DBUG_ASSERT(false, "Unknown monop detected.");
        break;
    }
    CTIabortOnError();

    MONOP_RESOLVED_TY(node) = resolved_ty.ty;
    MONOP_RESOLVED_DIMS(node) = resolved_ty.dims;

    return node;
}

node_st *ARbinop(node_st *node) {
    TRAVchildren(node);

    vartype left_ty = RESOLVED_TY(BINOP_LEFT(node));
    vartype right_ty = RESOLVED_TY(BINOP_RIGHT(node));

    if (left_ty.dims != 0) {
        CTI(CTI_ERROR, true,
            "can't apply binop '%s' to %d-dimensional array of type '%s'",
            fmt_BinOpKind(BINOP_OP(node)), left_ty.dims,
            fmt_BasicType(left_ty.ty));
    } else if (right_ty.dims != 0) {
        CTI(CTI_ERROR, true,
            "can't apply binop '%s' to %d-dimensional array of type '%s'",
            fmt_BinOpKind(BINOP_OP(node)), right_ty.dims,
            fmt_BasicType(right_ty.ty));
    } else if (left_ty.ty != right_ty.ty) {
        CTI(CTI_ERROR, true,
            "can't apply binop '%s' to nonequal types '%s' and '%s'",
            fmt_BinOpKind(BINOP_OP(node)), fmt_BasicType(left_ty.ty),
            fmt_BasicType(right_ty.ty));
    }
    CTIabortOnError();

    enum BasicType ty = left_ty.ty;
    switch (BINOP_OP(node)) {
    case BO_lt:
    case BO_le:
    case BO_gt:
    case BO_ge:
        ty = TY_bool;
        // fallthrough
    case BO_sub:
    case BO_div:
    case BO_mod:
        if (left_ty.ty != TY_int && left_ty.ty != TY_float) {
            CTI(CTI_ERROR, true, "can't apply binop '%s' to value of type '%s'",
                fmt_BinOpKind(BINOP_OP(node)), fmt_BasicType(left_ty.ty));
        }
        break;
    case BO_eq:
    case BO_ne:
        ty = TY_bool;
        // fallthrough
    case BO_add:
    case BO_mul:
        if (left_ty.ty != TY_int && left_ty.ty != TY_float &&
            left_ty.ty != TY_bool) {
            CTI(CTI_ERROR, true, "can't apply binop '%s' to value of type '%s'",
                fmt_BinOpKind(BINOP_OP(node)), fmt_BasicType(left_ty.ty));
        }
        break;
    case BO_and:
    case BO_or:
        ty = TY_bool;
        if (left_ty.ty != TY_bool) {
            CTI(CTI_ERROR, true, "can't apply binop '%s' to value of type '%s'",
                fmt_BinOpKind(BINOP_OP(node)), fmt_BasicType(left_ty.ty));
        }
        break;
    default:
        DBUG_ASSERT(false, "Unknown binop detected.");
        break;
    }
    CTIabortOnError();

    BINOP_RESOLVED_TY(node) = ty;
    BINOP_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ARcast(node_st *node) {
    TRAVchildren(node);

    vartype resolved_ty = RESOLVED_TY(CAST_EXPR(node));

    if (resolved_ty.dims != 0) {
        CTI(CTI_ERROR, true,
            "can't cast %d-dimensional array of type '%s' to value of type "
            "'%s'",
            resolved_ty.dims, fmt_BasicType(resolved_ty.ty),
            fmt_BasicType(CAST_TY(node)));
    } else if (resolved_ty.ty != TY_int && resolved_ty.ty != TY_float &&
               resolved_ty.ty != TY_bool) {
        CTI(CTI_ERROR, true,
            "can't cast value of type '%s' to value of type '%s'",
            fmt_BasicType(resolved_ty.ty), fmt_BasicType(CAST_TY(node)));
    }
    CTIabortOnError();

    CAST_RESOLVED_TY(node) = CAST_TY(node);
    CAST_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ARcall(node_st *node) {
    TRAVchildren(node);

    node_st *arg = CALL_EXPRS(node);
    int param_count = 0;
    while (arg) {
        param_count++;
        arg = EXPRS_NEXT(arg);
    }

    funtable_ref r = funtable_resolve(DATA_AR_GET()->funtable,
                                      ID_VAL(CALL_ID(node)), param_count);
    CALL_N(node) = r.n;
    CALL_L(node) = r.l;

    funtype e = funtable_get(DATA_AR_GET()->funtable, r).ty;

    arg = CALL_EXPRS(node);
    for (int i = 0; i < param_count; i++) {
        vartype expected_ty = e.buf[i];
        vartype resolved_ty = RESOLVED_TY(EXPRS_EXPR(arg));

        if (expected_ty.dims != resolved_ty.dims ||
            expected_ty.ty != resolved_ty.ty) {
            if (expected_ty.dims == 0) {
                if (resolved_ty.dims == 0) {
                    CTI(CTI_ERROR, true,
                        "expected value of type '%s', found value of type '%s'",
                        fmt_BasicType(expected_ty.ty),
                        fmt_BasicType(resolved_ty.ty));
                } else {
                    CTI(CTI_ERROR, true,
                        "expected value of type '%s', found %d-dimensional "
                        "array of type '%s'",
                        fmt_BasicType(expected_ty.ty), resolved_ty.dims,
                        fmt_BasicType(resolved_ty.ty));
                }
            } else {
                if (resolved_ty.dims == 0) {
                    CTI(CTI_ERROR, true,
                        "expected %d-dimensional array of type '%s', found "
                        "value of type '%s'",
                        expected_ty.dims, fmt_BasicType(expected_ty.ty),
                        fmt_BasicType(resolved_ty.ty));
                } else {
                    CTI(CTI_ERROR, true,
                        "expected %d-dimensional array of type '%s', found "
                        "%d-dimensional array of type '%s'",
                        expected_ty.dims, fmt_BasicType(expected_ty.ty),
                        resolved_ty.dims, fmt_BasicType(resolved_ty.ty));
                }
            }
            CTIabortOnError();
        }

        arg = EXPRS_NEXT(arg);
    }

    CALL_RESOLVED_TY(node) = e.ret_ty;
    CALL_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ARvarref(node_st *node) {
    TRAVchildren(node);

    vartable_ref r =
        vartable_resolve(DATA_AR_GET()->vartable, ID_VAL(VARREF_ID(node)));
    VARREF_N(node) = r.n;
    VARREF_L(node) = r.l;

    vartype ty = vartable_get(DATA_AR_GET()->vartable, r).ty;

    int count = 0;
    node_st *expr = VARREF_EXPRS(node);
    while (expr) {
        count++;
        expr = EXPRS_NEXT(expr);
    }

    if (count > ty.dims) {
        if (ty.dims > 0) {
            CTI(CTI_ERROR, true,
                "can't index %d times into %d-dimensional array of type '%s'",
                count, ty.dims, fmt_BasicType(ty.ty));
        } else {
            CTI(CTI_ERROR, true, "can't index into value of type '%s'",
                fmt_BasicType(ty.ty));
        }
        CTIabortOnError();
    }

    ty.dims -= count;

    VARREF_RESOLVED_TY(node) = ty.ty;
    VARREF_RESOLVED_DIMS(node) = ty.dims;

    return node;
}

node_st *ARint(node_st *node) {
    INT_RESOLVED_TY(node) = TY_int;
    INT_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ARfloat(node_st *node) {
    FLOAT_RESOLVED_TY(node) = TY_float;
    FLOAT_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ARbool(node_st *node) {
    BOOL_RESOLVED_TY(node) = TY_bool;
    BOOL_RESOLVED_DIMS(node) = 0;

    return node;
}

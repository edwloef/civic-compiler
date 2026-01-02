#include "ccn/ccn.h"
#include "palm/ctinfo.h"
#include "palm/dbug.h"
#include "print/print.h"

vartype RESOLVED_TY(node_st *node) {
    vartype ty = {ARREXPR_RESOLVED_TY(node), ARREXPR_RESOLVED_DIMS(node)};
    return ty;
}

void ATCcheckassign(vartype from, vartype to) {
    if (to.dims != 0 && from.dims != to.dims) {
        if (from.dims == 0) {
            if (from.ty != to.ty) {
                CTI(CTI_ERROR, true,
                    "can't spread value of type '%s' into %d-dimensional array "
                    "of type '%s'",
                    fmt_BasicType(from.ty), to.dims, fmt_BasicType(to.ty));
            }
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
                "%d-dimensional array of type '%s'",
                from.dims, fmt_BasicType(from.ty), to.dims,
                fmt_BasicType(to.ty));
        }
    }
}

void ATCinit(void) {}

void ATCfini(void) { CTIabortOnError(); }

node_st *ATCprogram(node_st *node) {
    DATA_ATC_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_ATC_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *ATCvardecl(node_st *node) {
    TRAVchildren(node);

    if (VARDECL_EXPR(node)) {
        int dims = 0;
        node_st *expr = TYPE_EXPRS(VARDECL_TY(node));
        while (expr) {
            dims++;
            expr = EXPRS_NEXT(expr);
        }

        vartype ty = {TYPE_TY(VARDECL_TY(node)), dims};

        ATCcheckassign(RESOLVED_TY(VARDECL_EXPR(node)), ty);
    }

    return node;
}

node_st *ATCfundecl(node_st *node) {
    enum BasicType prev = DATA_ATC_GET()->ret_ty;
    DATA_ATC_GET()->ret_ty = FUNDECL_TY(node);
    DATA_ATC_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_ATC_GET()->ret_ty = prev;
    DATA_ATC_GET()->vartable = DATA_ATC_GET()->vartable->parent;

    return node;
}

node_st *ATCfunbody(node_st *node) {
    DATA_ATC_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_ATC_GET()->funtable = DATA_ATC_GET()->funtable->parent;

    return node;
}

node_st *ATCassign(node_st *node) {
    TRAVchildren(node);

    ATCcheckassign(RESOLVED_TY(ASSIGN_EXPR(node)),
                   RESOLVED_TY(ASSIGN_REF(node)));

    return node;
}

node_st *ATCreturn(node_st *node) {
    TRAVchildren(node);

    if (RETURN_EXPR(node)) {
        vartype resolved_ty = RESOLVED_TY(RETURN_EXPR(node));

        if (resolved_ty.dims != 0) {
            CTI(CTI_ERROR, true,
                "can't return %d-dimensional array of type '%s' from function "
                "returning value of type '%s'",
                resolved_ty.dims, fmt_BasicType(resolved_ty.ty),
                fmt_BasicType(DATA_ATC_GET()->ret_ty));
        } else if (DATA_ATC_GET()->ret_ty != resolved_ty.ty) {
            CTI(CTI_ERROR, true,
                "can't return value of type '%s' from function returning value "
                "of type '%s'",
                fmt_BasicType(resolved_ty.ty),
                fmt_BasicType(DATA_ATC_GET()->ret_ty));
        }
    } else {
        if (DATA_ATC_GET()->ret_ty != TY_void) {
            CTI(CTI_ERROR, true,
                "can't return from function returning type '%s' without "
                "providing a value of that type",
                fmt_BasicType(DATA_ATC_GET()->ret_ty));
        }
    }

    return node;
}

node_st *ATCarrexprs(node_st *node) {
    TRAVchildren(node);

    vartype ty = RESOLVED_TY(ARREXPRS_EXPR(node));
    node_st *inner = ARREXPRS_NEXT(node);
    while (inner) {
        vartype resolved_ty = RESOLVED_TY(ARREXPRS_EXPR(inner));

        if (ty.dims != resolved_ty.dims || ty.ty != resolved_ty.ty) {
            if (ty.dims == 0) {
                if (resolved_ty.dims == 0) {
                    CTI(CTI_ERROR, true,
                        "encountered inconsistent array expression containing "
                        "value of type '%s' and value of type '%s'",
                        fmt_BasicType(ty.ty), fmt_BasicType(resolved_ty.ty));
                } else {
                    CTI(CTI_ERROR, true,
                        "encountered inconsistent array expression containing "
                        "value of type '%s' and %d-dimensional array of type "
                        "'%s'",
                        fmt_BasicType(ty.ty), resolved_ty.dims,
                        fmt_BasicType(resolved_ty.ty));
                }
            } else {
                if (resolved_ty.dims == 0) {
                    CTI(CTI_ERROR, true,
                        "encountered inconsistent array expression containing "
                        "%d-dimensional array of type '%s' and value of type "
                        "'%s'",
                        ty.dims, fmt_BasicType(ty.ty),
                        fmt_BasicType(resolved_ty.ty));
                } else {
                    CTI(CTI_ERROR, true,
                        "encountered inconsistent array expression containing "
                        "%d-dimensional array of type '%s' and %d-dimensional "
                        "array of type '%s'",
                        ty.dims, fmt_BasicType(ty.ty), resolved_ty.dims,
                        fmt_BasicType(resolved_ty.ty));
                }
            }
        }

        inner = ARREXPRS_NEXT(inner);
    }

    ARREXPRS_RESOLVED_TY(node) = ty.ty;
    ARREXPRS_RESOLVED_DIMS(node) = ty.dims + 1;

    return node;
}

node_st *ATCmonop(node_st *node) {
    TRAVchildren(node);

    vartype resolved_ty = RESOLVED_TY(MONOP_EXPR(node));

    if (resolved_ty.dims != 0) {
        CTI(CTI_ERROR, true,
            "can't apply monop '%s' to %d-dimensional array of type '%s'",
            fmt_MonOpKind(MONOP_OP(node)), resolved_ty.dims,
            fmt_BasicType(resolved_ty.ty));
    }
    CTIabortOnError();

    enum BasicType ty = resolved_ty.ty;
    switch (MONOP_OP(node)) {
    case MO_pos:
    case MO_neg:
        if (ty != TY_int && ty != TY_float) {
            CTI(CTI_ERROR, true, "can't apply monop '%s' to value of type '%s'",
                fmt_MonOpKind(MONOP_OP(node)), fmt_BasicType(ty));
        }
        break;
    case MO_not:
        if (ty != TY_bool) {
            CTI(CTI_ERROR, true, "can't apply monop '%s' to value of type '%s'",
                fmt_MonOpKind(MONOP_OP(node)), fmt_BasicType(ty));
        }
        break;
    default:
        DBUG_ASSERT(false, "Unknown monop detected.");
        break;
    }
    CTIabortOnError();

    MONOP_RESOLVED_TY(node) = ty;
    MONOP_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ATCbinop(node_st *node) {
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
            "can't apply binop '%s' to values of nonequal types '%s' and '%s'",
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
            CTI(CTI_ERROR, true,
                "can't apply binop '%s' to values of type '%s'",
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
            CTI(CTI_ERROR, true,
                "can't apply binop '%s' to values of type '%s'",
                fmt_BinOpKind(BINOP_OP(node)), fmt_BasicType(left_ty.ty));
        }
        break;
    case BO_and:
    case BO_or:
        if (left_ty.ty != TY_bool) {
            CTI(CTI_ERROR, true,
                "can't apply binop '%s' to values of type '%s'",
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

node_st *ATCcast(node_st *node) {
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

    CAST_RESOLVED_TY(node) = CAST_TY(node);
    CAST_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ATCcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtype e = funtable_get(DATA_ATC_GET()->funtable, r).ty;

    node_st *arg = CALL_EXPRS(node);
    for (int i = 0; i < e.len; i++) {
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
        }

        arg = EXPRS_NEXT(arg);
    }

    CALL_RESOLVED_TY(node) = e.ret_ty;
    CALL_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ATCvarref(node_st *node) {
    TRAVchildren(node);

    vartable_ref r = {VARREF_N(node), VARREF_L(node)};
    vartype ty = vartable_get(DATA_ATC_GET()->vartable, r).ty;

    int count = 0;
    node_st *expr = VARREF_EXPRS(node);
    while (expr) {
        count++;

        if (ty.dims != 0) {
            vartype resolved_ty = RESOLVED_TY(EXPRS_EXPR(expr));
            if (resolved_ty.dims == 0) {
                if (resolved_ty.ty != TY_int) {
                    CTI(CTI_ERROR, true,
                        "can't index into %d-dimensional array of type '%s' "
                        "with value of type '%s'",
                        ty.dims, fmt_BasicType(ty.ty),
                        fmt_BasicType(resolved_ty.ty));
                }
            } else {
                CTI(CTI_ERROR, true,
                    "can't index into %d-dimensional array of type '%s' with "
                    "%d-dimensional array of type '%s'",
                    ty.dims, fmt_BasicType(ty.ty), resolved_ty.dims,
                    fmt_BasicType(resolved_ty.ty));
            }
        }

        expr = EXPRS_NEXT(expr);
    }

    if (count == 0) {
    } else if (count == ty.dims) {
        ty.dims = 0;
    } else if (ty.dims == 0) {
        CTI(CTI_ERROR, true, "can't index into value of type '%s'",
            fmt_BasicType(ty.ty));
    } else {
        CTI(CTI_ERROR, true,
            "can't index %d times into %d-dimensional array of type '%s'",
            count, ty.dims, fmt_BasicType(ty.ty));
        ty.dims = 0;
    }

    VARREF_RESOLVED_TY(node) = ty.ty;
    VARREF_RESOLVED_DIMS(node) = ty.dims;

    return node;
}

node_st *ATCint(node_st *node) {
    INT_RESOLVED_TY(node) = TY_int;
    INT_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ATCfloat(node_st *node) {
    FLOAT_RESOLVED_TY(node) = TY_float;
    FLOAT_RESOLVED_DIMS(node) = 0;

    return node;
}

node_st *ATCbool(node_st *node) {
    BOOL_RESOLVED_TY(node) = TY_bool;
    BOOL_RESOLVED_DIMS(node) = 0;

    return node;
}

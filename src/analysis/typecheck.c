#include "analysis/error.h"
#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "palm/ctinfo.h"
#include "palm/dbug.h"
#include "print/print.h"

static vartype RESOLVED_TY(node_st *node) {
    vartype ty = {ARREXPR_RESOLVED_TY(node), ARREXPR_RESOLVED_DIMS(node)};
    return ty;
}

void ATCcheckassign(vartype from, vartype to, node_st *node) {
    if (from.ty == TY_error) {
        return;
    }

    if (to.dims != 0 && from.dims != to.dims) {
        if (from.dims == 0) {
            if (from.ty != to.ty) {
                ERROR(
                    node,
                    "can't spread value of type '%s' into %d-dimensional array "
                    "of type '%s'",
                    fmt_BasicType(from.ty), to.dims, fmt_BasicType(to.ty));
            }
        } else {
            ERROR(node,
                  "can't assign %d-dimensional array of type '%s' to "
                  "%d-dimensional array of type '%s'",
                  from.dims, fmt_BasicType(from.ty), to.dims,
                  fmt_BasicType(to.ty));
        }
    } else if (from.ty != to.ty) {
        if (from.dims == 0) {
            if (to.dims == 0) {
                ERROR(node,
                      "can't assign value of type '%s' to value of type '%s'",
                      fmt_BasicType(from.ty), fmt_BasicType(to.ty));
            } else {
                ERROR(
                    node,
                    "can't spread value of type '%s' into %d-dimensional array "
                    "of type '%s'",
                    fmt_BasicType(from.ty), to.dims, fmt_BasicType(to.ty));
            }
        } else {
            ERROR(node,
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

node_st *ATCstmts(node_st *node) {
    TRAVchildren(node);

    STMTS_ALWAYS_RETURNS(node) =
        NODE_TYPE(STMTS_STMT(node)) == NT_RETURN ||
        (NODE_TYPE(STMTS_STMT(node)) == NT_IFELSE &&
         IFELSE_ALWAYS_RETURNS(STMTS_STMT(node))) ||
        (STMTS_NEXT(node) != NULL && STMTS_ALWAYS_RETURNS(STMTS_NEXT(node)));

    return node;
}

node_st *ATCarrexprs(node_st *node) {
    bool error = false;
    vartype self_ty = {TY_void, 0};

    for (node_st *inner = node; inner; inner = ARREXPRS_NEXT(inner)) {
        TRAVexpr(inner);
        vartype resolved_ty = RESOLVED_TY(ARREXPRS_EXPR(inner));

        if (resolved_ty.ty == TY_error) {
            error = true;
            continue;
        }

        if (self_ty.ty == TY_void) {
            self_ty = resolved_ty;
            continue;
        }

        if (self_ty.dims == resolved_ty.dims && self_ty.ty == resolved_ty.ty) {
            continue;
        }

        if (self_ty.dims == 0) {
            if (resolved_ty.dims == 0) {
                ERROR(ARREXPRS_EXPR(inner),
                      "encountered inconsistent array expression containing "
                      "value of type '%s' and value of type '%s'",
                      fmt_BasicType(self_ty.ty), fmt_BasicType(resolved_ty.ty));
            } else {
                ERROR(ARREXPRS_EXPR(inner),
                      "encountered inconsistent array expression containing "
                      "value of type '%s' and %d-dimensional array of type "
                      "'%s'",
                      fmt_BasicType(self_ty.ty), resolved_ty.dims,
                      fmt_BasicType(resolved_ty.ty));
            }
        } else {
            if (resolved_ty.dims == 0) {
                ERROR(ARREXPRS_EXPR(inner),
                      "encountered inconsistent array expression containing "
                      "%d-dimensional array of type '%s' and value of type "
                      "'%s'",
                      self_ty.dims, fmt_BasicType(self_ty.ty),
                      fmt_BasicType(resolved_ty.ty));
            } else {
                ERROR(ARREXPRS_EXPR(inner),
                      "encountered inconsistent array expression containing "
                      "%d-dimensional array of type '%s' and %d-dimensional "
                      "array of type '%s'",
                      self_ty.dims, fmt_BasicType(self_ty.ty), resolved_ty.dims,
                      fmt_BasicType(resolved_ty.ty));
            }
        }

        error = true;
    }

    if (error) {
        ARREXPRS_RESOLVED_TY(node) = TY_error;
    } else {
        ARREXPRS_RESOLVED_TY(node) = self_ty.ty;
    }
    ARREXPRS_RESOLVED_DIMS(node) = self_ty.dims + 1;

    return node;
}

node_st *ATCfundecl(node_st *node) {
    enum BasicType prev = DATA_ATC_GET()->ret_ty;
    DATA_ATC_GET()->ret_ty = FUNDECL_TY(node);
    DATA_ATC_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_ATC_GET()->ret_ty = prev;
    DATA_ATC_GET()->vartable = DATA_ATC_GET()->vartable->parent;

    if (!FUNDECL_EXTERNAL(node) && FUNDECL_TY(node) != TY_void &&
        (!FUNDECL_BODY(node) || !FUNBODY_STMTS(FUNDECL_BODY(node)) ||
         !STMTS_ALWAYS_RETURNS(FUNBODY_STMTS(FUNDECL_BODY(node))))) {
        ERROR(
            FUNDECL_ID(node),
            "function '%s' returning value of type '%s' doesn't return in all "
            "paths",
            ID_VAL(FUNDECL_ID(node)), fmt_BasicType(FUNDECL_TY(node)));
    }

    return node;
}

node_st *ATCfunbody(node_st *node) {
    DATA_ATC_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_ATC_GET()->funtable = DATA_ATC_GET()->funtable->parent;

    return node;
}

node_st *ATCvardecl(node_st *node) {
    TRAVchildren(node);

    vartable_ref r = {0, VARDECL_L(node)};
    vartable_entry e = vartable_get(DATA_ATC_GET()->vartable, r);
    vartype ty = e.ty;

    for (node_st *expr = TYPE_EXPRS(VARDECL_TY(node)); expr;
         expr = EXPRS_NEXT(expr)) {
        vartype resolved_ty = RESOLVED_TY(EXPRS_EXPR(expr));
        if (resolved_ty.dims == 0) {
            if (resolved_ty.ty != TY_int) {
                ERROR(EXPRS_EXPR(expr),
                      "can't declare %d-dimensional array of type '%s' with "
                      "length of value of type '%s'",
                      ty.dims, fmt_BasicType(ty.ty),
                      fmt_BasicType(resolved_ty.ty));
            }
        } else {
            ERROR(EXPRS_EXPR(expr),
                  "can't declare %d-dimensional array of type '%s' with length "
                  "of %d-dimensional array of type '%s'",
                  ty.dims, fmt_BasicType(ty.ty), resolved_ty.dims,
                  fmt_BasicType(resolved_ty.ty));
        }
    }

    if (VARDECL_EXPR(node))
        ATCcheckassign(RESOLVED_TY(VARDECL_EXPR(node)), ty, node);

    return node;
}

node_st *ATCassign(node_st *node) {
    TRAVchildren(node);

    vartable_ref r = {VARREF_N(ASSIGN_REF(node)), VARREF_L(ASSIGN_REF(node))};
    vartable_entry e = vartable_get(DATA_ATC_GET()->vartable, r);

    if (e.loopvar)
        ERROR(node, "can't assign to loop variable");

    ATCcheckassign(RESOLVED_TY(ASSIGN_EXPR(node)),
                   RESOLVED_TY(ASSIGN_REF(node)), node);

    return node;
}

node_st *ATCifelse(node_st *node) {
    TRAVchildren(node);

    vartype resolved_ty = RESOLVED_TY(IFELSE_EXPR(node));
    if (resolved_ty.ty != TY_bool) {
        if (resolved_ty.dims == 0) {
            ERROR(
                IFELSE_EXPR(node),
                "expected value of type 'bool' as if-condition, found value of "
                "type '%s'",
                fmt_BasicType(resolved_ty.ty));
        } else {
            ERROR(IFELSE_EXPR(node),
                  "expected value of type 'bool' as if-condition, found "
                  "%d-dimensional array of type '%s'",
                  resolved_ty.dims, fmt_BasicType(resolved_ty.ty));
        }
    }

    IFELSE_ALWAYS_RETURNS(node) = IFELSE_IF_BLOCK(node) != NULL &&
                                  STMTS_ALWAYS_RETURNS(IFELSE_IF_BLOCK(node)) &&
                                  IFELSE_ELSE_BLOCK(node) != NULL &&
                                  STMTS_ALWAYS_RETURNS(IFELSE_ELSE_BLOCK(node));

    return node;
}

node_st *ATCwhile(node_st *node) {
    TRAVchildren(node);

    vartype resolved_ty = RESOLVED_TY(WHILE_EXPR(node));
    if (resolved_ty.ty != TY_error) {
        if (resolved_ty.ty != TY_bool) {
            if (resolved_ty.dims == 0) {
                ERROR(WHILE_EXPR(node),
                      "expected value of type 'bool' as while-condition, found "
                      "value "
                      "of type '%s'",
                      fmt_BasicType(resolved_ty.ty));
            } else {
                ERROR(WHILE_EXPR(node),
                      "expected value of type 'bool' as while-condition, found "
                      "%d-dimensional array of type '%s'",
                      resolved_ty.dims, fmt_BasicType(resolved_ty.ty));
            }
        }
    }

    return node;
}

node_st *ATCdowhile(node_st *node) {
    TRAVchildren(node);

    vartype resolved_ty = RESOLVED_TY(DOWHILE_EXPR(node));
    if (resolved_ty.ty != TY_error) {
        if (resolved_ty.ty != TY_bool) {
            if (resolved_ty.dims == 0) {
                ERROR(DOWHILE_EXPR(node),
                      "expected value of type 'bool' as do-while-condition, "
                      "found value of type '%s'",
                      fmt_BasicType(resolved_ty.ty));
            } else {
                ERROR(DOWHILE_EXPR(node),
                      "expected value of type 'bool' as do-while-condition, "
                      "found %d-dimensional array of type '%s'",
                      resolved_ty.dims, fmt_BasicType(resolved_ty.ty));
            }
        }
    }

    return node;
}

node_st *ATCfor(node_st *node) {
    TRAVchildren(node);

    vartype start_ty = RESOLVED_TY(FOR_LOOP_START(node));
    if (start_ty.ty != TY_error) {
        if (start_ty.ty != TY_int) {
            if (start_ty.dims == 0) {
                ERROR(FOR_LOOP_START(node),
                      "expected value of type 'int' as for-loop start value, "
                      "found value of type '%s'",
                      fmt_BasicType(start_ty.ty));
            } else {
                ERROR(FOR_LOOP_START(node),
                      "expected value of type 'int' as for-loop start value, "
                      "found %d-dimensional array of type '%s'",
                      start_ty.dims, fmt_BasicType(start_ty.ty));
            }
        }
    }

    vartype end_ty = RESOLVED_TY(FOR_LOOP_END(node));
    if (end_ty.ty != TY_error) {
        if (end_ty.ty != TY_int) {
            if (end_ty.dims == 0) {
                ERROR(FOR_LOOP_END(node),
                      "expected value of type 'int' as for-loop end value, "
                      "found value of type '%s'",
                      fmt_BasicType(end_ty.ty));
            } else {
                ERROR(FOR_LOOP_END(node),
                      "expected value of type 'int' as for-loop end value, "
                      "found %d-dimensional array of type '%s'",
                      end_ty.dims, fmt_BasicType(end_ty.ty));
            }
        }
    }

    vartype step_ty = RESOLVED_TY(FOR_LOOP_STEP(node));
    if (step_ty.ty != TY_error) {
        if (step_ty.ty != TY_int) {
            if (step_ty.dims == 0) {
                ERROR(FOR_LOOP_STEP(node),
                      "expected value of type 'int' as for-loop step value, "
                      "found value of type '%s'",
                      fmt_BasicType(step_ty.ty));
            } else {
                ERROR(FOR_LOOP_STEP(node),
                      "expected value of type 'int' as for-loop step value, "
                      "found %d-dimensional array of type '%s'",
                      step_ty.dims, fmt_BasicType(step_ty.ty));
            }
        }
    }

    return node;
}

node_st *ATCreturn(node_st *node) {
    TRAVchildren(node);

    if (RETURN_EXPR(node)) {
        vartype resolved_ty = RESOLVED_TY(RETURN_EXPR(node));
        if (DATA_ATC_GET()->ret_ty == TY_void) {
            ERROR(node,
                  "can't return value from function without return value");
        } else if (resolved_ty.ty != TY_error) {
            if (resolved_ty.dims != 0) {
                ERROR(node,
                      "can't return %d-dimensional array of type '%s' from "
                      "function returning value of type '%s'",
                      resolved_ty.dims, fmt_BasicType(resolved_ty.ty),
                      fmt_BasicType(DATA_ATC_GET()->ret_ty));
            } else if (DATA_ATC_GET()->ret_ty != resolved_ty.ty) {
                ERROR(node,
                      "can't return value of type '%s' from function returning "
                      "value of type '%s'",
                      fmt_BasicType(resolved_ty.ty),
                      fmt_BasicType(DATA_ATC_GET()->ret_ty));
            }
        }
    } else {
        if (DATA_ATC_GET()->ret_ty != TY_void) {
            ERROR(node,
                  "can't return from function returning type '%s' without "
                  "providing a value of that type",
                  fmt_BasicType(DATA_ATC_GET()->ret_ty));
        }
    }

    return node;
}

node_st *ATCmonop(node_st *node) {
    TRAVchildren(node);

    vartype resolved_ty = RESOLVED_TY(MONOP_EXPR(node));

    MONOP_RESOLVED_DIMS(node) = 0;

    if (resolved_ty.ty == TY_error) {
        MONOP_RESOLVED_TY(node) = TY_error;
        return node;
    } else if (resolved_ty.dims != 0) {
        ERROR(node,
              "can't apply monop '%s' to %d-dimensional array of type '%s'",
              fmt_MonOpKind(MONOP_OP(node)), resolved_ty.dims,
              fmt_BasicType(resolved_ty.ty));
        MONOP_RESOLVED_TY(node) = TY_error;
        return node;
    }

    enum BasicType ty = resolved_ty.ty;
    switch (MONOP_OP(node)) {
    case MO_pos:
    case MO_neg:
        if (ty != TY_int && ty != TY_float) {
            ERROR(node, "can't apply monop '%s' to value of type '%s'",
                  fmt_MonOpKind(MONOP_OP(node)), fmt_BasicType(ty));
            ty = TY_error;
        }
        break;
    case MO_not:
        if (ty != TY_bool) {
            ERROR(node, "can't apply monop '%s' to value of type '%s'",
                  fmt_MonOpKind(MONOP_OP(node)), fmt_BasicType(ty));
            ty = TY_bool;
        }
        break;
    default:
        DBUG_ASSERT(false, "Unknown monop detected.");
        break;
    }
    MONOP_RESOLVED_TY(node) = ty;

    return node;
}

node_st *ATCbinop(node_st *node) {
    TRAVchildren(node);

    vartype left_ty = RESOLVED_TY(BINOP_LEFT(node));
    vartype right_ty = RESOLVED_TY(BINOP_RIGHT(node));

    BINOP_RESOLVED_DIMS(node) = 0;

    if (left_ty.ty == TY_error || right_ty.ty == TY_error) {
        BINOP_RESOLVED_TY(node) = TY_error;
    } else if (left_ty.dims != 0) {
        CTI(CTI_ERROR, true,
            "can't apply binop '%s' to %d-dimensional array of type '%s'",
            fmt_BinOpKind(BINOP_OP(node)), left_ty.dims,
            fmt_BasicType(left_ty.ty));
        BINOP_RESOLVED_TY(node) = TY_error;
    } else if (right_ty.dims != 0) {
        CTI(CTI_ERROR, true,
            "can't apply binop '%s' to %d-dimensional array of type '%s'",
            fmt_BinOpKind(BINOP_OP(node)), right_ty.dims,
            fmt_BasicType(right_ty.ty));
        BINOP_RESOLVED_TY(node) = TY_error;
    } else if (left_ty.ty != right_ty.ty) {
        CTI(CTI_ERROR, true,
            "can't apply binop '%s' to values of nonequal types '%s' and '%s'",
            fmt_BinOpKind(BINOP_OP(node)), fmt_BasicType(left_ty.ty),
            fmt_BasicType(right_ty.ty));
        BINOP_RESOLVED_TY(node) = TY_error;
    }

    if (BINOP_RESOLVED_TY(node) == TY_error) {
        return node;
    }

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
        if (left_ty.ty != TY_int && left_ty.ty != TY_float) {
            CTI(CTI_ERROR, true,
                "can't apply binop '%s' to values of type '%s'",
                fmt_BinOpKind(BINOP_OP(node)), fmt_BasicType(left_ty.ty));
            ty = TY_error;
        }
        break;
    case BO_mod:
        if (left_ty.ty != TY_int) {
            CTI(CTI_ERROR, true,
                "can't apply binop '%s' to values of type '%s'",
                fmt_BinOpKind(BINOP_OP(node)), fmt_BasicType(left_ty.ty));
            ty = TY_error;
        }
        break;
    case BO_eq:
    case BO_ne:
        ty = TY_bool;
        // fallthrough
    case BO_add:
    case BO_mul:
        break;
    case BO_and:
    case BO_or:
        if (left_ty.ty != TY_bool) {
            CTI(CTI_ERROR, true,
                "can't apply binop '%s' to values of type '%s'",
                fmt_BinOpKind(BINOP_OP(node)), fmt_BasicType(left_ty.ty));
            ty = TY_error;
        }
        break;
    default:
        DBUG_ASSERT(false, "Unknown binop detected.");
        break;
    }

    BINOP_RESOLVED_TY(node) = ty;

    return node;
}

node_st *ATCcast(node_st *node) {
    TRAVchildren(node);

    vartype resolved_ty = RESOLVED_TY(CAST_EXPR(node));

    if (resolved_ty.ty != TY_error) {
        if (resolved_ty.dims != 0) {
            ERROR(node,
                  "can't cast %d-dimensional array of type '%s' to value of "
                  "type '%s'",
                  resolved_ty.dims, fmt_BasicType(resolved_ty.ty),
                  fmt_BasicType(CAST_TY(node)));
        } else if (resolved_ty.ty != TY_int && resolved_ty.ty != TY_float &&
                   resolved_ty.ty != TY_bool) {
            ERROR(node, "can't cast value of type '%s' to value of type '%s'",
                  fmt_BasicType(resolved_ty.ty), fmt_BasicType(CAST_TY(node)));
        }
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
        if (resolved_ty.ty != TY_error) {
            if ((expected_ty.dims != resolved_ty.dims ||
                 expected_ty.ty != resolved_ty.ty)) {
                if (expected_ty.dims == 0) {
                    if (resolved_ty.dims == 0) {
                        ERROR(EXPRS_EXPR(arg),
                              "expected parameter of type '%s', found argument "
                              "of type '%s'",
                              fmt_BasicType(expected_ty.ty),
                              fmt_BasicType(resolved_ty.ty));
                    } else {
                        ERROR(EXPRS_EXPR(arg),
                              "expected parameter of type '%s', found "
                              "%d-dimensional array of type '%s'",
                              fmt_BasicType(expected_ty.ty), resolved_ty.dims,
                              fmt_BasicType(resolved_ty.ty));
                    }
                } else {
                    if (resolved_ty.dims == 0) {
                        ERROR(EXPRS_EXPR(arg),
                              "expected %d-dimensional array of argument '%s', "
                              "found argument of type '%s'",
                              expected_ty.dims, fmt_BasicType(expected_ty.ty),
                              fmt_BasicType(resolved_ty.ty));
                    } else {
                        ERROR(EXPRS_EXPR(arg),
                              "expected %d-dimensional array of type '%s', "
                              "found %d-dimensional array of type '%s'",
                              expected_ty.dims, fmt_BasicType(expected_ty.ty),
                              resolved_ty.dims, fmt_BasicType(resolved_ty.ty));
                    }
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

    for (node_st *expr = VARREF_EXPRS(node); expr; expr = EXPRS_NEXT(expr)) {
        if (ty.ty != TY_error && ty.dims != 0) {
            vartype resolved_ty = RESOLVED_TY(EXPRS_EXPR(expr));
            if (resolved_ty.ty != TY_error) {
                if (resolved_ty.dims == 0) {
                    if (resolved_ty.ty != TY_int) {
                        ERROR(EXPRS_EXPR(expr),
                              "can't index into %d-dimensional array of type "
                              "'%s' with value of type '%s'",
                              ty.dims, fmt_BasicType(ty.ty),
                              fmt_BasicType(resolved_ty.ty));
                    }
                } else {
                    ERROR(EXPRS_EXPR(node),
                          "can't index into %d-dimensional array of type '%s' "
                          "with %d-dimensional array of type '%s'",
                          ty.dims, fmt_BasicType(ty.ty), resolved_ty.dims,
                          fmt_BasicType(resolved_ty.ty));
                }
            }
        }
    }

    int index_count = 0;
    for (node_st *expr = VARREF_EXPRS(node); expr; expr = EXPRS_NEXT(expr)) {
        index_count++;
    }

    if (ty.ty == TY_error) {
    } else if (index_count <= ty.dims) {
        ty.dims -= index_count;
    } else if (ty.dims == 0) {
        ERROR(node, "can't index into value of type '%s'",
              fmt_BasicType(ty.ty));
    } else {
        ERROR(node,
              "can't index %d times into %d-dimensional array of type '%s'",
              index_count, ty.dims, fmt_BasicType(ty.ty));
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

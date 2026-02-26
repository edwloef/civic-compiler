#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "utils.h"

#define EMIT_INDENT() printf("%*s", DATA_PRT_GET()->indent * 4, "");
#define WITH_INDENT(e)                                                         \
    {                                                                          \
        DATA_PRT_GET()->indent++;                                              \
        e;                                                                     \
        DATA_PRT_GET()->indent--;                                              \
        EMIT_INDENT();                                                         \
    }

char *fmt_BasicType(enum BasicType ty) {
    switch (ty) {
    case TY_bool:
        return "bool";
    case TY_int:
        return "int";
    case TY_float:
        return "float";
    case TY_void:
        return "void";
    default:
        DBUG_ASSERT(false, "Unknown basic type detected.");
        return NULL;
    }
}

char *fmt_MonOpKind(enum MonOpKind mo) {
    switch (mo) {
    case MO_pos:
        return "+";
    case MO_neg:
        return "-";
    case MO_not:
        return "!";
    default:
        DBUG_ASSERT(false, "Unknown monop detected.");
        return NULL;
    }
}

char *fmt_BinOpKind(enum BinOpKind bo) {
    switch (bo) {
    case BO_add:
        return "+";
    case BO_sub:
        return "-";
    case BO_mul:
        return "*";
    case BO_div:
        return "/";
    case BO_mod:
        return "%";
    case BO_lt:
        return "<";
    case BO_le:
        return "<=";
    case BO_gt:
        return ">";
    case BO_ge:
        return ">=";
    case BO_eq:
        return "==";
    case BO_ne:
        return "!=";
    case BO_and:
        return "&&";
    case BO_or:
        return "||";
    default:
        DBUG_ASSERT(false, "Unknown binop detected.");
        return NULL;
    }
}

void PRTinit(void) {}
void PRTfini(void) {}

node_st *PRTscope(node_st *node) {
    OUT_OF_LIFETIME();
}

node_st *PRTprogram(node_st *node) {
    DATA_PRT_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_PRT_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *PRTdecls(node_st *node) {
    EMIT_INDENT();

    TRAVchildren(node);

    return node;
}

node_st *PRTstmts(node_st *node) {
    EMIT_INDENT();
    TRAVstmt(node);

    if (NODE_TYPE(STMTS_STMT(node)) == NT_CALL) {
        printf(";\n");
    }

    TRAVnext(node);

    return node;
}

node_st *PRTexprs(node_st *node) {
    TRAVexpr(node);

    if (EXPRS_NEXT(node)) {
        printf(", ");
        TRAVnext(node);
    }

    return node;
}

node_st *PRTarrexprs(node_st *node) {
    if (NODE_TYPE(ARREXPRS_EXPR(node)) == NT_ARREXPRS) {
        printf("[");
        TRAVexpr(node);
        printf("]");
    } else {
        TRAVexpr(node);
    }

    if (ARREXPRS_NEXT(node)) {
        printf(", ");
        TRAVnext(node);
    }

    return node;
}

node_st *PRTfundecl(node_st *node) {
    vartable *prev = DATA_PRT_GET()->vartable;
    DATA_PRT_GET()->vartable = FUNDECL_VARTABLE(node);

    if (FUNDECL_EXPORTED(node)) {
        printf("export ");
    }

    if (FUNDECL_EXTERNAL(node)) {
        printf("extern ");
    }

    printf("%s ", fmt_BasicType(FUNDECL_TY(node)));
    if (FUNDECL_ID(node)) {
        TRAVid(node);
    } else {
        funtable_ref r = {0, FUNDECL_L(node)};
        funtable_entry *e = funtable_get(DATA_PRT_GET()->funtable, r);
        printf("%s", e->name);
    }
    printf("(");
    TRAVparams(node);
    printf(")");

    if (FUNDECL_BODY(node)) {
        printf(" {\n");
        WITH_INDENT(TRAVbody(node));
        printf("}");
    } else {
        printf(";");
    }

    printf("\n");

    DATA_PRT_GET()->vartable = prev;

    return node;
}

node_st *PRTfunbody(node_st *node) {
    funtable *prev = DATA_PRT_GET()->funtable;
    DATA_PRT_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_PRT_GET()->funtable = prev;

    return node;
}

node_st *PRTvardecl(node_st *node) {
    if (VARDECL_EXPORTED(node)) {
        printf("export ");
    }

    if (VARDECL_EXTERNAL(node)) {
        printf("extern ");
    }

    TRAVty(node);
    printf(" ");
    if (VARDECL_ID(node)) {
        TRAVid(node);
    } else {
        vartable_ref r = {0, VARDECL_L(node)};
        vartable_entry *e = vartable_get(DATA_PRT_GET()->vartable, r);
        printf("%s", e->name);
    }

    if (VARDECL_EXPR(node)) {
        printf(" = ");
        if (NODE_TYPE(VARDECL_EXPR(node)) == NT_ARREXPRS) {
            printf("[");
            TRAVexpr(node);
            printf("]");
        } else {
            TRAVexpr(node);
        }
    }

    printf(";\n");

    return node;
}

node_st *PRTparams(node_st *node) {
    TRAVparam(node);

    if (PARAMS_NEXT(node)) {
        printf(", ");
        TRAVnext(node);
    }

    return node;
}

node_st *PRTparam(node_st *node) {
    TRAVty(node);
    printf(" ");
    if (PARAM_ID(node)) {
        TRAVid(node);
    } else {
        vartable_ref r = {0, PARAM_L(node)};
        vartable_entry *e = vartable_get(DATA_PRT_GET()->vartable, r);
        printf("%s", e->name);
    }

    return node;
}

node_st *PRTassign(node_st *node) {
    TRAVref(node);
    printf(" = ");

    if (NODE_TYPE(ASSIGN_EXPR(node)) == NT_ARREXPRS) {
        printf("[");
        TRAVexpr(node);
        printf("]");
    } else {
        TRAVexpr(node);
    }

    printf(";\n");

    return node;
}

node_st *PRTcall(node_st *node) {
    if (CALL_ID(node)) {
        TRAVid(node);
    } else {
        funtable_ref r = {CALL_N(node), CALL_L(node)};
        funtable_entry *e = funtable_get(DATA_PRT_GET()->funtable, r);
        printf("%s", e->name);
    }
    printf("(");
    TRAVexprs(node);
    printf(")");

    return node;
}

node_st *PRTifelse(node_st *node) {
    printf("if (");
    TRAVexpr(node);
    printf(") {\n");
    WITH_INDENT(TRAVif_block(node));
    printf("}");
    if (IFELSE_ELSE_BLOCK(node)) {
        printf(" else ");
        if (NODE_TYPE(STMTS_STMT(IFELSE_ELSE_BLOCK(node))) == NT_IFELSE &&
            !STMTS_NEXT(IFELSE_ELSE_BLOCK(node))) {
            TRAVstmt(IFELSE_ELSE_BLOCK(node));
        } else {
            printf("{\n");
            WITH_INDENT(TRAVelse_block(node));
            printf("}\n");
        }
    } else {
        printf("\n");
    }

    return node;
}

node_st *PRTwhile(node_st *node) {
    printf("while (");
    TRAVexpr(node);
    printf(") {\n");
    WITH_INDENT(TRAVstmts(node));
    printf("}\n");

    return node;
}

node_st *PRTdowhile(node_st *node) {
    printf("do {\n");
    WITH_INDENT(TRAVstmts(node));
    printf("} while (");
    TRAVexpr(node);
    printf(");\n");

    return node;
}

node_st *PRTfor(node_st *node) {
    printf("for (int ");
    TRAVref(node);
    printf(" = ");
    TRAVloop_start(node);
    printf(", ");
    TRAVloop_end(node);
    printf(", ");
    TRAVloop_step(node);
    printf(") {\n");
    WITH_INDENT(TRAVstmts(node));
    printf("}\n");

    return node;
}

node_st *PRTreturn(node_st *node) {
    printf("return");

    if (RETURN_EXPR(node)) {
        printf(" ");
        TRAVexpr(node);
    }

    printf(";\n");

    return node;
}

node_st *PRTbinop(node_st *node) {
    printf("(");
    TRAVleft(node);
    printf(" %s ", fmt_BinOpKind(BINOP_OP(node)));
    TRAVright(node);
    printf(")");

    return node;
}

node_st *PRTmonop(node_st *node) {
    printf("(%s", fmt_MonOpKind(MONOP_OP(node)));
    TRAVexpr(node);
    printf(")");

    return node;
}

node_st *PRTcast(node_st *node) {
    printf("((%s) ", fmt_BasicType(CAST_RESOLVED_TY(node)));
    TRAVexpr(node);
    printf(")");

    return node;
}

node_st *PRTmalloc(node_st *node) {
    printf("__malloc(");
    TRAVexprs(node);
    printf(")");
    return node;
}

node_st *PRTid(node_st *node) {
    printf("%s", ID_VAL(node));

    return node;
}

node_st *PRTtype(node_st *node) {
    printf("%s", fmt_BasicType(TYPE_TY(node)));

    if (TYPE_ARRAY(node)) {
        printf("[");
        TRAVexprs(node);
        printf("]");
    }

    return node;
}

node_st *PRTvarref(node_st *node) {
    if (VARREF_ID(node)) {
        TRAVid(node);
    } else {
        vartable_ref r = {VARREF_N(node), VARREF_L(node)};
        vartable_entry *e = vartable_get(DATA_PRT_GET()->vartable, r);
        printf("%s", e->name);
    }

    if (VARREF_EXPRS(node)) {
        printf("[");
        TRAVexprs(node);
        printf("]");
    }

    return node;
}

node_st *PRTint(node_st *node) {
    printf("%d", INT_VAL(node));

    return node;
}

node_st *PRTfloat(node_st *node) {
    printf("%lg", FLOAT_VAL(node));

    return node;
}

node_st *PRTbool(node_st *node) {
    printf("%s", BOOL_VAL(node) ? "true" : "false");

    return node;
}

node_st *PRTconstref(node_st *node) {
    printf("%sC%d", CONSTREF_RESOLVED_TY(node) == TY_int ? "I" : "F",
           CONSTREF_L(node));

    return node;
}

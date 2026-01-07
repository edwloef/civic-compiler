#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "palm/dbug.h"

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
    case TY_error:
        return "error";
    default:
        DBUG_ASSERT(false, "Unknown basic type detected.");
        return NULL;
    }
}

char *fmt_MonOpKind(enum MonOpKind bo) {
    switch (bo) {
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

node_st *PRTprogram(node_st *node) {
    TRAVchildren(node);
    return node;
}

node_st *PRTdecls(node_st *node) {
    TRAVchildren(node);
    return node;
}

node_st *PRTstmts(node_st *node) {
    TRAVchildren(node);
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
    } else
        TRAVexpr(node);
    if (ARREXPRS_NEXT(node)) {
        printf(", ");
        TRAVnext(node);
    }
    return node;
}

node_st *PRTfundecls(node_st *node) {
    TRAVchildren(node);
    return node;
}

node_st *PRTfundecl(node_st *node) {
    if (FUNDECL_EXPORTED(node))
        printf("export ");
    if (FUNDECL_EXTERNAL(node))
        printf("extern ");
    printf("%s ", fmt_BasicType(FUNDECL_TY(node)));
    TRAVid(node);
    printf("(");
    TRAVopt(FUNDECL_PARAMS(node));
    printf(")");
    if (FUNDECL_BODY(node)) {
        printf(" {\n");
        TRAVbody(node);
        printf("}");
    }
    printf("\n");
    return node;
}

node_st *PRTfunbody(node_st *node) {
    TRAVchildren(node);
    return node;
}

node_st *PRTvardecls(node_st *node) {
    TRAVchildren(node);
    return node;
}

node_st *PRTvardecl(node_st *node) {
    if (VARDECL_EXPORTED(node))
        printf("export ");
    TRAVty(node);
    printf(" ");
    TRAVid(node);
    if (VARDECL_EXPR(node)) {
        printf(" = ");
        if (NODE_TYPE(VARDECL_EXPR(node)) == NT_ARREXPRS) {
            printf("[");
            TRAVexpr(node);
            printf("]");
        } else
            TRAVexpr(node);
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
    printf("%s", fmt_BasicType(PARAM_TY(node)));
    if (PARAM_IDS(node)) {
        printf("[");
        TRAVids(node);
        printf("]");
    }
    printf(" ");
    TRAVid(node);
    return node;
}

node_st *PRTassign(node_st *node) {
    TRAVref(node);
    printf(" = ");
    if (NODE_TYPE(ASSIGN_EXPR(node)) == NT_ARREXPRS) {
        printf("[");
        TRAVexpr(node);
        printf("]");
    } else
        TRAVexpr(node);
    printf(";\n");
    return node;
}

node_st *PRTcall(node_st *node) {
    TRAVid(node);
    printf("(");
    TRAVexprs(node);
    printf(")");
    return node;
}

node_st *PRTifelse(node_st *node) {
    printf("if (");
    TRAVexpr(node);
    printf(") {\n");
    TRAVopt(IFELSE_IF_BLOCK(node));
    printf("} else {\n");
    TRAVopt(IFELSE_ELSE_BLOCK(node));
    printf("}\n");
    return node;
}

node_st *PRTwhile(node_st *node) {
    printf("while (");
    TRAVexpr(node);
    printf(") {\n");
    TRAVopt(WHILE_STMTS(node));
    printf("}\n");
    return node;
}

node_st *PRTdowhile(node_st *node) {
    printf("do {\n");
    TRAVopt(DOWHILE_STMTS(node));
    printf("} while (");
    TRAVexpr(node);
    printf(")\n");
    return node;
}

node_st *PRTfor(node_st *node) {
    printf("for (");
    TRAVid(node);
    printf(" = ");
    TRAVloop_start(node);
    printf(", ");
    TRAVloop_end(node);
    printf(", ");
    TRAVloop_step(node);
    printf(") {\n");
    TRAVopt(FOR_STMTS(node));
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
    printf("((%s) ", fmt_BasicType(CAST_TY(node)));
    TRAVexpr(node);
    printf(")");
    return node;
}

node_st *PRTids(node_st *node) {
    TRAVid(node);
    if (IDS_NEXT(node)) {
        printf(", ");
        TRAVnext(node);
    }
    return node;
}

node_st *PRTid(node_st *node) {
    printf("%s", ID_VAL(node));
    return node;
}

node_st *PRTtype(node_st *node) {
    printf("%s", fmt_BasicType(TYPE_TY(node)));
    if (TYPE_EXPRS(node)) {
        printf("[");
        TRAVexprs(node);
        printf("]");
    }
    return node;
}

node_st *PRTvarref(node_st *node) {
    TRAVid(node);
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
    printf("%f", FLOAT_VAL(node));
    return node;
}

node_st *PRTbool(node_st *node) {
    printf("%s", BOOL_VAL(node) ? "true" : "false");
    return node;
}

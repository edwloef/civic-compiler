#include "ccn/ccn.h"
#include "ccn/dynamic_core.h"
#include "palm/str.h"
#include "table/table.h"
#include "table/vartable.h"
#include <stdio.h>

void DADinit(void) {}
void DADfini(void) {}

node_st *DADprogram(node_st *node) {
    DATA_DAD_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    node_st *init = ASTfundecl(ASTid(STRcpy("__init")), NULL, TY_void);
    node_st *body = ASTfunbody(NULL, DATA_DAD_GET()->stmts);
    FUNBODY_FUNTABLE(body) = funtable_new(PROGRAM_FUNTABLE(node));
    FUNDECL_BODY(init) = body;
    FUNDECL_VARTABLE(init) = vartable_new(PROGRAM_VARTABLE(node));
    PROGRAM_DECLS(node) = ASTdecls(init, PROGRAM_DECLS(node));

    return node;
}

node_st *DADvarref(node_st *node) {
    if (DATA_DAD_GET()->vartable->parent == NULL) {
        VARREF_N(node)++;
    }
    return node;
}

node_st *DADcall(node_st *node) {
    if (DATA_DAD_GET()->vartable->parent == NULL) {
        CALL_N(node)++;
    }
    return node;
}

node_st *DADfunbody(node_st *node) {
    node_st *before_stmts = DATA_DAD_GET()->stmts;
    DATA_DAD_GET()->stmts = FUNBODY_STMTS(node);

    TRAVchildren(node);

    FUNBODY_STMTS(node) = DATA_DAD_GET()->stmts;
    DATA_DAD_GET()->stmts = before_stmts;

    return node;
}

node_st *gen_loop(node_st *exprs, node_st *id, node_st *loop_exprs,
                  node_st *value) {
    printf("<< ");
    TRAVstart(exprs, TRAV_PRT);
    printf(" >> [[ ");
    TRAVstart(loop_exprs, TRAV_PRT);
    printf(" ]] \n");

    if (exprs == NULL) {
        return ASTassign(ASTvarref(id, loop_exprs), value);
    }

    node_st *self = EXPRS_EXPR(exprs);
    loop_exprs = ASTexprs(self, loop_exprs);

    return gen_loop(EXPRS_NEXT(exprs), id, loop_exprs, value);
}

node_st *DADdecls(node_st *node) {
    TRAVchildren(node);

    node_st *decl = DECLS_DECL(node);
    if (NODE_TYPE(decl) != NT_VARDECL) {
        return node;
    }

    node_st *ty = VARDECL_TY(decl);
    enum BasicType basic_ty = TYPE_TY(ty);
    node_st *exprs = TYPE_EXPRS(ty);

    node_st *assign_val;
    if (exprs == NULL) {
        assign_val = VARDECL_EXPR(decl);
    } else {
        node_st *size = CCNcopy(EXPRS_EXPR(exprs));
        for (node_st *expr = EXPRS_NEXT(exprs); expr != NULL;
             expr = EXPRS_NEXT(expr)) {
            size = ASTbinop(size, CCNcopy(EXPRS_EXPR(expr)), BO_mul);
        }

        assign_val = ASTmalloc(size, basic_ty);
    }

    node_st *assign_var = ASTvarref(VARDECL_ID(decl), NULL);
    node_st *assign = ASTassign(assign_var, assign_val);
    DATA_DAD_GET()->stmts = ASTstmts(assign, DATA_DAD_GET()->stmts);

    if (exprs != NULL) {
        node_st *spread_ref =
            vartable_temp_var(DATA_DAD_GET()->vartable, basic_ty);
        node_st *spread = ASTassign(spread_ref, CCNcopy(VARDECL_EXPR(decl)));
        DATA_DAD_GET()->stmts = ASTstmts(spread, DATA_DAD_GET()->stmts);

        node_st *loop =
            gen_loop(exprs, VARREF_ID(spread_ref), NULL, assign_var);
        DATA_DAD_GET()->stmts = ASTstmts(loop, DATA_DAD_GET()->stmts);
    }

    // VARDECL_EXPR(decl) = NULL;

    //  VARDECL_ID(decl) = NULL;
    // CCNfree(decl);

    node_st *next = DECLS_NEXT(node);

    DECLS_DECL(node) = NULL;
    DECLS_NEXT(node) = NULL;
    CCNfree(node);

    return next;
}

/*
DECLS_NEXT(node) = TRAVopt(DECLS_NEXT(node));
DECLS_DECL(node) = TRAVdo(DECLS_DECL(node));

node_st *decl = DECLS_DECL(node);
if (NODE_TYPE(decl) != NT_VARDECL) {
return node;
}

node_st *ty = VARDECL_TY(decl);
node_st *exprs = TYPE_EXPRS(ty);
if (exprs == NULL) {
return node;

enum BasicType basic = TYPE_TY(ty);
node_st *value_ref =
vartable_temp_var(DATA_DAD_GET()->vartable, (vartype){basic, 0});
node_st *value =
ASTvardecl(ASTtype(NULL, basic), CCNcopy(VARREF_ID(value_ref)),
           VARDECL_EXPR(decl));
DECLS_NEXT(node) = ASTdecls(value, DECLS_NEXT(node));

CCNfree(value_ref);

node_st *assign = ASTassign(ASTvarref(VARDECL_ID(decl), NULL), value_ref);
DECLS_NEXT(node) = ASTdecls(assign, DECLS_NEXT(node));

VARDECL_EXPR(decl) = ASTmalloc(size, TYPE_TY(ty));
*/

node_st *DADfundecl(node_st *node) {
    DATA_DAD_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DAD_GET()->vartable = DATA_DAD_GET()->vartable->parent;

    return node;
}

#include <limits.h>

#include "ccn/ccn.h"
#include "macros.h"
#include "table/vartable.h"

void DFLinit(void) {}
void DFLfini(void) {}

node_st *DFLprogram(node_st *node) {
    DATA_DFL_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *DFLfundecl(node_st *node) {
    DATA_DFL_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_DFL_GET()->vartable = DATA_DFL_GET()->vartable->parent;

    return node;
}

node_st *DFLstmts(node_st *node) {
    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);
    if (NODE_TYPE(stmt) == NT_FOR) {
        FOR_LOOP_END(stmt) = TRAVstart(FOR_LOOP_END(stmt), TRAV_AOCF);
        FOR_LOOP_STEP(stmt) = TRAVstart(FOR_LOOP_STEP(stmt), TRAV_AOCF);

        node_st *start_ref = FOR_REF(stmt);
        node_st *end_ref = vartable_temp_var(DATA_DFL_GET()->vartable, TY_int);
        node_st *step_ref = vartable_temp_var(DATA_DFL_GET()->vartable, TY_int);

        FOR_REF(stmt) = NULL;

        node_st *clamped_end_ref =
            vartable_temp_var(DATA_DFL_GET()->vartable, TY_int);

        node_st *start_assign = ASTassign(start_ref, FOR_LOOP_START(stmt));
        node_st *end_assign = ASTassign(end_ref, FOR_LOOP_END(stmt));
        node_st *step_assign = ASTassign(step_ref, FOR_LOOP_STEP(stmt));

        FOR_LOOP_START(stmt) = NULL;
        FOR_LOOP_END(stmt) = NULL;
        FOR_LOOP_STEP(stmt) = NULL;

        node_st *step_inc =
            ASTassign(CCNcopy(start_ref),
                      ASTbinop(CCNcopy(start_ref), CCNcopy(step_ref), BO_add));
        VARREF_WRITE(ASSIGN_REF(step_inc)) = true;

        if (FOR_STMTS(stmt)) {
            node_st *tmp = FOR_STMTS(stmt);
            while (STMTS_NEXT(tmp)) {
                tmp = STMTS_NEXT(tmp);
            }
            STMTS_NEXT(tmp) = ASTstmts(step_inc, NULL);
        } else {
            FOR_STMTS(stmt) = ASTstmts(step_inc, NULL);
        }

        node_st *positive_step = NULL;
        node_st *negative_step = NULL;

        if (NODE_TYPE(ASSIGN_EXPR(step_assign)) != NT_INT ||
            INT_VAL(ASSIGN_EXPR(step_assign)) > 0) {
            node_st *positive_step_overflow = NULL;
            node_st *positive_step_no_overflow = NULL;

            node_st *positive_step_clamp_assign =
                ASTassign(CCNcopy(clamped_end_ref),
                          ASTbinop(ASTint(INT_MIN), CCNcopy(step_ref), BO_sub));
            VARREF_WRITE(ASSIGN_REF(positive_step_clamp_assign)) = true;

            if (NODE_TYPE(ASSIGN_EXPR(step_assign)) == NT_INT &&
                NODE_TYPE(ASSIGN_EXPR(end_assign)) == NT_INT &&
                INT_VAL(ASSIGN_EXPR(step_assign)) >
                    INT_MIN - INT_VAL(ASSIGN_EXPR(step_assign))) {

                node_st *positive_step_overflow_while =
                    ASTwhile(ASTbinop(CCNcopy(start_ref),
                                      CCNcopy(clamped_end_ref), BO_lt),
                             CCNcopy(FOR_STMTS(stmt)));

                node_st *positive_step_overflow_if = ASTifelse(
                    ASTbinop(CCNcopy(start_ref), CCNcopy(end_ref), BO_lt),
                    CCNcopy(FOR_STMTS(stmt)), NULL);

                positive_step_overflow =
                    ASTstmts(positive_step_overflow_while,
                             ASTstmts(positive_step_overflow_if, NULL));
            }

            if (NODE_TYPE(ASSIGN_EXPR(step_assign)) == NT_INT &&
                NODE_TYPE(ASSIGN_EXPR(end_assign)) == NT_INT &&
                INT_VAL(ASSIGN_EXPR(step_assign)) <=
                    INT_MIN - INT_VAL(ASSIGN_EXPR(step_assign))) {
                node_st *positive_step_no_overflow_while = ASTwhile(
                    ASTbinop(CCNcopy(start_ref), CCNcopy(end_ref), BO_lt),
                    CCNcopy(FOR_STMTS(stmt)));

                positive_step_no_overflow =
                    ASTstmts(positive_step_no_overflow_while, NULL);
            }

            node_st *positive_step_ifelse = ASTifelse(
                ASTbinop(CCNcopy(clamped_end_ref), CCNcopy(end_ref), BO_lt),
                positive_step_overflow, positive_step_no_overflow);

            positive_step = ASTstmts(positive_step_clamp_assign,
                                     ASTstmts(positive_step_ifelse, NULL));
        }

        if (NODE_TYPE(ASSIGN_EXPR(step_assign)) != NT_INT ||
            INT_VAL(ASSIGN_EXPR(step_assign)) < 0) {
            node_st *negative_step_overflow = NULL;
            node_st *negative_step_no_overflow = NULL;

            node_st *negative_step_clamp_assign =
                ASTassign(CCNcopy(clamped_end_ref),
                          ASTbinop(ASTint(INT_MAX), CCNcopy(step_ref), BO_sub));
            VARREF_WRITE(ASSIGN_REF(negative_step_clamp_assign)) = true;

            if (NODE_TYPE(ASSIGN_EXPR(step_assign)) == NT_INT &&
                NODE_TYPE(ASSIGN_EXPR(end_assign)) == NT_INT &&
                INT_VAL(ASSIGN_EXPR(step_assign)) >
                    INT_MAX - INT_VAL(ASSIGN_EXPR(step_assign))) {
                node_st *negative_step_overflow_while =
                    ASTwhile(ASTbinop(CCNcopy(start_ref),
                                      CCNcopy(clamped_end_ref), BO_gt),
                             CCNcopy(FOR_STMTS(stmt)));

                node_st *negative_step_overflow_if = ASTifelse(
                    ASTbinop(CCNcopy(start_ref), CCNcopy(end_ref), BO_gt),
                    CCNcopy(FOR_STMTS(stmt)), NULL);

                negative_step_overflow =
                    ASTstmts(negative_step_overflow_while,
                             ASTstmts(negative_step_overflow_if, NULL));
            }

            if (NODE_TYPE(ASSIGN_EXPR(step_assign)) == NT_INT &&
                NODE_TYPE(ASSIGN_EXPR(end_assign)) == NT_INT &&
                INT_VAL(ASSIGN_EXPR(step_assign)) <=
                    INT_MAX - INT_VAL(ASSIGN_EXPR(step_assign))) {
                node_st *negative_step_no_overflow_while = ASTwhile(
                    ASTbinop(CCNcopy(start_ref), CCNcopy(end_ref), BO_gt),
                    CCNcopy(FOR_STMTS(stmt)));

                negative_step_no_overflow =
                    ASTstmts(negative_step_no_overflow_while, NULL);
            }

            node_st *negative_step_ifelse = ASTifelse(
                ASTbinop(CCNcopy(clamped_end_ref), CCNcopy(end_ref), BO_gt),
                negative_step_overflow, negative_step_no_overflow);

            negative_step = ASTstmts(negative_step_clamp_assign,
                                     ASTstmts(negative_step_ifelse, NULL));
        }

        node_st *transformed =
            ASTifelse(ASTbinop(CCNcopy(step_ref), ASTint(0), BO_gt),
                      positive_step, negative_step);

        CCNfree(clamped_end_ref);

        VARREF_WRITE(start_ref) = true;
        VARREF_WRITE(end_ref) = true;
        VARREF_WRITE(step_ref) = true;

        TAKE(STMTS_NEXT(node));

        node = ASTstmts(
            start_assign,
            ASTstmts(end_assign,
                     ASTstmts(step_assign, ASTstmts(transformed, node))));
    }

    return node;
}

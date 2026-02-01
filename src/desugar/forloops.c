#include <limits.h>

#include "ccn/ccn.h"
#include "table/vartable.h"
#include "utils.h"

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
        node_st *end_ref;
        node_st *step_ref;
        node_st *clamped_end_ref = NULL;

        node_st *start_assign = ASTassign(start_ref, FOR_LOOP_START(stmt));
        node_st *end_assign = NULL;
        node_st *step_assign = NULL;
        node_st *clamped_end_assign = NULL;

        if (NODE_TYPE(FOR_LOOP_END(stmt)) == NT_INT) {
            end_ref = FOR_LOOP_END(stmt);
        } else {
            end_ref = vartable_temp_var(DATA_DFL_GET()->vartable, TY_int);
            end_assign = ASTassign(end_ref, FOR_LOOP_END(stmt));
        }

        if (NODE_TYPE(FOR_LOOP_STEP(stmt)) == NT_INT) {
            step_ref = FOR_LOOP_STEP(stmt);
        } else {
            step_ref = vartable_temp_var(DATA_DFL_GET()->vartable, TY_int);
            step_assign = ASTassign(step_ref, FOR_LOOP_STEP(stmt));
        }

        if (NODE_TYPE(step_ref) == NT_INT) {
            if (INT_VAL(step_ref) >= -1 && INT_VAL(step_ref) <= 1) {
            } else if (NODE_TYPE(step_ref) == NT_INT) {
                if (INT_VAL(step_ref) > 0 &&
                    INT_VAL(end_ref) + INT_VAL(step_ref) > INT_VAL(end_ref)) {
                } else if (INT_VAL(step_ref) < 0 &&
                           INT_VAL(end_ref) + INT_VAL(step_ref) <
                               INT_VAL(end_ref)) {
                } else {
                    clamped_end_ref =
                        ASTint(INT_VAL(end_ref) - INT_VAL(step_ref), TY_int);
                }
            } else {
                clamped_end_ref =
                    vartable_temp_var(DATA_DFL_GET()->vartable, TY_int);
                clamped_end_assign =
                    ASTassign(clamped_end_ref,
                              ASTbinop(CCNcopy(end_ref), CCNcopy(step_ref),
                                       BO_sub, TY_int));
            }
        } else {
            clamped_end_ref =
                vartable_temp_var(DATA_DFL_GET()->vartable, TY_int);
            clamped_end_assign = ASTassign(
                clamped_end_ref,
                ASTbinop(CCNcopy(end_ref), CCNcopy(step_ref), BO_sub, TY_int));
        }

        node_st *inc = ASTassign(
            CCNcopy(start_ref),
            ASTbinop(CCNcopy(start_ref), CCNcopy(step_ref), BO_add, TY_int));
        VARREF_WRITE(ASSIGN_REF(inc)) = true;

        if (FOR_STMTS(stmt)) {
            node_st *tmp = FOR_STMTS(stmt);
            while (STMTS_NEXT(tmp)) {
                tmp = STMTS_NEXT(tmp);
            }
            STMTS_NEXT(tmp) = ASTstmts(inc, NULL);
        } else {
            FOR_STMTS(stmt) = ASTstmts(inc, NULL);
        }

        FOR_REF(stmt) = NULL;
        FOR_LOOP_START(stmt) = NULL;
        FOR_LOOP_END(stmt) = NULL;
        FOR_LOOP_STEP(stmt) = NULL;

        node_st *positive_step = NULL;
        node_st *negative_step = NULL;

        if (!(NODE_TYPE(step_ref) == NT_INT && INT_VAL(step_ref) <= 0)) {
            node_st *positive_step_overflow = NULL;
            node_st *positive_step_no_overflow = NULL;

            if (clamped_end_ref) {
                node_st *positive_step_overflow_while =
                    ASTwhile(ASTbinop(CCNcopy(start_ref),
                                      CCNcopy(clamped_end_ref), BO_lt, TY_bool),
                             CCNcopy(FOR_STMTS(stmt)));

                node_st *positive_step_overflow_if =
                    ASTifelse(ASTbinop(CCNcopy(start_ref), CCNcopy(end_ref),
                                       BO_lt, TY_int),
                              CCNcopy(FOR_STMTS(stmt)), NULL);

                if (clamped_end_assign) {
                    positive_step_overflow = ASTstmts(
                        CCNcopy(clamped_end_assign),
                        ASTstmts(positive_step_overflow_while,
                                 ASTstmts(positive_step_overflow_if, NULL)));
                    VARREF_WRITE(
                        ASSIGN_REF(STMTS_STMT(positive_step_overflow))) = true;
                } else {
                    positive_step_overflow =
                        ASTstmts(positive_step_overflow_while,
                                 ASTstmts(positive_step_overflow_if, NULL));
                }
            }

            if (!(clamped_end_ref && NODE_TYPE(clamped_end_ref) == NT_INT)) {
                positive_step_no_overflow = ASTstmts(
                    ASTwhile(ASTbinop(CCNcopy(start_ref), CCNcopy(end_ref),
                                      BO_lt, TY_bool),
                             CCNcopy(FOR_STMTS(stmt))),
                    NULL);
            }

            if (!positive_step_no_overflow) {
                positive_step = positive_step_overflow;
            } else if (!positive_step_overflow) {
                positive_step = positive_step_no_overflow;
            } else {
                positive_step = ASTstmts(
                    ASTifelse(
                        ASTbinop(ASTbinop(CCNcopy(end_ref), CCNcopy(step_ref),
                                          BO_add, TY_int),
                                 CCNcopy(end_ref), BO_lt, TY_bool),
                        positive_step_overflow, positive_step_no_overflow),
                    NULL);
            }
        }

        if (!(NODE_TYPE(step_ref) == NT_INT && INT_VAL(step_ref) >= 0)) {
            node_st *negative_step_overflow = NULL;
            node_st *negative_step_no_overflow = NULL;

            if (clamped_end_ref) {
                node_st *negative_step_overflow_while =
                    ASTwhile(ASTbinop(CCNcopy(start_ref),
                                      CCNcopy(clamped_end_ref), BO_gt, TY_bool),
                             CCNcopy(FOR_STMTS(stmt)));

                node_st *negative_step_overflow_if =
                    ASTifelse(ASTbinop(CCNcopy(start_ref), CCNcopy(end_ref),
                                       BO_gt, TY_int),
                              CCNcopy(FOR_STMTS(stmt)), NULL);

                if (clamped_end_assign) {
                    negative_step_overflow = ASTstmts(
                        CCNcopy(clamped_end_assign),
                        ASTstmts(negative_step_overflow_while,
                                 ASTstmts(negative_step_overflow_if, NULL)));
                    VARREF_WRITE(
                        ASSIGN_REF(STMTS_STMT(negative_step_overflow))) = true;
                } else {
                    negative_step_overflow =
                        ASTstmts(negative_step_overflow_while,
                                 ASTstmts(negative_step_overflow_if, NULL));
                }
            }

            if (!(clamped_end_ref && NODE_TYPE(clamped_end_ref) == NT_INT)) {
                negative_step_no_overflow = ASTstmts(
                    ASTwhile(ASTbinop(CCNcopy(start_ref), CCNcopy(end_ref),
                                      BO_gt, TY_bool),
                             CCNcopy(FOR_STMTS(stmt))),
                    NULL);
            }

            if (!negative_step_no_overflow) {
                negative_step = negative_step_overflow;
            } else if (!negative_step_overflow) {
                negative_step = negative_step_no_overflow;
            } else {
                negative_step = ASTstmts(
                    ASTifelse(
                        ASTbinop(ASTbinop(CCNcopy(end_ref), CCNcopy(step_ref),
                                          BO_add, TY_int),
                                 CCNcopy(end_ref), BO_gt, TY_bool),
                        negative_step_overflow, negative_step_no_overflow),
                    NULL);
            }
        }

        TAKE(STMTS_NEXT(node));

        node_st *transformed;
        if (!positive_step) {
            transformed = negative_step;
        } else if (!negative_step) {
            transformed = positive_step;
        } else {
            transformed =
                ASTstmts(ASTifelse(ASTbinop(CCNcopy(step_ref),
                                            ASTint(0, TY_int), BO_gt, TY_bool),
                                   positive_step, negative_step),
                         NULL);
        }
        node = inline_stmts(node, transformed);

        if (clamped_end_assign) {
            CCNfree(clamped_end_assign);
        } else {
            CCNfree(clamped_end_ref);
        }

        if (step_assign) {
            VARREF_WRITE(step_ref) = true;
            node = ASTstmts(step_assign, node);
        } else {
            CCNfree(step_ref);
        }

        if (end_assign) {
            VARREF_WRITE(end_ref) = true;
            node = ASTstmts(end_assign, node);
        } else {
            CCNfree(end_ref);
        }

        VARREF_WRITE(start_ref) = true;
        node = ASTstmts(start_assign, node);
    }

    return node;
}

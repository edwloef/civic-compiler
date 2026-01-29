#include <limits.h>

#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "globals/globals.h"
#include "utils.h"

void AOLUinit(void) {}
void AOLUfini(void) {}

node_st *AOLUprogram(node_st *node) {
    DATA_AOLU_GET()->funtable = PROGRAM_FUNTABLE(node);
    DATA_AOLU_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOLUfundecl(node_st *node) {
    DATA_AOLU_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_AOLU_GET()->vartable = DATA_AOLU_GET()->vartable->parent;

    return node;
}

node_st *AOLUfunbody(node_st *node) {
    DATA_AOLU_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AOLU_GET()->funtable = DATA_AOLU_GET()->funtable->parent;

    return node;
}

node_st *AOLUstmts(node_st *node) {
    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);
    if (NODE_TYPE(stmt) == NT_DOWHILE) {

        bool can_unroll = DOWHILE_KNOWN_START(stmt) &&
                          NODE_TYPE(DOWHILE_EXPR(stmt)) == NT_BINOP &&
                          NODE_TYPE(BINOP_LEFT(DOWHILE_EXPR(stmt))) == NT_INT;

        int start = DOWHILE_UNROLL_START(stmt);
        int end;
        int step;
        int count;

        if (can_unroll) {
            end = INT_VAL(BINOP_LEFT(DOWHILE_EXPR(stmt)));

            node_st *ref = BINOP_RIGHT(DOWHILE_EXPR(stmt));
            funtable *funtable = DATA_AOLU_GET()->funtable;
            vartable *vartable = DATA_AOLU_GET()->vartable;

            TRAVpush(TRAV_CU);

            DATA_CU_GET()->ref = ref;
            DATA_CU_GET()->funtable = funtable;
            DATA_CU_GET()->vartable = vartable;

            TRAVstmts(stmt);

            can_unroll = DATA_CU_GET()->can_unroll;
            step = DATA_CU_GET()->step;

            TRAVpop();
        }

        if (can_unroll) {
            if (step == 0) {
                CCNfree(DOWHILE_EXPR(stmt));
                DOWHILE_EXPR(stmt) = ASTbool(true);
                can_unroll = false;
            } else {
                count = (end - start) / step;

                if (count <= 0) {
                    CCNfree(DOWHILE_EXPR(stmt));
                    DOWHILE_EXPR(stmt) = ASTbool(false);
                    can_unroll = false;
                } else {
                    TRAVpush(TRAV_EC);

                    TRAVdo(DOWHILE_STMTS(stmt));

                    int cost = DATA_EC_GET()->cost;

                    TRAVpop();

                    if (count * cost > globals.unroll_limit) {
                        can_unroll = false;
                    } else if (step > 0) {
                        can_unroll = INT_MIN - step > end;
                    } else {
                        can_unroll = INT_MAX - step < end;
                    }
                }
            }
        }

        if (can_unroll) {
            node_st *head = STMTS_NEXT(node);
            STMTS_NEXT(node) = NULL;

            for (int i = start; i < count; i++) {
                head = inline_stmts(head, CCNcopy(DOWHILE_STMTS(stmt)));
            }

            CCNfree(node);
            node = head;
        }
    }

    return node;
}

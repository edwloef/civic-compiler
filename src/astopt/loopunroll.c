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
                          NODE_TYPE(DOWHILE_EXPR(stmt)) == NT_BINOP;

        int start = DOWHILE_UNROLL_START(stmt);
        int end;
        int step;

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
            if (step > 0) {
                can_unroll = end + step > end;
            } else if (step < 0) {
                can_unroll = end + step < end;
            } else {
                CCNfree(BINOP_RIGHT(DOWHILE_EXPR(stmt)));
                BINOP_RIGHT(DOWHILE_EXPR(stmt)) = ASTint(start, TY_int);
                can_unroll = false;
                CCNcycleNotify();
            }
        }

        long long count;

        if (can_unroll) {
            long long llstart = start;
            long long llend = end;
            long long llstep = step;

            long long lllen = llend - llstart;

            switch (BINOP_OP(DOWHILE_EXPR(stmt))) {
            case BO_lt:
                count = (step < 0) ? lllen / llstep + (lllen % llstep != 0) : 1;
                can_unroll = (step < 0) == (llend < llstart + llstep);
                break;
            case BO_le:
                count = (step < 0) ? lllen / llstep + 1 : 1;
                can_unroll = (step < 0) == (llend <= llstart + llstep);
                break;
            case BO_gt:
                count = (step > 0) ? lllen / llstep + (lllen % llstep != 0) : 1;
                can_unroll = (step > 0) == (llend > llstart + llstep);
                break;
            case BO_ge:
                count = (step > 0) ? lllen / llstep + 1 : 1;
                can_unroll = (step > 0) == (llend >= llstart + llstep);
                break;
            case BO_eq:
                count = (llstart + llstep == llend) + 1;
                break;
            case BO_ne:
                count = lllen / llstep;
                can_unroll = lllen % llstep == 0;
                break;
            default:
                DBUG_ASSERT(false, "Unknown binop detected.");
            }
        }

        long long cost;
        bool can_strip_mine = false;

        if (can_unroll && count != 1) {
            TRAVpush(TRAV_EC);

            TRAVdo(DOWHILE_STMTS(stmt));

            cost = DATA_EC_GET()->cost;

            TRAVpop();

            can_unroll = count * cost <= globals.unroll_limit;
            can_strip_mine = 2 * cost <= globals.unroll_limit;
        }

        if (can_unroll) {
            node_st *head = STMTS_NEXT(node);
            STMTS_NEXT(node) = NULL;

            for (int i = 0; i < count; i++) {
                head = inline_stmts(head, CCNcopy(DOWHILE_STMTS(stmt)));
            }

            CCNfree(node);
            node = head;
        } else if (can_strip_mine) {
            int inner_count = globals.unroll_limit / cost;
            int remainder = count % inner_count;

            node_st *head = node;
            for (int i = 0; i < remainder; i++) {
                head = inline_stmts(head, CCNcopy(DOWHILE_STMTS(stmt)));
            }
            node = head;

            head = DOWHILE_STMTS(stmt);
            for (int i = 1; i < inner_count; i++) {
                head = inline_stmts(head, CCNcopy(DOWHILE_STMTS(stmt)));
            }
            DOWHILE_STMTS(stmt) = head;
        }
    }

    return node;
}

#include "ccn/ccn.h"
#include "ccngen/trav.h"
#include "macros.h"

static node_st *AODSinlinestmts(node_st *node, node_st *stmts) {
    TAKE(STMTS_NEXT(node));

    if (stmts) {
        node_st *tmp = stmts;
        while (STMTS_NEXT(tmp)) {
            tmp = STMTS_NEXT(tmp);
        }
        STMTS_NEXT(tmp) = node;
        return stmts;
    } else {
        return node;
    }
}

void AODSinit(void) {}
void AODSfini(void) {}

node_st *AODSprogram(node_st *node) {
    DATA_AODS_GET()->vartable = PROGRAM_VARTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AODSfundecl(node_st *node) {
    DATA_AODS_GET()->vartable = FUNDECL_VARTABLE(node);

    TRAVchildren(node);

    DATA_AODS_GET()->vartable = DATA_AODS_GET()->vartable->parent;

    return node;
}

node_st *AODSstmts(node_st *node) {
    TRAVchildren(node);

    node_st *stmt = STMTS_STMT(node);
    if (NODE_TYPE(stmt) == NT_ASSIGN && !VARREF_EXPRS(ASSIGN_REF(stmt))) {
        TRAVpush(TRAV_CD);

        DATA_CD_GET()->n = VARREF_N(ASSIGN_REF(stmt));
        DATA_CD_GET()->l = VARREF_L(ASSIGN_REF(stmt));

        TRAVnext(node);

        bool is_dead = DATA_CD_GET()->is_dead;

        TRAVpop();

        if (is_dead) {
            TRAVpush(TRAV_EC);

            TRAVexpr(stmt);
            node = AODSinlinestmts(node, DATA_EC_GET()->stmts);

            TRAVpop();
        }
    }

    return node;
}

node_st *AODSifelse(node_st *node) {
    TRAVexpr(node);

    // TODO

    return node;
}

node_st *AODSdowhile(node_st *node) {
    // TODO

    return node;
}

node_st *AODSfor(node_st *node) {
    TRAVloop_start(node);
    TRAVloop_end(node);
    TRAVloop_step(node);

    // TODO

    return node;
}

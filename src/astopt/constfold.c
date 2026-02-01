#include "ccn/ccn.h"
#include "palm/dbug.h"

#define CONST_INT(val) CONST_RETURN(ASTint(val, TY_int));
#define CONST_FLOAT(val) CONST_RETURN(ASTfloat(val, TY_float));
#define CONST_BOOL(val) CONST_RETURN(ASTbool(val, TY_bool));
#define CONST_RETURN(val)                                                      \
    {                                                                          \
        node_st *constval = val;                                               \
        CCNcycleNotify();                                                      \
        CCNfree(node);                                                         \
        return constval;                                                       \
    }

#define INT_MONOP(op) op INT_VAL(MONOP_EXPR(node))
#define FLOAT_MONOP(op) op FLOAT_VAL(MONOP_EXPR(node))
#define BOOL_MONOP(op) op BOOL_VAL(MONOP_EXPR(node))

node_st *AOCFmonop(node_st *node) {
    TRAVchildren(node);
    switch (MONOP_OP(node)) {
    case MO_pos:
        switch (NODE_TYPE(MONOP_EXPR(node))) {
        case NT_INT:
            CONST_INT(INT_MONOP(+));
        case NT_FLOAT:
            CONST_FLOAT(FLOAT_MONOP(+));
        default:
            return node;
        }
    case MO_neg:
        switch (NODE_TYPE(MONOP_EXPR(node))) {
        case NT_INT:
            CONST_INT(INT_MONOP(-));
        case NT_FLOAT:
            CONST_FLOAT(FLOAT_MONOP(-));
        default:
            return node;
        }
    case MO_not:
        switch (NODE_TYPE(MONOP_EXPR(node))) {
        case NT_BOOL:
            CONST_BOOL(BOOL_MONOP(!));
        default:
            return node;
        }
    default:
        DBUG_ASSERT(false, "Unknown monop detected.");
        return node;
    }
}

#define INT_BINOP(op) INT_VAL(BINOP_LEFT(node)) op INT_VAL(BINOP_RIGHT(node))
#define FLOAT_BINOP(op)                                                        \
    FLOAT_VAL(BINOP_LEFT(node)) op FLOAT_VAL(BINOP_RIGHT(node))
#define BOOL_BINOP(op) BOOL_VAL(BINOP_LEFT(node)) op BOOL_VAL(BINOP_RIGHT(node))

node_st *AOCFbinop(node_st *node) {
    TRAVchildren(node);
    if (NODE_TYPE(BINOP_LEFT(node)) != NODE_TYPE(BINOP_RIGHT(node))) {
        return node;
    }

    switch (BINOP_OP(node)) {
    case BO_lt:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            CONST_BOOL(INT_BINOP(<));
        case NT_FLOAT:
            CONST_BOOL(FLOAT_BINOP(<));
        default:
            return node;
        }
    case BO_le:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            CONST_BOOL(INT_BINOP(<=));
        case NT_FLOAT:
            CONST_BOOL(FLOAT_BINOP(<=));
        default:
            return node;
        }
    case BO_gt:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            CONST_BOOL(INT_BINOP(>));
        case NT_FLOAT:
            CONST_BOOL(FLOAT_BINOP(>));
        default:
            return node;
        }
    case BO_ge:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            CONST_BOOL(INT_BINOP(>=));
        case NT_FLOAT:
            CONST_BOOL(FLOAT_BINOP(>=));
        default:
            return node;
        }
    case BO_sub:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            CONST_INT(INT_BINOP(-));
        case NT_FLOAT:
            CONST_FLOAT(FLOAT_BINOP(-));
        default:
            return node;
        }
    case BO_div:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            if (INT_VAL(BINOP_RIGHT(node)) != 0) {
                CONST_INT(INT_BINOP(/));
            } else {
                return node;
            }
        case NT_FLOAT:
            CONST_FLOAT(FLOAT_BINOP(/));
        default:
            return node;
        }
    case BO_mod:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            if (INT_VAL(BINOP_RIGHT(node)) != 0) {
                CONST_INT(INT_BINOP(%));
            } else {
                return node;
            }
        default:
            return node;
        }
    case BO_eq:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            CONST_BOOL(INT_BINOP(==));
        case NT_FLOAT:
            CONST_BOOL(FLOAT_BINOP(==));
        case NT_BOOL:
            CONST_BOOL(BOOL_BINOP(==));
        default:
            return node;
        }
    case BO_ne:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            CONST_BOOL(INT_BINOP(!=));
        case NT_FLOAT:
            CONST_BOOL(FLOAT_BINOP(!=));
        case NT_BOOL:
            CONST_BOOL(BOOL_BINOP(!=));
        default:
            return node;
        }
    case BO_add:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            CONST_INT(INT_BINOP(+));
        case NT_FLOAT:
            CONST_FLOAT(FLOAT_BINOP(+));
        case NT_BOOL:
            CONST_BOOL(BOOL_BINOP(||));
        default:
            return node;
        }
    case BO_mul:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_INT:
            CONST_INT(INT_BINOP(*));
        case NT_FLOAT:
            CONST_FLOAT(FLOAT_BINOP(*));
        case NT_BOOL:
            CONST_BOOL(BOOL_BINOP(&&));
        default:
            return node;
        }
    case BO_and:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_BOOL:
            CONST_BOOL(BOOL_BINOP(&&));
        default:
            return node;
        }
    case BO_or:
        switch (NODE_TYPE(BINOP_LEFT(node))) {
        case NT_BOOL:
            CONST_BOOL(BOOL_BINOP(||));
        default:
            return node;
        }
    default:
        DBUG_ASSERT(false, "Unknown binop detected.");
        return node;
    }
}

node_st *AOCFcast(node_st *node) {
    TRAVchildren(node);
    switch (CAST_RESOLVED_TY(node)) {
    case TY_int:
        switch (NODE_TYPE(CAST_EXPR(node))) {
        case NT_INT:
            CONST_INT(INT_VAL(CAST_EXPR(node)));
        case NT_FLOAT:
            CONST_INT(FLOAT_VAL(CAST_EXPR(node)));
        case NT_BOOL:
            CONST_INT(BOOL_VAL(CAST_EXPR(node)));
        default:
            return node;
        }
    case TY_float:
        switch (NODE_TYPE(CAST_EXPR(node))) {
        case NT_INT:
            CONST_FLOAT(INT_VAL(CAST_EXPR(node)));
        case NT_FLOAT:
            CONST_FLOAT(FLOAT_VAL(CAST_EXPR(node)));
        case NT_BOOL:
            CONST_FLOAT(BOOL_VAL(CAST_EXPR(node)));
        default:
            return node;
        }
    case TY_bool:
        switch (NODE_TYPE(CAST_EXPR(node))) {
        case NT_INT:
            CONST_BOOL(INT_VAL(CAST_EXPR(node)));
        case NT_FLOAT:
            CONST_BOOL(FLOAT_VAL(CAST_EXPR(node)));
        case NT_BOOL:
            CONST_BOOL(BOOL_VAL(CAST_EXPR(node)));
        default:
            return node;
        }
    default:
        DBUG_ASSERT(false, "Unknown basic type detected.");
        return node;
    };
}

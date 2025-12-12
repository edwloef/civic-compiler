/**
 * @file
 *
 * This file contains the code for the CountOps traversal.
 * The traversal has the uid: SCO
 *
 *
 */

#include "ccn/ccn.h"
#include "ccngen/ast.h"
#include "ccngen/enum.h"
#include "ccngen/trav.h"
#include "ccngen/trav_data.h"
#include <stdio.h>

void SCOinit(void) {}

void SCOfini(void) {
    printf("%d additions\n", DATA_SCO_GET()->add);
    printf("%d subtractions\n", DATA_SCO_GET()->sub);
    printf("%d multiplications\n", DATA_SCO_GET()->mul);
    printf("%d divisions\n", DATA_SCO_GET()->div);
    printf("%d modulos\n", DATA_SCO_GET()->mod);
}

node_st *SCObinop(node_st *node) {
    TRAVleft(node);
    TRAVright(node);

    switch (BINOP_OP(node)) {
    case BO_add:
        DATA_SCO_GET()->add++;
        break;
    case BO_sub:
        DATA_SCO_GET()->sub++;
        break;
    case BO_mul:
        DATA_SCO_GET()->mul++;
        break;
    case BO_div:
        DATA_SCO_GET()->div++;
        break;
    case BO_mod:
        DATA_SCO_GET()->mod++;
        break;
    default:
        break;
    }

    return node;
}

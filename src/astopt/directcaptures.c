#include "ccn/ccn.h"
#include "utils.h"

void AODCinit(void) {}
void AODCfini(void) {}

node_st *AODCprogram(node_st *node) {
    DATA_AODC_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AODCfundecl(node_st *node) {
    int prev_read_capture = DATA_AOIC_GET()->read_capture;
    int prev_write_capture = DATA_AOIC_GET()->write_capture;
    int prev_scalar_read_capture = DATA_AOIC_GET()->scalar_read_capture;
    int prev_scalar_write_capture = DATA_AOIC_GET()->scalar_write_capture;

    DATA_AODC_GET()->read_capture = FUNDECL_EXTERNAL(node);
    DATA_AODC_GET()->write_capture = FUNDECL_EXTERNAL(node);
    DATA_AODC_GET()->scalar_read_capture = FUNDECL_EXTERNAL(node);
    DATA_AODC_GET()->scalar_write_capture = FUNDECL_EXTERNAL(node);

    TRAVchildren(node);

    funtable_ref r = {0, FUNDECL_L(node)};
    funtable_entry *e = funtable_get(DATA_AODC_GET()->funtable, r);
    e->read_capture = DATA_AODC_GET()->read_capture;
    e->write_capture = DATA_AODC_GET()->write_capture;
    e->scalar_read_capture = DATA_AODC_GET()->scalar_read_capture;
    e->scalar_write_capture = DATA_AODC_GET()->scalar_write_capture;

    DATA_AOIC_GET()->read_capture = prev_read_capture;
    DATA_AOIC_GET()->write_capture = prev_write_capture;
    DATA_AOIC_GET()->scalar_read_capture = prev_scalar_read_capture;
    DATA_AOIC_GET()->scalar_write_capture = prev_scalar_write_capture;

    return node;
}

node_st *AODCfunbody(node_st *node) {
    DATA_AODC_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AODC_GET()->funtable = DATA_AODC_GET()->funtable->parent;

    return node;
}

node_st *AODCvarref(node_st *node) {
    TRAVchildren(node);

    if (VARREF_WRITE(node)) {
        DATA_AODC_GET()->write_capture =
            MAX(DATA_AODC_GET()->write_capture, VARREF_N(node));
    } else {
        DATA_AODC_GET()->read_capture =
            MAX(DATA_AODC_GET()->read_capture, VARREF_N(node));
    }

    if (VARREF_RESOLVED_DIMS(node) == 0 && !VARREF_EXPRS(node)) {
        if (VARREF_WRITE(node)) {
            DATA_AODC_GET()->scalar_write_capture =
                MAX(DATA_AODC_GET()->scalar_write_capture, VARREF_N(node));
        } else {
            DATA_AODC_GET()->scalar_read_capture =
                MAX(DATA_AODC_GET()->scalar_read_capture, VARREF_N(node));
        }
    }

    return node;
}

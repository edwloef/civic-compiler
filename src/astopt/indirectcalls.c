#include "ccn/ccn.h"
#include "utils.h"

void AOICinit(void) {}
void AOICfini(void) {}

node_st *AOICprogram(node_st *node) {
    DATA_AOIC_GET()->funtable = PROGRAM_FUNTABLE(node);

    TRAVchildren(node);

    return node;
}

node_st *AOICfundecl(node_st *node) {
    int prev_read_capture = DATA_AOIC_GET()->read_capture;
    int prev_write_capture = DATA_AOIC_GET()->write_capture;
    int prev_scalar_read_capture = DATA_AOIC_GET()->scalar_read_capture;
    int prev_scalar_write_capture = DATA_AOIC_GET()->scalar_write_capture;

    funtable_ref r = {0, FUNDECL_L(node)};
    funtable_entry *e = funtable_get(DATA_AOIC_GET()->funtable, r);

    DATA_AOIC_GET()->read_capture = e->read_capture;
    DATA_AOIC_GET()->write_capture = e->write_capture;
    DATA_AOIC_GET()->scalar_read_capture = e->scalar_read_capture;
    DATA_AOIC_GET()->scalar_write_capture = e->scalar_write_capture;

    TRAVchildren(node);

    if (e->read_capture != DATA_AOIC_GET()->read_capture) {
        e->read_capture = DATA_AOIC_GET()->read_capture;
        CCNcycleNotify();
    }

    if (e->write_capture != DATA_AOIC_GET()->write_capture) {
        e->write_capture = DATA_AOIC_GET()->write_capture;
        CCNcycleNotify();
    }

    if (e->scalar_read_capture != DATA_AOIC_GET()->scalar_read_capture) {
        e->scalar_read_capture = DATA_AOIC_GET()->scalar_read_capture;
        CCNcycleNotify();
    }

    if (e->scalar_write_capture != DATA_AOIC_GET()->scalar_write_capture) {
        e->scalar_write_capture = DATA_AOIC_GET()->scalar_write_capture;
        CCNcycleNotify();
    }

    DATA_AOIC_GET()->read_capture = prev_read_capture;
    DATA_AOIC_GET()->write_capture = prev_write_capture;
    DATA_AOIC_GET()->scalar_read_capture = prev_scalar_read_capture;
    DATA_AOIC_GET()->scalar_write_capture = prev_scalar_write_capture;

    return node;
}

node_st *AOICfunbody(node_st *node) {
    DATA_AOIC_GET()->funtable = FUNBODY_FUNTABLE(node);

    TRAVchildren(node);

    DATA_AOIC_GET()->funtable = DATA_AOIC_GET()->funtable->parent;

    return node;
}

node_st *AOICcall(node_st *node) {
    TRAVchildren(node);

    funtable_ref r = {CALL_N(node), CALL_L(node)};
    funtable_entry *e = funtable_get(DATA_AOIC_GET()->funtable, r);

    DATA_AOIC_GET()->read_capture =
        MAX(DATA_AOIC_GET()->read_capture, e->read_capture + r.n - 1);
    DATA_AOIC_GET()->write_capture =
        MAX(DATA_AOIC_GET()->write_capture, e->write_capture + r.n - 1);
    DATA_AOIC_GET()->scalar_read_capture = MAX(
        DATA_AOIC_GET()->scalar_read_capture, e->scalar_read_capture + r.n - 1);
    DATA_AOIC_GET()->scalar_write_capture =
        MAX(DATA_AOIC_GET()->scalar_write_capture,
            e->scalar_write_capture + r.n - 1);

    return node;
}

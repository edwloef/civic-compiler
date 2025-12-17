#include "globals.h"

struct globals global;

void GLBinitializeGlobals(void) {
  global.col = 0;
  global.line = 0;
  global.input_file = NULL;
  global.output_file = NULL;
  global.strength_reduction_limit = 2;
}

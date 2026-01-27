cmake_minimum_required(VERSION 3.13)

project(civicc VERSION 1.0 LANGUAGES C)

set(TEST_DIR "${CMAKE_CURRENT_LIST_DIR}/test")
set(COMPILER "${CMAKE_CURRENT_BINARY_DIR}/civicc")

option(DISABLE_ASAN "Disable address sanitizer")
option(DISABLE_UBSAN "Disable undefined behavior sanitizer")

# These will only work after you received the testing framework from us.
enable_testing()
add_test(NAME "basic" COMMAND "${TEST_COMMAND}/run.bash" "${COMPILER}" basic WORKING_DIRECTORY "${TEST_DIR}")
add_test(NAME "nested_funs" COMMAND "${TEST_COMMAND}/run.bash" "${COMPILER}" nested_funs WORKING_DIRECTORY "${TEST_DIR}")
add_test(NAME "arrays" COMMAND "${TEST_COMMAND}/run.bash" "${COMPILER}" arrays WORKING_DIRECTORY "${TEST_DIR}")


find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(CivicParser src/scanparse/parser.y "${CMAKE_CURRENT_BINARY_DIR}/parser.c"
    DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/parser.h" VERBOSE COMPILE_FLAGS "-Wall -Wcounterexamples")

flex_target(CivicLexer src/scanparse/lexer.l "${CMAKE_CURRENT_BINARY_DIR}/lexer.c"
    DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/lexer.h")
ADD_FLEX_BISON_DEPENDENCY(CivicLexer CivicParser)


include(./coconut/coconut.cmake)

# Whenever you add a file, add it here too.
add_executable(civicc ${FLEX_CivicLexer_OUTPUTS} ${BISON_CivicParser_OUTPUTS}
    src/analysis/divergence.c
    src/analysis/functions.c
    src/analysis/globals.c
    src/analysis/resolve.c
    src/analysis/typecheck.c
    src/astopt/checkdead.c
    src/astopt/checkproploop.c
    src/astopt/constfold.c
    src/astopt/copyprop.c
    src/astopt/countcalls.c
    src/astopt/countreads.c
    src/astopt/countwrites.c
    src/astopt/deadfunctions.c
    src/astopt/deadstores.c
    src/astopt/directcaptures.c
    src/astopt/escapes.c
    src/astopt/extractstmts.c
    src/astopt/identities.c
    src/astopt/indirectcalls.c
    src/astopt/reassoc.c
    src/astopt/sideeffects.c
    src/astopt/trivialcontrolflow.c
    src/astopt/valueprop.c
    src/astopt/valueproploop.c
    src/codegen/shrinkfuntables.c
    src/codegen/shrinkvartables.c
    src/desugar/arrayassign.c
    src/desugar/arrayparams.c
    src/desugar/boolcasts.c
    src/desugar/dimreduce.c
    src/desugar/forloops.c
    src/desugar/vardecls.c
    src/desugar/whileloops.c
    src/error/error.c
    src/free/free.c
    src/globals/globals.c
    src/print/print.c
    src/table/funtable.c
    src/table/vartable.c
    src/main.c
    src/utils.c
)

target_compile_options(civicc PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -fwrapv>
)

# Enable address sanitizer
if(NOT DISABLE_ASAN)
    target_compile_options(civicc PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:$<$<CONFIG:Debug>:-fsanitize=address>>
    )

    target_link_options(civicc PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:$<$<CONFIG:Debug>:-fsanitize=address>>
    )
endif()

# Enable undefined behavior sanitizer
if(NOT DISABLE_UBSAN)
    target_compile_options(civicc PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:$<$<CONFIG:Debug>:-fsanitize=undefined>>
    )

    target_link_options(civicc PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:$<$<CONFIG:Debug>:-fsanitize=undefined>>
    )
endif()

coconut_target_generate(civicc "${CMAKE_CURRENT_LIST_DIR}/src/main.ccn" dynamic)
target_include_directories(civicc
    PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src"
)

add_custom_target(dot
    dot -Tpng ccngen/ast.dot > ast.png
    COMMENT "Generate a png of your ast based on the generated dot diagram."
)

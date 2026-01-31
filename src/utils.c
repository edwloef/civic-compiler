#include <spawn.h>
#include <stdio.h>
#include <unistd.h>

#include "ccn/ccn.h"
#include "utils.h"

node_st *inline_stmts(node_st *node, node_st *stmts) {
    if (stmts) {
        node_st *tmp = stmts;
        while (STMTS_NEXT(tmp)) {
            tmp = STMTS_NEXT(tmp);
        }
        STMTS_NEXT(tmp) = node;
        CCNcycleNotify();
        return stmts;
    } else {
        return node;
    }
}

extern char **environ;

FILE *spawn_command(FILE *in, const char *cmd, char *const argv[]) {
    int out_fds[2];
    if (pipe(out_fds)) {
        return NULL;
    }

    posix_spawn_file_actions_t actions;
    if (posix_spawn_file_actions_init(&actions)) {
        return NULL;
    }

    if (in != NULL &&
        posix_spawn_file_actions_adddup2(&actions, fileno(in), STDIN_FILENO)) {
        posix_spawn_file_actions_destroy(&actions);
        return NULL;
    }

    if (posix_spawn_file_actions_addclose(&actions, out_fds[0]) ||
        posix_spawn_file_actions_adddup2(&actions, out_fds[1], STDOUT_FILENO)) {
        posix_spawn_file_actions_destroy(&actions);
        return NULL;
    }

    pid_t pid;
    int res = posix_spawnp(&pid, cmd, &actions, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&actions);
    if (res) {
        return NULL;
    }

    close(out_fds[1]);

    return fdopen(out_fds[0], "r");
}

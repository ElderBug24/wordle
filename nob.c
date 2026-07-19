#define NOB_IMPLEMENTATION
#include "../nob.h"


int main(int argc, char** argv) {
  NOB_GO_REBUILD_URSELF(argc, argv);

  Nob_Cmd cmd = {0};

  if (!nob_mkdir_if_not_exists("build")) return 1;
  nob_cmd_append(&cmd, "gcc", "-Wall", "-Wextra", "-Wconversion", "-pedantic", "-Wno-overlength-strings", "-o", "build/main", "main.c");
  if (!nob_cmd_run(&cmd)) return 1;
  nob_cmd_append(&cmd, "build/main");
  if (!nob_cmd_run(&cmd)) return 1;

  return 0;
}


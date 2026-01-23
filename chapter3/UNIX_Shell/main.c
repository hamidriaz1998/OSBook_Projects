#include "linenoise.h"
#include "shell.h"
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#define HISTORY_FILENAME ".osh_history"
#define MAX_HISTORY_LEN 500

// Helper function to get full history path
char *get_history_path() {
  static char path[1024];
  const char *home = getenv("HOME");

  // If HOME is not set, try getting it from passwd
  if (home == NULL) {
    struct passwd *pw = getpwuid(getuid());
    if (pw != NULL) {
      home = pw->pw_dir;
    }
  }

  if (home != NULL) {
    snprintf(path, sizeof(path), "%s/%s", home, HISTORY_FILENAME);
    return path;
  }

  // Fallback to current directory if HOME not found
  return HISTORY_FILENAME;
}

int main() {
  // Setup linenoise
  linenoiseHistorySetMaxLen(MAX_HISTORY_LEN);
  char *history_path = get_history_path();
  linenoiseHistoryLoad(history_path);

  bool should_run = true;
  char *line;
  struct Command cmd = {{0},  {0},   {0},  false, false, false,
                        NULL, false, NULL, false, {0},   0};

  while (should_run) {
    line = linenoise("osh> ");

    // Check for EOF (Ctrl+D)
    if (line == NULL) {
      printf("\n");
      break;
    }

    // Check for empty input
    if (line[0] == '\0') {
      free(line);
      continue;
    }

    // Copy to command buffer
    if (strlen(line) >= sizeof(cmd.input_buf)) {
      printf("Error: Command too long (max %d characters)\n",
             (int)sizeof(cmd.input_buf) - 1);
      free(line);
      continue;
    }
    strcpy(cmd.input_buf, line);

    // Check if it's a built-in command before adding to history
    bool is_builtin = (strcmp(line, "exit") == 0 ||
                       strcmp(line, "clear") == 0 || strcmp(line, "!!") == 0);

    free(line);

    // Handle exit
    if (strcmp(cmd.input_buf, "exit") == 0) {
      should_run = false;
      continue;
    }

    // Handle clear command
    if (strcmp(cmd.input_buf, "clear") == 0) {
      linenoiseClearScreen();
      continue;
    }

    // Run last command
    if (strcmp(cmd.input_buf, "!!") == 0) {
      if (cmd.last_command_buf[0] == '\0') {
        printf("Error: No commands in history.\n");
        continue;
      } else {
        strcpy(cmd.input_buf, cmd.last_command_buf);
        printf("Running command: %s\n", cmd.last_command_buf);
      }
    }

    // Save the command to cmd.last_command_buf before parsing
    memset(cmd.last_command_buf, 0, sizeof(cmd.last_command_buf));
    strcpy(cmd.last_command_buf, cmd.input_buf);

    // Add to history (after processing !! but before execution)
    // Don't add built-in commands to history
    if (!is_builtin) {
      linenoiseHistoryAdd(cmd.input_buf);
    }

    // Tokenize and parse
    if (tokenize_input(&cmd) == -1) {
      reset_command(&cmd);
      continue;
    }

    if (parse_input(&cmd) == -1) {
      reset_command(&cmd);
      continue;
    }

#ifdef DEBUG
    debug_command(&cmd);
#endif

    // Execute command
    execute_command(&cmd);

    // Reset command structure for next iteration
    reset_command(&cmd);
  }

  // Save history on exit
  linenoiseHistorySave(history_path);

  return 0;
}

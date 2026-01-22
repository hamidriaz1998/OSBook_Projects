#include "shell.h"

int main() {
  bool should_run = 1;
  struct Command cmd = {{0},   {0},  {0},   false, false,
                        false, NULL, false, NULL,  false};

  while (should_run) {
    printf("osh> ");
    fflush(stdout);
    if (fgets(cmd.input_buf, sizeof(cmd.input_buf), stdin) == NULL) {
      printf("Error: Invalid Input");
      continue;
    } else {
      size_t length = strlen(cmd.input_buf);
      // Check for empty input
      if (length == 1 && cmd.input_buf[0] == '\n') {
        printf("Error: Invalid Input\n");
        continue;
      }
      // Remove trailing \n
      if (length > 0 && cmd.input_buf[length - 1] == '\n') {
        cmd.input_buf[length - 1] = '\0';
      }
    }

    if (strcmp(cmd.input_buf, "exit") == 0) {
      should_run = false;
      continue;
    }

    // Handle clear command
    if (strcmp(cmd.input_buf, "clear") == 0) {
      printf("\033[H\033[J");
      fflush(stdout);
      continue;
    }

    // Run last command
    if (strcmp(cmd.input_buf, "!!") == 0) {
      if (cmd.last_command_buf[0] == 0) {
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

    tokenize_input(&cmd);
    parse_input(&cmd);

#ifdef DEBUG
    // Print contents of args array
    printf("DEBUG: Arguments:\n");
    for (int i = 0; i < cmd.args_length; i++) {
      printf("DEBUG: args[%d]: %s\n", i, cmd.args[i]);
    }
    printf("DEBUG: Background: %s\n", cmd.run_background ? "yes" : "no");
    printf("DEBUG: Redirect In: %s\n", cmd.redirect_in ? "yes" : "no");
    printf("DEBUG: Redirect In File: %s\n",
           cmd.redirect_in_file ? cmd.redirect_in_file : "none");
    printf("DEBUG: Redirect Out: %s\n", cmd.redirect_out ? "yes" : "no");
    printf("DEBUG: Redirect Out File: %s\n",
           cmd.redirect_out_file ? cmd.redirect_out_file : "none");
    printf("DEBUG: Pipe: %s\n", cmd.has_pipe ? "yes" : "no");
#endif

    execute_command(&cmd);

    reset_command(&cmd);
  }
}

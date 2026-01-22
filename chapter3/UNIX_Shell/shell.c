#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXLINE 80
#define BUFFER_LENGTH 256

struct Command {
  char input_buf[BUFFER_LENGTH];
  char last_command_buf[BUFFER_LENGTH];
  char *args[MAXLINE + 1];
  int args_length;
  bool run_background;
  bool redirect_out;
  char *redirect_out_file;
  bool redirect_in;
  char *redirect_in_file;
  bool has_pipe;
};

int tokenize_input(struct Command *cmd);
int parse_input(struct Command *cmd);
int execute_command(struct Command *cmd);

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

    // clear cmd.input_buf
    memset(cmd.input_buf, 0, sizeof(cmd.input_buf));
  }
}

int tokenize_input(struct Command *cmd) {
  char *token;
  token = strtok(cmd->input_buf, " ");
  int i = 0;
  while (token != NULL) {
#ifdef DEBUG
    printf("DEBUG: Token: %s\n", token);
#endif
    cmd->args[i++] = token;
    token = strtok(NULL, " ");
  }

  cmd->args[i] = NULL;
  cmd->args_length = i;
  return i;
}

int parse_input(struct Command *cmd) {
  // Check if last argument is "&" for background execution
  if (cmd->args_length > 0 &&
      strcmp(cmd->args[cmd->args_length - 1], "&") == 0) {
    cmd->run_background = true;
    cmd->args[cmd->args_length - 1] = NULL;
    cmd->args_length--;
  }

  if (cmd->args_length > 0) {
    // Check if args contains "<" or ">" for input/output redirection
    for (int i = 0; i < cmd->args_length; i++) {

      if (cmd->args[i] == NULL)
        continue;

      if (!cmd->redirect_in && strcmp(cmd->args[i], "<") == 0) {
        if (i + 1 >= cmd->args_length || cmd->args[i + 1] == NULL) {
          printf("Error: No input file specified for redirection\n");
          return -1;
        }
        cmd->redirect_in = true;
        cmd->redirect_in_file = cmd->args[i + 1];
        cmd->args[i] = NULL;
        cmd->args[i + 1] = NULL;

      } else if (!cmd->redirect_out && strcmp(cmd->args[i], ">") == 0) {
        if (i + 1 >= cmd->args_length || cmd->args[i + 1] == NULL) {
          printf("Error: No output file specified for redirection\n");
          return -1;
        }
        cmd->redirect_out = true;
        cmd->redirect_out_file = cmd->args[i + 1];
        cmd->args[i] = NULL;
        cmd->args[i + 1] = NULL;
      }
    }

    // Compact the args array by removing NULLs
    int write_idx = 0;
    for (int read_idx = 0; read_idx < cmd->args_length; read_idx++) {
      if (cmd->args[read_idx] != NULL) {
        cmd->args[write_idx++] = cmd->args[read_idx];
      }
    }
    cmd->args[write_idx] = NULL;
    cmd->args_length = write_idx;
  }
  return 0;
}

int execute_command(struct Command *cmd) {
  pid_t pid = fork();
  if (pid == -1) {
    perror("fork");
    return -1;
  }

  if (pid == 0) {
    // child process
    execvp(cmd->args[0], cmd->args);
  } else {
    // parent process
    if (!cmd->run_background) {
      wait(NULL);
    }
  }
  return 0;
}

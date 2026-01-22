#include "shell.h"

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
    if (cmd->redirect_in) {
      int fd = open(cmd->redirect_in_file, O_RDONLY);
      if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
      }
      dup2(fd, STDIN_FILENO);
      close(fd);
    }

    if (cmd->redirect_out) {
      int fd = open(cmd->redirect_out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
      }
      dup2(fd, STDOUT_FILENO);
      close(fd);
    }
    execvp(cmd->args[0], cmd->args);
  } else {
    // parent process
    if (!cmd->run_background) {
      wait(NULL);
    }
  }
  return 0;
}

void reset_command(struct Command *cmd) {
  cmd->args_length = 0;
  cmd->args[0] = NULL;
  cmd->redirect_in = false;
  cmd->redirect_in_file = NULL;
  cmd->redirect_out = false;
  cmd->redirect_out_file = NULL;
  cmd->run_background = false;
  memset(cmd->input_buf, 0, sizeof(cmd->input_buf));
}

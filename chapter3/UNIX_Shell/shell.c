#include "shell.h"
#include <unistd.h>
#define READ_END 0
#define WRITE_END 1

#ifdef DEBUG
void debug_command(struct Command *cmd) {
  // Debug Command
  printf("DEBUG: Arguments:\n");
  for (int i = 0; i < cmd->args_length; i++) {
    printf("DEBUG: args[%d]: %s\n", i, cmd->args[i]);
  }
  printf("DEBUG: Background: %s\n", cmd->run_background ? "yes" : "no");
  printf("DEBUG: Redirect In: %s\n", cmd->redirect_in ? "yes" : "no");
  printf("DEBUG: Redirect In File: %s\n",
         cmd->redirect_in_file ? cmd->redirect_in_file : "none");
  printf("DEBUG: Redirect Out: %s\n", cmd->redirect_out ? "yes" : "no");
  printf("DEBUG: Redirect Out File: %s\n",
         cmd->redirect_out_file ? cmd->redirect_out_file : "none");
  printf("DEBUG: Num of pipes: %d\n", cmd->num_pipes);
  printf("DEBUG: Pipe command count: %d\n", cmd->pipe_cmd_count);
  for (int i = 0; i < cmd->pipe_cmd_count; i++) {
    printf("DEBUG: pipe_cmds[%d] full command: ", i);
    for (int j = 0; cmd->pipe_cmds[i][j] != NULL; j++) {
      printf("%s ", cmd->pipe_cmds[i][j]);
    }
    printf("\n");
  }
}
#endif

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
    // Add first command to pipe_cmds array
    cmd->pipe_cmds[0] = &cmd->args[0];
    cmd->pipe_cmd_count++;
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

    // Parse pipes
    for (int i = 0; i < cmd->args_length; i++) {
      if (strcmp(cmd->args[i], "|") == 0) {
        cmd->pipe_cmds[cmd->pipe_cmd_count++] = &cmd->args[i + 1];
        cmd->args[i] = NULL;
        cmd->num_pipes++;
      }
    }
  }
  return 0;
}

int execute_command(struct Command *cmd) {
  int pipes[cmd->num_pipes][2];
  for (int i = 0; i < cmd->num_pipes; i++) {
    if (pipe(pipes[i]) == -1) {
      perror("Pipe");
      return -1;
    }
  }
  for (int i = 0; i < cmd->pipe_cmd_count; i++) {
    pid_t pid = fork();
    if (pid == -1) {
      perror("fork");
      return -1;
    }

    if (pid == 0) {
      // child process

      // Only first command can have a input redirect "<"
      if (i == 0 && cmd->redirect_in) {
        int fd = open(cmd->redirect_in_file, O_RDONLY);
        if (fd == -1) {
          perror("open");
          exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
      } else if (i > 0) {
        dup2(pipes[i - 1][READ_END], STDIN_FILENO);
      }

      // Only last command can have a output redirect ">"
      if (i == cmd->pipe_cmd_count - 1 && cmd->redirect_out) {
        int fd =
            open(cmd->redirect_out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
          perror("open");
          exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
      } else if (i < cmd->pipe_cmd_count - 1) {
        dup2(pipes[i][WRITE_END], STDOUT_FILENO);
      }

      // Close all pipes
      for (int j = 0; j < cmd->num_pipes; j++) {
        close(pipes[j][READ_END]);
        close(pipes[j][WRITE_END]);
      }

      execvp(cmd->pipe_cmds[i][0], cmd->pipe_cmds[i]);
      exit(1);
    }
  }
  // Close all pipes
  for (int j = 0; j < cmd->num_pipes; j++) {
    close(pipes[j][READ_END]);
    close(pipes[j][WRITE_END]);
  }

  if (!cmd->run_background) {
    for (int i = 0; i < cmd->pipe_cmd_count; i++)
      wait(NULL);
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
  cmd->pipe_cmd_count = 0;
  cmd->num_pipes = 0;
  memset(cmd->pipe_cmds, 0, sizeof(cmd->pipe_cmds));
  memset(cmd->input_buf, 0, sizeof(cmd->input_buf));
}

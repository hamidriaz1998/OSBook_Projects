#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXLINE 80
#define BUFFER_LENGTH 256

struct Command {
  ////////// INPUT //////////
  char input_buf[BUFFER_LENGTH];
  char last_command_buf[BUFFER_LENGTH];
  ////////// ARGS //////////
  char *args[MAXLINE];
  int args_length;
  bool run_background;
  ////////// REDIRECTS //////////
  bool redirect_out;
  char *redirect_out_file;
  bool redirect_in;
  char *redirect_in_file;
  ////////// PIPE //////////
  int num_pipes;
  char **pipe_cmds[MAXLINE];
  int pipe_cmd_count;
};

#ifdef DEBUG
void debug_command(struct Command *cmd);
#endif

int tokenize_input(struct Command *cmd);
int parse_input(struct Command *cmd);
int execute_command(struct Command *cmd);
void reset_command(struct Command *cmd);

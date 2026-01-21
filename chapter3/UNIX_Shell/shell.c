#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXLINE 80
#define BUFFER_LENGTH 256

int parse_input(char *input_buffer, char *args[]);

int main() {
  char input_buffer[BUFFER_LENGTH];
  char last_command_buffer[BUFFER_LENGTH] = {0};
  bool should_run = 1;
  char *args[MAXLINE + 1];

  while (should_run) {
    printf("osh> ");
    fflush(stdout);
    if (fgets(input_buffer, sizeof(input_buffer), stdin) == NULL) {
      printf("Error: Invalid Input");
      continue;
    } else {
      size_t length = strlen(input_buffer);
      // Check for empty input
      if (length == 1 && input_buffer[0] == '\n') {
        printf("Error: Invalid Input\n");
        continue;
      }
      // Remove trailing \n
      if (length > 0 && input_buffer[length - 1] == '\n') {
        input_buffer[length - 1] = '\0';
      }
    }

    if (strcmp(input_buffer, "exit") == 0) {
      should_run = false;
      continue;
    }

    // Handle clear command
    if (strcmp(input_buffer, "clear") == 0) {
      printf("\033[H\033[J");
      fflush(stdout);
      continue;
    }
    
    // Run last command
    if (strcmp(input_buffer, "!!") == 0){
        if (last_command_buffer[0] == 0) {
          printf("Error: No commands in history.\n");
          continue;
        } else {
          strcpy(input_buffer, last_command_buffer);
          printf("Running command: %s\n", last_command_buffer);
        }
    }

    // Save the command to last_command_buffer before parsing
    memset(last_command_buffer, 0, sizeof(last_command_buffer));
    strcpy(last_command_buffer, input_buffer);

    int args_length = parse_input(input_buffer, args);

    // Check if last argument is "&" for background execution
    bool run_background = false;
    if (args_length > 0 && strcmp(args[args_length - 1], "&") == 0) {
      run_background = true;
      args[args_length - 1] = NULL;
      args_length--;
    }

#ifdef DEBUG
    // Print contents of args array
    printf("DEBUG: Arguments:\n");
    for (int i = 0; i < args_length; i++) {
      printf("DEBUG: args[%d]: %s\n", i, args[i]);
    }
    printf("DEBUG: Background: %s\n", run_background ? "yes" : "no");
#endif

    if (fork() == 0) {
      // child process
      execvp(args[0], args);
    } else {
      if (!run_background) {
        wait(NULL);
      }
    }

    // clear input_buffer
    memset(input_buffer, 0, sizeof(input_buffer));
  }
}

int parse_input(char *input_buffer, char *args[]) {
  char *token;
  token = strtok(input_buffer, " ");
  int i = 0;

  while (token != NULL) {
#ifdef DEBUG
    printf("DEBUG: Token: %s\n", token);
#endif
    args[i++] = token;
    token = strtok(NULL, " ");
  }

  args[i] = NULL;
  return i;
}

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>




/*
  Function Declarations for builtin shell commands:
 */
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);



/*
  List of builtin commands.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};




int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit
};



int shell_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}



/*
  Builtin function implementations.
*/

/*
   chdir Bultin command: change directory.
   List of args.  args[0] is "cd".  args[1] is the path/directory.
   return 0 if error otherwise always returns 1, to continue executing.
 */
int shell_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "shell: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("shell");
    }
  }
  return 1;
}





int shell_help(char **args)
{
  int i;
  printf("Navin Anirudh SHELL\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < shell_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}







int shell_exit(char **args)
{
  return 0;
}






/*
   We actually start a new process and run the tokens which we have given as input
 */
int shell_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("shell");
  } else {
    // Parent process
    // prevention of zombie process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}






/*
   Execute shell built-in or launch program.
   args Null terminated list of arguments.
   return 1 if the shell should continue running, 0 if it should terminate
 */
int shell_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < shell_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return shell_launch(args);
}





/*
   This function basically reads lines from the user input.
   we basically implement this with two methods.
   * The first method is implemented using a new approach which has a inbuilt function in c getline() which reads the input from the user and allocates
     memory space according to it.
   * The second method we use a normal method of getting input from the user where we run a while loop and get the user input and store it in a buffer if it comes to a
     EOF or \n \0 it exits and return the command if the buffer size is full we reallocate the buffer and continue.
*/

char *shell_read_line(void)
{

  //first method of reading a line
 /* char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We received an EOF
    } else  {
      perror("shell: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line; */




//reading line using buffer
  #define SHELL_RL_BUFSIZE 1024

  int bufsize = SHELL_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += SHELL_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}





/*
   This Split function actually splits the line which the user has given input in the command line we need to seprate it into a list of arguments.
   We tokenize the commands using white spaces instead of backslash or using quotes we store these tokens in a buffer asa same as read_line().
   Each token has been seperated by a \0 terminated charecter to distinguish it this is actually done by strtok().
*/

#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\n"

char **shell_split_line(char *line)
{
  int bufsize = SHELL_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, SHELL_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SHELL_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
                free(tokens_backup);
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SHELL_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}





//Main function
int main(int argc, char **argv)
{

  char *line;
  char **args;
  int status;
 /*  we generally do the following set of functions.
   Read: Reads the user input command from standard input.
   Parse: Seperate the command string into a program and arguments.
   Execute: Run the parsed command.
*/
// run command loop
printf("Type help for more information\nDocumentation : https://help.ubuntu.com \n");
  do {
    printf("#+#:> ");
    line = shell_read_line();
    args = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);


  // Perform any shutdown/cleanup.
  return EXIT_SUCCESS;
}

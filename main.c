#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <signal.h>
#include "list.h"
#define EXEC_FILE_MAX_SIZE 500
#define FILE_LINE_MAX 5000

/*
 * Runs bg command.
 *
 * @param args: null-terminate array of strings
 * @param programs: pointer to list of programs running
 */
void bg(char **args, list *programs) {
  if (args[0] == NULL) {
    // check at least 1 argument
    printf("Usage: bg <program> [args]\n");
    return;
  }
  
  pid_t pid;
  if ((pid = fork()) == -1) {
    perror("Error. Fork Failed");
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    // child process
    execvp(args[0], args);
    perror("Error. execvp failed");
    exit(EXIT_FAILURE);
  } else {
    // parent process
    node* process = node_init();
    process->pid = pid;
    process->path = strdup(args[0]);
    process->next = NULL;

    list_prepend(programs, process);
  }
}

/**
 * Runs bglist command.
 * @param programs: pointer to list of programs running
 */
void bglist(list *programs) {
  list_print(programs);
}

/*
 * Sends kill() system call with signal number to a pid.
 *
 * @param args: null-terminated list of strings
 * @param programs: pointer to list of programs running
 * @param signal_number: signal number to send
 *
 * @return 0 on success. Non-zero on failure.
 */
int send_signal(char** args, list* programs, int signal_number) {
  if (args[0] == NULL || args[1] != NULL) {
    // check if there are 0 or more than 1 arguments
    printf("Usage: bgkill <pid>\n");
    return -1;
  }
  
  int pid = atoi(args[0]);
  if (pid == 0) {
    // atoi failure
    printf("Usage: bgkill <pid>\n");
    return -1;
  }

  if (!list_search(programs, pid)) { 
    fprintf(stderr, "Process %d does not exist\n", pid);
    return -1;
  }

  return kill((pid_t) pid, signal_number);
}

void bgkill(char** args, list* programs) {
  send_signal(args, programs, SIGKILL);
}

void bgstop(char** args, list* programs) {
  send_signal(args, programs, SIGSTOP);
}

void bgstart(char** args, list* programs) {
  send_signal(args, programs, SIGCONT);
}

void stat(char* pid) {
  // create path /proc/pid/stat
  int path_length = strlen("/proc/") + strlen(pid) + strlen("/stat");
  char path[path_length+1]; // +1 for string terminator
  path[0] = '\0';
  strcat(path, "/proc/");
  strcat(path, pid);
  strcat(path, "/stat");

  // open file at path
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening file %s\n", path);
    exit(EXIT_FAILURE);
  }

  double ticks = (double) sysconf(_SC_CLK_TCK);
  char comm[EXEC_FILE_MAX_SIZE]; 
  char state;
  float utime, stime;
  long int rss;

  // read from /proc/pid/stat
  char* stat_format_specifier = "%*d %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %f %f %*d %*d %*d %*d %*d %*d %*u %*u %ld";
  fscanf(file, stat_format_specifier, comm, &state, &utime, &stime, &rss);
  printf("comm: %s\nstate: %c\nutime: %f\nstime: %f\nrss: %ld\n", comm, state, utime/ticks, stime/ticks, rss);

  fclose(file); 
}

void status(char* pid) {
  int path_length = strlen("/proc/") + strlen(pid) + strlen("/status");
  char path[path_length+1];
  path[0] = '\0';
  strcat(path, "/proc/");
  strcat(path, pid);
  strcat(path, "/status");

  // open file at path
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening file %s\n", path);
    exit(EXIT_FAILURE);
  }

  char line[FILE_LINE_MAX]; 
  char* VOLUNTARY_CTXT_SWITCHES = "voluntary_ctxt_switches:";
  char* NONVOLUNTARY_CTXT_SWITCHES = "nonvoluntary_ctxt_switches:";
  int context_switches;

  while(fgets(line, FILE_LINE_MAX, file)) {
    if (strncmp(line, VOLUNTARY_CTXT_SWITCHES, strlen(VOLUNTARY_CTXT_SWITCHES)) == 0){
      sscanf(line, "%*s %d", &context_switches); 
      printf("%s %d\n", VOLUNTARY_CTXT_SWITCHES, context_switches);
    }
    else if (strncmp(line, NONVOLUNTARY_CTXT_SWITCHES, strlen(NONVOLUNTARY_CTXT_SWITCHES)) == 0){
      sscanf(line, "%*s %d", &context_switches);
      printf("%s %d\n", NONVOLUNTARY_CTXT_SWITCHES, context_switches);
    }
  }

  fclose(file);
}

/*
 * Runs pstat program.
 */
void pstat(char** args, list* programs) {
  if (args[0] == NULL || args[1] != NULL) {
    // check exactly 1 argument
    printf("Usage pstat <pid>\n");
    return;
  }

  pid_t pid = (pid_t)(atoi(args[0]));
  if (list_search(programs, pid) != NULL) {
    stat(args[0]);
    status(args[0]);
  } else {
    fprintf(stderr, "Error: Process %s does not exist.\n", args[0]);
  }
}

/*
 * Returns null-terminated array of strings from a space separated 
 * string of tokens.
 * Array is allocated on heap.
 */
char** tokenize(char *str) {
  char **res = NULL;
  char *p = strtok(str, " ");
  int n_spaces = 0, i;
  
  while (p) {
    res = realloc(res, sizeof(char*) * ++ n_spaces);
    if (res == NULL) {
      exit(-1); // malloc failed
    }

    res[n_spaces-1] = p;
    p = strtok(NULL, " ");
  }
  res = realloc(res, sizeof (char*) * (n_spaces+1));
  res[n_spaces] = NULL;
  
  return res;
}

/*
 * Removes zombie processes when they are terminated.
 */
void remove_zombies(list* programs) {
  pid_t pid;
  int status;

  // wait for any process to change state
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
    if (list_search(programs, pid) != NULL) {
      list_delete(programs, pid);
      printf("Process %d has been terminated\n", pid);
    }
  }
}

int main(){

  char* input = NULL ;
  char* prompt = "PMan: > ";

  list* programs = list_init();

  while (1) {
    remove_zombies(programs);

    input = readline(prompt);
    if (input == NULL) continue;
 
    char** args = tokenize(input);
    if (args[0] == NULL) continue;

    if (strcmp(args[0], "bg") == 0) {
      bg(args + 1, programs); 
    } else if (strcmp(args[0], "bglist") == 0) {
      bglist(programs);
    } else if (strcmp(args[0], "bgkill") == 0) {
      bgkill(args+1, programs);
    } else if (strcmp(args[0], "bgstop") == 0) {
      bgstop(args+1, programs);
    } else if (strcmp(args[0], "bgstart") == 0) {
      bgstart(args+1, programs);
    } else if (strcmp(args[0], "pstat") == 0) {
      pstat(args+1, programs);
    } else {
      printf("%s %s: command not found\n", prompt, args[0]);
      continue;
    }

    free(args); 
  }

  list_end(programs);

  return 1;
}




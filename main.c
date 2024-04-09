/* Testing and data collection
  Objective: using processes, we are going to run "AISolver", with all three algorithms, passed in as argsments. We will run the program 100 times, and collect the data.
  AISolver returns the data back via STDOUT, we will pipe this information back into a file for each algorithm.
  The 3 output files with be csvs with the following columns:
  - Attempt Number
  - Win/Loss
  - Final Score
  - Highest tile
  - Time elapsed
  - Number of moves

  example process to run: AISolver -a minimax -n 1 -d 1 -p 3
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// Batch size is determined at runtime now
pid_t *pids;

#define MONTECARLO 0
#define MINIMAX 1
#define EXPECTIMINIMAX 2

int main(int argc, char *argv[]) {
  // check number of inputs
  if (argc != 4) {
    printf("Usage: %s <number of runs> <minimax/expectminimax depth> <monte carlo num of sims>\n", argv[0]);
    exit(1);
  }
  // Define the number of times to run the program
  int numRuns = atoi(argv[1]);
  char* minimaxDepth = argv[2];
  char* monteCarloSims = argv[3];

  // Define the output file names
  char *outputFiles[] = {"output/montecarlo.csv", "output/minimax.csv", "output/expectiminimax.csv"};

  // Define the algorithms to run
  char *algorithms[] = {"montecarlo", "minimax", "expectiminimax"};

  // Create an array to store the child process IDs
  pids = malloc(sizeof(pid_t) * numRuns);

  // track how many processes for each algorithm is complete
  int processCount[] = {numRuns, numRuns, numRuns};

  // Loop through each algorithm
  for (int i = 0; i < 3; i++) {
    // Open the output file for writing
    FILE *outputFile = fopen(outputFiles[i], "w+");
    if (outputFile == NULL) {
      perror("Error opening output file");
      exit(1);
    }
    // Redirect STDOUT to the output file
    dup2(fileno(outputFile), STDOUT_FILENO);
    printf("Win/Loss, Final Score, Highest Tile, Number of Moves, Time Elapsed\n");
    fflush(stdout);

    // Loop through each run
    for (int j = 0; j < numRuns; j++) {
      // Create a child process
      pid_t pid = fork();
      
      if (pid == -1) {
        perror("Error creating child process");
        exit(1);
      } else if (pid == 0) {
        fprintf(stderr, "ENTER: (%d) process for %s\n", getpid(), algorithms[i]);
        // Child process
        if (i == MONTECARLO) {
          // Execute the AISolver program with the current algorithm and arguments
          execl("./AISolver", "AISolver", "-a", algorithms[i], "-n", "1", "-r", monteCarloSims, "-p", "0", NULL);
        } else {
          // Execute the AISolver program with the current algorithm and arguments
          execl("./AISolver", "AISolver", "-a", algorithms[i], "-n", "1", "-d", minimaxDepth, "-p", "0", NULL);
        }

        // If execl returns, there was an error
        perror("Error executing AISolver");
        exit(1);
      } else {
        // Parent process
        pids[j] = pid;
      }
    }
    // fprintf(stderr, "Waiting for all child processes to finish for %s\n", algorithms[i])

    // Wait for all child processes to finish
    for (int j = 0; j < numRuns; j++) {
      int status;
      pid_t exitedPid = waitpid(pids[j], &status, 0);
      if (exitedPid > 0) {
        processCount[i]--;
        fprintf(stderr, "EXIT:  (%d) %d left to complete for %s\n", exitedPid, processCount[i], algorithms[i]);
      } else {
        perror("Error waiting for child process");
      }
    }

    // Close the output file
    fclose(outputFile);
  }

  // Free the memory allocated for the child process IDs
  free(pids);

  return 0;
}
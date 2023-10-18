#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <time.h>


// TODO: Add more fields to this struct
struct job {
    int id;
    int arrival;
    int length;
    int tickets;
    struct job *next;
    
    // For analyze
    int responseTime;
    int turnAround;
    int wait;
    int timeSinceWait; // the time after last time job ran
};

bool allDone(struct job *head);

/*** Globals ***/ 
int seed = 100;

//This is the start of our linked list of jobs, i.e., the job list
struct job *head = NULL;

/*** Globals End ***/

/*Function to append a new job to the list*/
void append(int id, int arrival, int length, int tickets){
  // create a new struct and initialize it with the input data
  struct job *tmp = (struct job*) malloc(sizeof(struct job));

  //tmp->id = numofjobs++;
  tmp->id = id;
  tmp->length = length;
  tmp->arrival = arrival;
  tmp->tickets = tickets;

  // give the following a base value.
    tmp->responseTime = -1;
    tmp->turnAround = -1;
    tmp->wait = -1;
    tmp->timeSinceWait = 0;
  //

  // the new job is the last job
  tmp->next = NULL;

  // Case: job is first to be added, linked list is empty 
  if (head == NULL){
    head = tmp;
    return;
  }

  struct job *prev = head;

  //Find end of list 
  while (prev->next != NULL){
    prev = prev->next;
  }

  //Add job to end of list 
  prev->next = tmp;
  return;
}


/*Function to read in the workload file and create job list*/
void read_workload_file(char* filename) {
  int id = 0;
  FILE *fp;
  size_t len = 0;
  ssize_t read;
  char *line = NULL,
       *arrival = NULL, 
       *length = NULL;

  struct job **head_ptr = malloc(sizeof(struct job*));

  if( (fp = fopen(filename, "r")) == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) > 1) {
    arrival = strtok(line, ",\n");
    length = strtok(NULL, ",\n");
       
    // Make sure neither arrival nor length are null. 
    assert(arrival != NULL && length != NULL);
        
    append(id++, atoi(arrival), atoi(length), 100);
  }

  fclose(fp);

  // Make sure we read in at least one job
  assert(id > 0);

  return;
}


void policy_STCF(struct job *head, int slice) {
  int time = 0;
  struct job *current;
  printf("Execution trace with STCF:\n");
  while (!allDone(head)){
    struct job *candidate = NULL;
    current = head;
    while((current != NULL) && (current->arrival <= time)){
      if ((current->length > 0) && ((candidate == NULL) || (current->length < candidate->length))){
        candidate = current;
      }
      current = current->next;
    }
    if(candidate != NULL){
      if (candidate->length >= slice){
        /* when length is more than equal ticks. DO with slice length in mind. */
        // ANALYSIS: Set response time and initial wait value
        // >>
        if (candidate->responseTime == -1) 
        {
          candidate->responseTime = time - candidate->arrival;
          candidate->wait = candidate->responseTime;
        }
        // <<
        // ANALYSIS: wait increments for in-between jobs
        // >>
        else{
          candidate->wait += time - candidate->timeSinceWait;
        }
        // <<
        printf("t=%d,\t[Job %d] arrived at [%d], ran for: [%d]\n",
           time, candidate->id, candidate->arrival, slice);
        time += slice;
        candidate->length -= slice;
        // ANALYSIS: Record time for timeSinceWait
        // >>
        candidate->timeSinceWait = time;
        // <<
      } else {
        // when length is less than ticks. Do normally.
        // ANALYSIS: Final wait increment
        // >>
        candidate->wait += time - candidate->timeSinceWait;
        // <<
        printf("t=%d,\t[Job %d] arrived at [%d], ran for: [%d]\n",
           time, candidate->id, candidate->arrival, candidate->length);

        time += candidate->length;
        candidate->length -= candidate->length;
      }
      if (candidate->length == 0)
      {
        // ANALYSIS: Set turnAround time
        // >>
        candidate->turnAround = time - candidate->arrival;
        // <<
      }
    } else{
      time++;
    }
  }
  printf("End of execution with STCF\n");
  return;
}

bool allDone(struct job *head){
  /*
    Promise:  Returns true if length of all jobs are more than zero. Returns false otherwise.
    
    Parameter:
      struct job *head: head of the linked list.

    Returns:
      boolean true or false.
  */

  struct job *current = head;
  while (current != NULL){
    if (current->length > 0){
      return false;
    }
    current = current->next;
  }
  return true;
}


void policy_RR(struct job *head, int slice) {
  int time = 0;
  struct job *current;
  struct job *candidate;
  bool noJumpNeeded;
  bool repeat = false;
  printf("Execution trace with RR:\n");
  while (!allDone(head)){
    current = head;
    while(current != NULL){
      if ((current->length > 0) && (current->arrival <= time)){
        noJumpNeeded = true;
        if (candidate == current)
        {
          repeat = true;
        } 
        else 
        {
          repeat = false;
        }
        candidate = current;
        if (candidate->length >= slice){
          /* when length is more than equal ticks. DO with slice length in mind. */
          // ANALYSIS: Set response time and initial wait value
          // >>
          if (candidate->responseTime == -1) 
          {
            candidate->responseTime = time - candidate->arrival;
            candidate->wait = candidate->responseTime;
          }
          // <<
          // ANALYSIS: wait increments for in-between jobs
          // >>
          else{
            if (repeat == false)
            {
              candidate->wait += time - candidate->timeSinceWait;
            } 
          }
          // <<
          printf("t=%d,\t[Job %d] arrived at [%d], ran for: [%d]\n",
            time, candidate->id, candidate->arrival, slice);
          time += slice;
          candidate->length -= slice;
          // ANALYSIS: Record time for timeSinceWait
          // >>
          candidate->timeSinceWait = time;
          // <<
        } else {
          // when length is less than ticks. Is it the first time it shows up.
          if (candidate->responseTime != -1)
          {
            // ANALYSIS: Final wait increment
            // >>
            if (repeat == false)
              candidate->wait += time - candidate->timeSinceWait;
            // <<
          } else
          // first time doing job and length < slice
          {
            candidate->responseTime = time - candidate->arrival;
            candidate->wait = candidate->responseTime;
          }
          printf("t=%d,\t[Job %d] arrived at [%d], ran for: [%d]\n",
            time, candidate->id, candidate->arrival, candidate->length);

          time += candidate->length;
          candidate->length -= candidate->length;
        }
        if (candidate->length == 0)
        {
          // ANALYSIS: Set turnAround time
          // >>
          candidate->turnAround = time - candidate->arrival;
          // <<
        }
      }
      current = current->next;
    }
    if (noJumpNeeded == false)
      time++;
  }
  printf("End of execution with RR\n");
  return;
}

void policy_LT(struct job *head, int slice)
{
  int timeNow = 0;
  struct job *current = head;
  int lotteryLimit = 0;

  while (current != NULL){
    lotteryLimit += current->tickets;
    current = current->next;
  }

  srand(time(NULL));


  printf("Execution trace with LT:\n");
  struct job * candidate;
  int lottery;
  while (!allDone(head)){
    candidate = NULL;
    current = head;
    lottery = rand() % lotteryLimit;

    while((current != NULL) && (current->arrival <= timeNow)){
      if ((current->length > 0) && (lottery <= current-> tickets)){
        candidate = current;
        break;
      }
      lottery -= current->tickets;
      current = current->next;
    }
    if(candidate != NULL){
      if (candidate->length > slice){
        /* when length is more than equal ticks. DO with slice length in mind. */
        // ANALYSIS: Set response time and initial wait value
        // >>
        if (candidate->responseTime == -1) 
        {
          candidate->responseTime = timeNow - candidate->arrival;
          candidate->wait = candidate->responseTime;
        }
        // <<
        // ANALYSIS: wait increments for in-between jobs
        // >>
        else{
          candidate->wait += timeNow - candidate->timeSinceWait;
        }
        // <<
        printf("t=%d,\t[Job %d] arrived at [%d], ran for: [%d]\n",
           timeNow, candidate->id, candidate->arrival, slice);
        timeNow += slice;
        candidate->length -= slice;
        candidate->tickets += 100;
        lotteryLimit += 100;
        // ANALYSIS: Record time for timeSinceWait
        // >>
        candidate->timeSinceWait = timeNow;
        // <<
      } else {
        // when length is less than ticks. Do normally.
        // ANALYSIS: Final wait increment
        // >>
        candidate->wait += timeNow - candidate->timeSinceWait;
        // <<
        printf("t=%d,\t[Job %d] arrived at [%d], ran for: [%d]\n",
           timeNow, candidate->id, candidate->arrival, candidate->length);

        timeNow += candidate->length;
        // ANALYSIS: Set turnAround time
        // >>
        candidate->turnAround = timeNow - candidate->arrival;
        // <<
        candidate->length -= candidate->length;
        lotteryLimit -= candidate->tickets;
        candidate->tickets = 0;
      }
    } else{
      timeNow++;
    }
  }
  printf("End of execution with LT\n");
  return;
}

void printAnalysis(struct job *head) {
  int totalRT = 0;
  int totalTA = 0;
  int totalWait = 0;
  int jobs = 0;
  
  struct job *current = head;
  while (current != NULL)
  {
    printf("Job %d -- Response time: %d Turnaround: %d Wait: %d\n"
      , current->id, current->responseTime, current->turnAround, current->wait);
    totalRT += current->responseTime;
    totalTA += current->turnAround;
    totalWait += current->wait;
    jobs++;
    current = current->next;
  }
  printf("Average -- Response time: %.2f Turnaround: %.2f Wait: %.2f\n"
    , (float)totalRT / jobs, (float)totalTA / jobs, (float)totalWait / jobs);

  return;
}

void analyze_STCF(struct job *head)
{
  printAnalysis(head);
  return;
}

void analyze_LT(struct job *head)
{
  printAnalysis(head);
  return;
}

void analyze_RR(struct job *head)
{
  printAnalysis(head);
  return;
}

int main(int argc, char **argv) {

 if (argc < 5) {
    fprintf(stderr, "missing variables\n");
    fprintf(stderr, "usage: %s analysis-flag policy workload-file slice-length\n", argv[0]);
		exit(EXIT_FAILURE);
  }

  int analysis = atoi(argv[1]);
  char *policy = argv[2],
       *workload = argv[3];
  int slice = atoi(argv[4]);

  // Note: we use a global variable to point to 
  // the start of a linked-list of jobs, i.e., the job list 
  read_workload_file(workload);

  if (strcmp(policy, "STCF") == 0 ) {
    policy_STCF(head, slice);
    if (analysis) {
      printf("Begin analyzing STCF:\n");
      analyze_STCF(head);
      printf("End analyzing STCF.\n");
    }

    exit(EXIT_SUCCESS);
  } else if (strcmp(policy, "LT") == 0 ) {
    policy_LT(head, slice);
    if (analysis) {
      printf("Begin analyzing LT:\n");
      analyze_LT(head);
      printf("End analyzing LT.\n");
    }

    exit(EXIT_SUCCESS);
  } else if (strcmp(policy, "RR") == 0 ) {
    policy_RR(head, slice);
    if (analysis) {
      printf("Begin analyzing RR:\n");
      analyze_RR(head);
      printf("End analyzing RR.\n");
    }

    exit(EXIT_SUCCESS);
  }

	exit(EXIT_SUCCESS);
}

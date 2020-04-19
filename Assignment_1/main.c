/******************************************************************************
  @file         main.c
  @author       Ahmed   
  @date         Saturday,  11 April 2020
  @brief        Simple Shell
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


/*
  Macros
 */

#define USRdefCMD            sizeof(user_def_cmd) / sizeof(char *)           // number of the User defined commands
#define MAXCOM                200                                              // max number of letters to be supported
#define MAXLIST               50                                               // max number of commands to be supported in one line
#define MAXDIR                200                                               // max length of directory to be supported


/*
  Functions Declarations
 */
char *init_shell(void);
void clear_log_file(void);
char *read_line(void);
char **parse(char *input);
int execute_cmd(char **args);
void signal_handler(int signum);
void shorten_current_directory(void);
void loop(char* username);


/*
  List of User defined commands
 */
char *user_def_cmd[] = { "cd","exit" };


/*
  Global Variables
 */
int background_flag = 0;                    //  Make sure that execution is in Foreground at the beginning
char current_directory[MAXDIR];             //  A string to hold the current directory during execution
char file_directory[MAXDIR];                //  A string to hold the Log file directory during execution


/**
   @brief Main entry point.
 */
int main()
{
  //call initialization function which does some necessary tasks at the beginning of execution also it Stores the User-name
  char *username = init_shell();

  // Read the Username and  Run the Read-Evaluate-Print loop, the Whole execution occurs inside this loop
  loop(username);

  //    Return when the Loop is terminated
  return EXIT_SUCCESS;
}



/*
  Functions Definitions
 */
/**
  @brief Performs some needed startup tasks and sends a greeting message
  @return User-name
 */

char *init_shell(void)
{
    //  Get the User name
    char* username = getenv("USER");

    //  Get the Current working directory
    getcwd(current_directory, sizeof(current_directory));

    //  Store the Shell Working directory
    strcpy(file_directory,current_directory);
    // append the file name to the directory to get the Log file directory
    strncat(file_directory, "/Log_file.txt", 13);

    //  Make the Current directory shorter to look like the Ubuntu Terminal
    shorten_current_directory();


    //  Print a Starting Message at the Beginning
    printf("*****************************"
        "**********************************************");
    printf("\n\n\n***************\t\t  Welcome to Our Shell \t\t******************");
    printf("\n\n\n\n*******************"
        "********************************************************\n\n\n\n");

    //  Clear the Log file each time you start the Shell
    clear_log_file();

    //  Return the username of the Machine
    return username;
}




/**
  @brief It clears the Log file at the beginning of running the Shell each time
 */
void clear_log_file(void)
{
    // pointer file_ptr to FILE
    FILE* file_ptr;

    // Opens a file "Log_file" or  creates is if it doesn't exist, with file acccess as write mode
    file_ptr = fopen(file_directory, "w");

    // Overwrite the content of the file with empty string to Clear the file
    fprintf(file_ptr, " %s ", "");

    // closes the file pointed by file_ptr
    fclose(file_ptr);
}


/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *read_line(void)
{
    //  Allocate memory for String of maximum number of characters equal to MAXCOM to Hold the Entered Line
    char *line = malloc(sizeof(char) * MAXCOM);

    //  Read the input from User and Check whether the User Entered Ctrl+d
    if((fgets(line,MAXCOM,stdin)) == NULL)
    {
        //   If the User Entered Ctrl+d , exit from Shell
        exit(0);
    }

    //  remove '\n' at the end of string
    int i =0;
    while(line[i] != '\n')
    i++;
    line[i] = '\0';

    //  Return the entered line by the user
    return line;
}


/**
   @brief Parse a line into tokens
   @param The line entered by the User
   @return Null-terminated array of tokens.
 */

char **parse(char *input)
 {
    //  Allocate memory for Array of Strings of maximum number of Strings equal to MAXLIST to Hold the Entered Commands
    char **command = malloc(MAXLIST * sizeof(char *));

    //  Handling if Malloc failed
    if (command == NULL)
    {
        perror("Malloc failed");
        exit(1);
    }

    //  Declare some variables used for parsing
    char *separator = " ";
    char *parsed;
    int index = 0;

    //  Parse the line separated by spaces into Tokens
    parsed = strtok(input, separator);
    while (parsed != NULL)
    {
        command[index] = parsed;
        index++;
        parsed = strtok(NULL, separator);
    }

    //  Add NULL at the end of the array
    command[index] = NULL;


    //  Check if the user wants to execute this Command in Background by Looking for the character '&'
    for (int i = 0; i < index; i++)
    {

        if (strcmp(command[i], "&") == 0)
        {
            //  This means the User wants this Process in Background
            command[i] = NULL;                              //  Overwrite the character '&' with Null, this will ignore anything written after it
            background_flag = 1;                            //  Set Background Flag
            break;                                          //  Break out of the for loop
        }

        else
        {
            //  This means the User wants this Process in Foreground
            background_flag = 0;                            //  Reset Background Flag
        }

    }

    //  Return the parsed command
    return command;
}


/**
   @brief Function where Shell Commands are executed.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int execute_cmd(char **args)
{
  // Handling empty command
  if (args[0] == NULL)
  {
    //  If the user entered nothing, return
    return 1;
  }


    // Checking whether a User defined command was Entered
  for (int i = 0; i < USRdefCMD; i++)
  {
        //  Look for the user defined command in the first command entered
        if (strcmp(args[0], user_def_cmd[i]) == 0)
        {
          //switch case for each User defined command
          switch (i) {
            //  The Command "cd" was typed
            case 0:

                //  Look for command arguments
                if (args[1] == NULL)
                 {
                    //  Change the directory to Home
                    chdir(getenv("HOME"));

                    //  Update the Current directory
                    *current_directory = '\0' ;                 //  We made it this way to look like Ubuntu Terminal

                 }

                //  There are other arguments were typed
                 else
                 {
                    //  Change the directory to the specified directory
                    if (chdir(args[1]) != 0)
                    {
                      //  Error occurred , directory wasn't changed
                      perror("Failed to change directory");
                    }

                    else
                    {
                        //  This means the directory was changed successfully

                        //  Get the current directory
                        getcwd(current_directory, sizeof(current_directory));

                        //  Shorten the Current directory to look like the Terminal in Ubuntu
                        shorten_current_directory();

                    }

                  }

                  //    Command was handled, return
                  return 1;


            //  The Command "exit" was typed
            case 1:
                //  Call the function exit
                exit(0);

            //  This case shouldn't happen
            default:
                perror("This shouldn't happen, Exiting the shell ");
                exit(0);
            }
        }
  }

  // No User defined command was called
  pid_t pid, wpid;

  //    Declare a variable to hold the status of execution
  int status;

  //    Make a duplicate of the Parent process into Child process and read the Process ID of the Child
  pid = fork();

  //    Check to see whether we are in Child Process or Parent Process
  if (pid == 0)
  {
    //  This means We are in Child process

    //  Replace the current running program with a new one to the execute the command given by the User and Check for Errors
    if (execvp(args[0], args) == -1)
     {
        //  This means and error occurred
        perror("Error");
     }
    exit(EXIT_FAILURE);
  }

  else
  {
      //    This means We are in Parent process

      //    Call Signal handler that appends the line to the log file when the SIGCHLD signal is received.
           signal(SIGCHLD, signal_handler);

      //    Check for errors in Forking
      if (pid < 0)
      {
            //  Error forking
            perror("Can't Fork, Error occurred");
      }

      else {
                //  This means that forking executed successfully

                // Check for Global Flag Background to see whether the Execution should be in Foreground or Background
                if (!background_flag)
                {
                    //  This means execution should be in the Foreground
                    do {

                           //   The parent has to wait for the child to finish executing the process
                            wpid = waitpid(pid, &status, WUNTRACED);
                        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
                }

           }
      return 1;

  }
}


/**
   @brief Signal handler which handles SIGCHLD Signal by appending the line "Child process was terminated"
   to the log file when the SIGCHLD signal is received.
 */
void signal_handler(int signum)
{
    // pointer file_ptr to FILE
    FILE* file_ptr;

    // Opens a file "Log_file" with file acccess as append mode
    file_ptr = fopen(file_directory, "a");

    // Appends content to the file
    fprintf(file_ptr, " %s ", "Child process was terminated\n ");

    // closes the file pointed by file_ptr
    fclose(file_ptr);
}


/**
   @brief This function keeps the last part of the directory only to be displayed in the Shell, so that it looks like Ubuntu Terminal
 */
void shorten_current_directory(void)
{
    //  Initilizations
    int pos = 0 ;

    // Look for the last "/" in the current directory
    for (int i = strlen(current_directory); i > 0 ; i--)
    {
        if (current_directory[i]  == '/')
        {
            //  This means you found the last '/' in the directory
            pos = i;
            break;
        }
    }


    // Store the last part only of the current directory
    int i,j;

    for (i = pos , j = 0 ; i < strlen(current_directory) ; i++ , j++)
    {
        current_directory[j] = current_directory[i];
    }

    // Add Null at the end of the directory String
    current_directory[j] = '\0';

}


/**
   @brief The Loop gets input and executes it continuously as long as there is no errors and the User didn't type "exit" or pressed "CTRL+d"
 */
void loop(char* username)
{
  char *line;
  char **args;
  int status;

  //    Loop as long as there is no errors or exit Command
  do {
        //  Print this line before any command each time, it also shows the Username and the current working directory
        printf("%s@Our-SimpleShell:~%s$ ", username , current_directory );

        //  Read the entered line
        line = read_line();

        //  Parse the read line into separate tokens
        args = parse(line);

        //  Execute the commands and return the status of the Process to see if there is errors or Exit command was entered
        status = execute_cmd(args);

        //  Freeing some Memory
        free(line);
        free(args);

        //  Check for the condition of the Loop
      } while (status);

}

/*
 * Title: Program 2: Multi-process Text Analysis
 * Description: Determine the number of occurrances of a particular, user-specified word in a set of files.
 * Author: Devon Ozoga
 * Date: 2/14/18
 */
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>

#define MAX_LINE_LENGTH 100

# This is a test
int main(int argc, char *argv[]){
    //Variables used for pipes
	int parent_pipe[2];
    int child_pipe_1[2];
	int child_pipe_2[2];
	
	//Variables used by the parent process
    char buff[100];
	int status = 0;
	
	//Variables for both child processes 
	pid_t child_1, child_2, wait_pid;
	
	//Create pipes for the parent and the two child processes
	if(pipe(parent_pipe) < 0 || pipe(child_pipe_1) < 0|| pipe(child_pipe_2) < 0) {
        perror("Failed to create pipes");
        exit(1);
    }
	//Spawn the first child
    child_1 = fork();
	//Check to see if spawning of the child was successful
    if (child_1 == -1) {
        perror("fork failed");
        exit(1);
    }

    if (child_1 == 0) {
        //This is the first child process. 
		//Reads from child_pipe_1, writes to parent_pipe
		//Loop until sentinel string is entered 
        while(1){
			int in, out, word_count_1;
			in = child_pipe_1[0];
			out = parent_pipe[1];
			
			//Read the first childs pipe for the search word
			read(in, buff, 100);

			//Check for quit condition.
			if (!strcmp("$$", buff)){
				//Close the read side of the first child's pipe
				close(in);
				//Quit.
				exit(0);
			}
			//Open the first text file for reading	
			FILE * file_1 = fopen (argv[1], "r" );
			//Set word count to zero
			word_count_1 = 0; 
			//Used for holding a line of text from the opened text file
			char line[128];
    
			if (file_1 != NULL) {
				while (fgets (line, sizeof(line), file_1) != NULL) { // read a line 
					if (strstr(line, buff)!=NULL){ // word is found in the line
						word_count_1++; // count increased by one 
					}
				}
				//Close the first text file
				fclose (file_1);
			}
			//Write word count to the parent pipe
			write(out, &word_count_1, sizeof(word_count_1));
        }
    }else{
		//Spawn the second child 
		child_2 = fork();
		//Check to see if spawning of the child was successful
		if (child_2 == -1) {
			perror("fork failed");
			exit(1);
		}
		
		if (child_2 == 0) {
			//This is the second child process. Reads from child_pipe_2, writes to parent_pipe
			//Loop until sentinel string is entered 
			while(1){
				int in, out, word_count_2;
				in = child_pipe_2[0];
				out = parent_pipe[1];
				
				//Read the second childs pipe for the search word
				read(in, buff, 100);
				
				//Check for quit condition.
				if (!strcmp("$$", buff)){
					//Close the read side of the second child's pipe
					close(in);
					//Quit.
					exit(0);
				}
				//Open the second text file	for reading
				FILE * file_2 = fopen (argv[2], "r");
				//Set word count to zero
				word_count_2 = 0; 
				//Used for holding a line of text from the opened text file
				char line2 [128];
    
				if (file_2 != NULL) {
					while (fgets (line2, sizeof(line2), file_2) != NULL) { // read a line 
						if (strstr(line2, buff)!=NULL){ // word is found in the line
							word_count_2++; // word count increased by one 
						}
					}
					//Close the second text file
					fclose (file_2);
				}
				//Write word count to the parent pipe
				write(out, &word_count_2, sizeof(word_count_2));
			}
		}else{
				//This is the parent process. 
				//Reads from child_pipe_1 and child_pipe_2, writes to parent_pipe
				int in, out_1,out_2, word_count_1, word_count_2, total_count;
				
				//Variable for line.
				char word[MAX_LINE_LENGTH];
			//Loop until setinel string is entered
			while(1){
				//Receive command from user.
				printf("Enter the search word(type $$ to quit): \n");
				scanf("%s", word);
			
				in = parent_pipe[0];
				out_1 = child_pipe_1[1];
				out_2 = child_pipe_2[1];
				
				//Write the search word to the first child's pipe 
				write(out_1, word, strlen(word) + 1);
				//Write the search word to the second child's pipe 
				write(out_2, word, strlen(word) + 1);
				
				//Check for quit condition.
				if (!strcmp("$$", word)){
					//Wait for all child processes to terminate.
					while ((wait_pid = wait(&status)) > 0);
					//Close all the remaining pipes
					close(out_1);
					close(out_2);
					close(in);
					close(parent_pipe[0]);
					break;
				}
				
				//Read in the word count from the first child process 
				read(in, &word_count_1, sizeof(word_count_1));
				printf("File 1 occurrences: %d\n", word_count_1);
				
				//Read in the word count from the second child process 
				read(in, &word_count_2, sizeof(word_count_2));
				printf("File 2 occurrences: %d\n", word_count_2);
			
				//Add the word count from both child processes
				total_count = word_count_1 + word_count_2;
				
				//Total number of word occurrences.
				printf("Total Word Count: %d\n", total_count);
			}
		}
	}
	return 0;
}
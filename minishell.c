#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <limits.h>

#define MaxLine 128
#define MaxCom 14
#define MaxWord 1000

#define clear() printf("\033[H\033[J")

//Display the help command
void helpCmd(){
	puts("\n******************************\n"
		"\n         HELP COMMANDS\n"
		"\n******************************\n"
		"\n1. ct [filename]                                              - Create file"
		"\n2. rn [filename1] [filename2]                                 - Rename file"
		"\n3. cp [filename1] [filename2]                                 - Copy file "
		"\n4. del [filename]                                             - Delete file"
		"\n5. mv [filename] [dirname]                                    - Move file to other directory"
		"\n6. pwd                                                        - Display current directory"
		"\n7. cd [dirname]                                               - Change the current directory"
		"\n8. ls / ls [dirname]                                          - List all directory"
		"\n9. wordcount [filename] / wordcount [filename1] > [filename2] - Get frequency of each word in file / save to other file"
		"\n10. getcount [filename]                                       - Get all integers in file"
		"\n11. sum [filename]                                            - Sum all integers"
		"\n12. clear                                                     - Clear the screen"
		"\n13. [command] | [command] | [command]                         - Use multiple commands using pipe '|'"
		"\n14. exit / Ctrl+C                                             - Exit the shell");
}

//Create the file
void fileCreate(int argc, char* argv[]){
	if (argc!=2){
		printf("Syntax Error. Use ct [filename]\n");
	}
	else if(access(argv[1], F_OK)<0){
		FILE *fl;
		fl = fopen(argv[1], "w");
	}
	else{
		printf("File existed\n");
	}
}

//Rename the file
void fileRename(int argc, char* argv[]){
	if (argc!=3){
		printf("Syntax Error. Use rn [filename1] [filename2]\n");
	}
	else if(strcmp(argv[1], argv[2])==0){
		printf("Files cannot be same.\n");
	}
	else if(access(argv[1], F_OK)<0){
		printf("File %s does not exist.\n", argv[1]);
	}
	else if(rename(argv[1], argv[2])<0){
		printf("Failed to rename from file %s to file %s\n", argv[1], argv[2]);
	}
}

//Copy the file to another file
void fileCopy(int argc, char* argv[]){
	int status;
	if (argc!=3){
		printf("Syntax Error. Use cp [filename1] [filename2]\n");
	}
	else if(fork()==0){
		execv("/bin/cp", argv);
		exit(0);
	}
	else if(wait(&status)<0){
		printf("Error: wait\n");
	}
}

//Delete the file
void fileDel(int argc, char* argv[]){
	if (argc!=2){
		printf("Syntax Error. Use del [filename]\n");
	}
	else if(access(argv[1], F_OK)<0){
		printf("File %s does not exist\n", argv[1]);
	}
	else if(unlink(argv[1])<0){
		printf("Cannot delete file %s\n", argv[1]);
	}
}

//Move the files to other directory
void fileMove(int argc, char* argv[]){
	int status;
	if (argc!=3){
		printf("Syntax Error. Use mv [filename] [dirname]\n");
		return;
	}
	else if(fork()==0){
		execv("/bin/cp", argv);
	}
	else if(wait(&status)<0){
		printf("Error: wait\n");
		return;
	}
	unlink(argv[1]);
}

//See the current directory
void dirCurr(){
	char cwd[1024];
	getcwd(cwd, sizeof(cwd));
	printf("Current directory: %s\n", cwd);
}

//Change the directory
void dirChange(int argc, char* argv[]){
	if (argc!=2){
		printf("Syntax Error. Use cd [dirname]\n");
	}
	else if(chdir(argv[1])<0){
		printf("Directory %s does not exist\n", argv[1]);
	}
}

//Used to open directory and directory' files
void dirOpen(char* dpath){
	DIR *d;
	struct dirent *dir;
	d=opendir(dpath);
	if(d!=NULL){
		while ((dir=readdir(d))!=NULL){
			printf("%s\n", dir->d_name);
		}
		closedir(d);
	}
	else{
		printf("Directory %s does not exist\n", dpath);
	}
}

//List all the directories or specific directory
void dirList(int argc, char* argv[]){
	if (argc>=3){
		printf("Syntax Error. Use ls / ls [dirname]\n");
	}
	else if(argc==1){
		dirOpen("./");
	}
	else{
		dirOpen(argv[1]);
	}
}

//Get the frequencies of each word in the file and save to new file
void wordCount(int argc, char* argv[]){
	if (argc!=2&&(argc!=4||(strcmp(argv[2], ">")!=0))){
		printf("Syntax Error. Use wordcount [filename] / wordcount [filename1] > [filename2]\n");
	}
	else{
		FILE *fl, *flp;
		int i, len;
		int isUnique;
		int index=0;
		char words[MaxWord][50];
		char word[50];
		char output[MaxWord];
		int count[MaxWord];

		fl = fopen(argv[1],"r");
	
		if(fl==NULL){
			printf("File %s does not exist.\n", argv[1]);
			return;
		}
	
		for(i=0; i<MaxWord; i++){
			count[i] = 0;
		}
				
		while(fscanf(fl, "%s", word)!=EOF){
			len = strlen(word);
			if(ispunct(word[len-1])){
				word[len-1] = '\0';
			}

			isUnique = 1;
			for(i=0; i<index && isUnique; i++){
				if(strcmp(words[i],word)==0){
					isUnique = 0;
				}
			}
		
			if(isUnique){
				strcpy(words[index], word);
				count[index]++;
				index++;
			}
			else{
				count[i-1]++;
			}
		}
	
		fclose(fl);
	
		printf("Frequencies of words in file: \n");
		for(i=0; i<index; i++){
			if (i==0){
				sprintf(output, "%s = %d\n", words[i], count[i]);
			}
			else{
				sprintf(output + strlen(output), "%s = %d\n", words[i], count[i]);
			}
		}
		printf("%s", output);

		if(argc==4){
			flp = fopen(argv[3], "w");
			if(flp==NULL){
				printf("The file existed.\n");
				return;
			}
			fputs(output, flp);
			fclose(flp);
			printf("File %s has been created\n", argv[3]);
		}
	}
}

//Print out all the integers from the file
void getCount(int argc, char* argv[]){
	if(argc!=2){
		printf("Syntax Error. Use getcount [filename]\n");
	}
	else{
		char num;
		unsigned char num1;
		FILE *fl;
		fl = fopen(argv[1], "rt");
		if (fl==NULL) {
    			printf("File %s does not exist.\n", argv[1]);
    			return;
  		}
		while((num=getc(fl))!=EOF){
			num1 = (unsigned char) num;
			if(num1 >= '0' && num1 <= '9'){
				printf("%c\n", num1);
			}
		}
	}
}

//Get the sum of all integers in file
void getSum(int argc, char* argv[]){
	int sum = 0, state = 0, i = 0, dir = 1;
  	unsigned int number = 0, check;

  	if (argc < 2) {
    		printf("Syntax Error. Use sum [filename]\n");
    		return;
  	}

  	FILE* fl = fopen(argv[1], "r");
  	if (fl==NULL) {
    		printf("File %s does not exist.\n", argv[1]);
    		return;
  	}

  	while (i != EOF) {
    		i = fgetc(fl);
    		if (isdigit(i)) {
      			if (dir) {
        			state = 1;
        			check = number;
        			number = number * 10 + i - '0';
        			if (check > number || number > INT_MAX) {
          				printf("Error: Single number overflow\n");
          				fclose(fl);
          				return;
        			}
      			}
    		}
		else{
			if (state && dir) {
        			check = number;
        			if (dir < 0 && sum < 0){
          				check -= sum;
				}
        			else if (dir > 0 && sum > 0){
          				check += sum;
				}
        			if (check > INT_MAX) {
          				printf("Error: Sum overflow error\n");
          				fclose(fl);
          				return;
        			}
        			sum += number * dir;
        			number = 0;
      			}
      			state = 0;
			dir = i == '-' ? -1 : i == '.' ? 0 : 1;
		}
  	}
  	printf("Sum of integers is: %i\n", sum);
  	fclose(fl);
}

//Clear the screen
void scrnClear(){
	system("clear");
}

//Start welcome message and start the shell
void shellStart(){
	clear();
	puts("\n************************\n"
		"\nWelcome To My Mini Shell\n"
		"\n************************\n\n");
}

//Display exit message and exit the shell
void shellExit(){
	clear();
	puts("\n\n******************************\n"
		"\nGoodbye and Have A Nice Day!!!\n"
		"\n******************************\n\n");
	exit(0);
}

//Store the command in array 
char* commandTxt[MaxCom]={"help", "ct", "rn", "cp", "del", "mv", "pwd", "cd", "ls", "wordcount", "getcount", "sum", "clear", "exit"};

//Store all command functions in array
void (*commandList[MaxCom])(int argc, char* argv[])={ helpCmd, fileCreate, fileRename, fileCopy, fileDel, fileMove, dirCurr, dirChange, dirList, wordCount, getCount, getSum, scrnClear, shellExit};

//Use pipe operation to execute 3 commands
void getPiped(char* argv1[], int argc1, char* argv2[], int argc2, char* argv3[], int argc3, int commandType1, int commandType2, int commandType3){
	int fd[2];
	int fd2[2];
	int pid, pid2;
	
	if (pipe(fd)==-1){
		printf("Error: Couldn't initialize pipe\n");
		return;
	}
	pid = fork();
	if (pid==-1){
		printf("Error: Couldn't fork\n");
		return;
	}
	if (pid == 0){
		if (pipe(fd2)==-1){
			printf("Error: Couldn't initialize pipe\n");
			return;
		}
		
		pid2 = fork();
		if (pid2==-1){
			printf("Error: Couldn't fork\n");
			return;
		}
		if (pid2 == 0){
			sleep(1);
			close(fd[1]);
			close(fd[0]);
			close(fd2[1]);
			dup2(fd2[0], 0);
			close(fd2[0]);
			commandList[commandType3](argc3, argv3);
		}
		else{
			sleep(1);
			close(fd[1]);
			dup2(fd[0], 0);
			close(fd[0]);
			close(fd2[0]);
			dup2(fd2[1], 1);
			close(fd2[1]);
			commandList[commandType2](argc2, argv2);
		}
	}
	else{	
		close(fd[0]);
		dup2(fd[1], 1);
		close(fd[1]);
		commandList[commandType1](argc1, argv1);
	}
	return;	
}

//Check the commands that contain pipe
int checkPipe(char* input, char* inputPiped[]){
	int i;
	for (i=0; i<3; i++){
		inputPiped[i] = strsep(&input, "|");
		if (inputPiped[i] == NULL){
			break;
		}
	}
	if (inputPiped[1] == NULL){
		return 0;
	}
	else{
		return 1;
	}
}

//Check the commands is valid and assign to specific command type
int checkCommand(char* input, int* argc, char* argv[]){
	int i;
	*argc=0;
	if ((argv[*argc]=strtok(input, " \n")) != NULL){
		for(i=MaxCom-1; i>=0; i--){
			if(strcmp(argv[*argc], commandTxt[i])==0){
				break;
			}
		}
		if (i==-1){
			return i;
		}
	}
	else{
		return -1;
	}
	(*argc)++;

	while(1){
		if((argv[*argc]=strtok(NULL, " \n"))==NULL){
			break;
		}
		(*argc)++;
		if(*argc==20){
			return -1;
		}
	}
	return i;
}

//Handles the command input
void giveCommand(char* input){
	char* inputPiped[3];
	char* argv1[5];
	char* argv2[5];
	char* argv3[5];
	int argc1, argc2, argc3, commandType, commandType1, commandType2, commandType3;
	int piped = 0;
	piped = checkPipe(input, inputPiped);
	
	if(piped==1){
		commandType1 = checkCommand(inputPiped[0], &argc1, argv1);
		commandType2 = checkCommand(inputPiped[1], &argc2, argv2);
		commandType3 = checkCommand(inputPiped[2], &argc3, argv3);
		if (commandType1==-1||commandType2==-1||commandType3==-1){
			printf("Error: Pipe command not existed. Use help command\n");
		}
		else{
			getPiped(argv1, argc1, argv2, argc2, argv3, argc3, commandType1, commandType2, commandType3);
		}
	}
	else{
		commandType = checkCommand(input, &argc1, argv1);
		if (commandType==-1){
			printf("Error: Command not existed. Use help command\n");
		}
		else{
			commandList[commandType](argc1, argv1);
		}
	}
}

//Give prompt like Linux
void displayPrompt(){
	char* promptLine = (char*)malloc(MaxWord);
	char* username = getenv("USER");
	char* pointer = ">>> ";
	sprintf(promptLine, "%s%s", username, pointer);
	if(write(STDOUT_FILENO, promptLine, strlen(promptLine))!=strlen(promptLine)){
		printf("Error: STDOUT cannot be written");
		exit(1);
	}
}

int main(){
	char input[MaxLine];
	int n;

	shellStart();
	displayPrompt();
	(void)signal(SIGINT, shellExit);
 	
	while((n=read(STDIN_FILENO, input, MaxLine))>0){
		input[n]='\0';
		giveCommand(input);
		displayPrompt();
	}
	return 0;
}

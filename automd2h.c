#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
// sys calls
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> 


#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

# define OPTION_MAX_LENGHT 3
# define NO_ARGUMENT 1
# define ONE_ARGUMENT 2
# define TWO_ARGUMENT 3
# define USAGE "\n\
Usage: [-h|--help] [-t|--???] [-n|--???]\n\
    [-n|--num_steps VALUE] [-t|--type STRING] [-a|--allowed-cells STRING]\n\
    [-d|--distribution VALUES] [-i|--interactive] [-s|--stdin]\n\
\n\
automd2h convertit les fichiers au format Markdown en fichiers au format HTML.\n\
\n\
 Arguments optionnel:\n\
  -h,                         Shows this help message and exit\n\
                              \n\
  -t,                         Avec l'option -t. La date de dernière modification des fichiers\n\
                              est utilisée pour savoir s'il faut reconvertir. Si le fichier source \n\
                              est plus récent que le ficher .html cible associé, ou si le fichier. \n\
                              html cible n'existe pas, alors il y a conversion. Si la date est identique \n\
                              ou si le fichier .html cible est plus récent, alors il n'y a pas de conversion.\n\
                              \n\
  -n,                         L'option -n désactive l'utilisation de pandoc, à la place, la liste des chemins\n\
                              des fichiers sources à convertir sera affichée (un par ligne).\n\
                              Combiné avec -n, l'option -t n'affiche que les fichiers \n\
                              sources effectivement à convertir.\n\
"

// utiliser un max d'appel system : opendir, closedir (pour se promenet dans les rep de facon recurive)
// fork() wait() etc...

/**
 * The status of the program.
 */
enum Status {
    OK,                         /**< Everything is alright */
    WRONG_VALUE                /**< Wrong value */
};

enum Format{
    txt,
    markdown,
    html
};

struct File
{
    /* data */
    enum Format format;
    char* filename;        //The name of the file
};


/**
 * The parsed arguments.
 */
struct Arguments {
    enum Status status;             /**< The status of the parsing */
    bool Default;                   /**< Default Args  */
    int argv_index;
    int num_files;
    int num_directories;
    int num_options;
    char option1,option2,option3,option4;            // options
    struct File files[];             // Array of Files to convert. It can be unlimited
};


/**
 * File Validation functions.
 */

bool is_HTML(char *filename){
    return strstr(filename, ".html") != NULL;
}
bool is_Markdown(char *filename){
    return strstr(filename, ".md") != NULL;
}
bool is_txt(char *filename){
    return strstr(filename, ".txt") != NULL;
}
bool Filename_is_Valide(char *filename){
    return is_txt(filename) || is_Markdown(filename) || is_HTML(filename);
}

/**
 * Option Validation functions.
 */
bool is_Option(char *option){
    return strstr(option, "-") != NULL  && strlen(option) == 2;
}
bool is_Option_t(char *option){
    return strstr(option, "-t") != NULL;
}

//  Note: les option -x sont entree en premier, ensuite les noms de fichiers
//  NomProgramme -options(4options Max) NomsFichiers(Unlimited)
//  ex automd2h foo.txt -> foo.txt.html
struct Arguments *parse_arguments(int argc, char *argv[]) {

    struct Arguments *arguments = malloc(sizeof(struct Arguments));
    arguments->argv_index=0;
    arguments->num_files=0;
    arguments->num_directories=0;
    arguments->num_options=0;
    char option1[OPTION_MAX_LENGHT];
    char option2[OPTION_MAX_LENGHT];

    // ---Option detection Part ---
    if (is_Option(argv[1]))
    {
        for (int i = 0; i < argc; i++)
        {
            if (is_Option(argv[i]))
            {
                arguments->num_options++;
                arguments->argv_index++;
            }
            
        }

        if (arguments->num_options == 1)
        {
            strcpy(option1, argv[1]);
            arguments->option1 =option1[1];
            arguments->status = OK;
        }else if (arguments->num_options == 2)
        {
            strcpy(option1, argv[1]);
            strcpy(option2, argv[2]);
            arguments->option1 =option1[1];
            arguments->option2 =option2[1];
            arguments->status = OK;

        }else if (arguments->num_options == 3)
        {
            /* code */
        }else if (arguments->num_options == 4)
        {
            /* code */
        }

        //next argv
        arguments->argv_index++;
        
    
    // ---File detection Part ---
    }
    if (Filename_is_Valide(argv[arguments->argv_index]))
    {

        if (is_Markdown(argv[arguments->argv_index]))
        {
            printf("You want me to convert a md file \n");
            arguments->files[0].filename = argv[arguments->argv_index];
            arguments->files[0].format= markdown;
            arguments->num_files++;
        }

        if (is_HTML(argv[arguments->argv_index]))
        {
            printf("You want me to convert a html file \n");
            arguments->files[0].filename = argv[arguments->argv_index];
            arguments->files[0].format= html;
            arguments->num_files++;
            
        }

        if (is_txt(argv[arguments->argv_index]))
        {
            printf("You want me to convert a txt file \n");
            arguments->files[0].filename = argv[arguments->argv_index];
            arguments->files[0].format= txt;
            arguments->num_files++;
            
        }   
        
    } 
    else
    {
        arguments->status = WRONG_VALUE;
    }

    
    // Parse options
    switch (arguments->option1)
    {
    case 'n':
        /* code */
        break;
    
    default:
        break;
    }

    return arguments;
}

void print_args(struct Arguments *arguments){
    printf("Arguments  \n");
    printf("Arg1 : %c \n",arguments->option1);
    printf("Arg2 : %c \n",arguments->option2);


}

//Check if converted document has a newer version
bool is_new_doc_version(time_t sourceFile, time_t destFile){
	return sourceFile > destFile;
}

// Check if file exists in Current repository
bool file_exist(char *filePath){
 	return access(filePath, F_OK) != -1;
};

// Return the new file name after conversion
char* new_file_name(char *filePath){
 	char *newFileName = (char *) malloc(255);
 	if (file_exist(filePath)){
 		if(is_txt(filePath)){
			strcpy(newFileName, filePath);
 		}else if(is_Markdown(filePath)){
			//copy all except .md
			strncpy(newFileName, filePath, sizeof(filePath) -4);
 		}
		//strncat(newFileName, ".html", 5);
 	}
	return newFileName;
}

//Check if documents has a new version to convert
bool file_needs_conversion(char *filePath){
 	struct stat attrib;
	struct stat newAttrib;
	bool convert = true;
 	if (file_exist(filePath) && !is_HTML(filePath)){
 		stat(filePath, &attrib);
		if (file_exist(new_file_name(filePath))){
			stat(new_file_name(filePath), &newAttrib);
			convert = is_new_doc_version(attrib.st_mtime, newAttrib.st_mtime);
		}
	}else{
		convert = false;
	}
	return convert;
};

// Function to replace a string with another 
// string 
char *replaceWord(const char *s, const char *oldW, 
                                 const char *newW) 
{ 
    char *result; 
    int i, cnt = 0; 
    int newWlen = strlen(newW); 
    int oldWlen = strlen(oldW); 
  
    // Counting the number of times old word 
    // occur in the string 
    for (i = 0; s[i] != '\0'; i++) 
    { 
        if (strstr(&s[i], oldW) == &s[i]) 
        { 
            cnt++; 
  
            // Jumping to index after the old word. 
            i += oldWlen - 1; 
        } 
    } 
  
    // Making new string of enough length 
    result = (char *)malloc(i + cnt * (newWlen - oldWlen) + 1); 
  
    i = 0; 
    while (*s) 
    { 
        // compare the substring with the result 
        if (strstr(s, oldW) == s) 
        { 
            strcpy(&result[i], newW); 
            i += newWlen; 
            s += oldWlen; 
        } 
        else
            result[i++] = *s++; 
    } 
  
    result[i] = '\0'; 
    return result; 
}

// To review
void free_arguments(struct Arguments *arguments) {
    free(arguments);
}

int main(int argc, char *argv[])
{
    printf("\nStarting the program... \n");

	printf("%d\n", file_exist("test.md"));
	printf("%d\n", file_exist("test.txt"));
	printf("%s\n", new_file_name("test.txt\0"));
	printf("%s\n", new_file_name("test.md\0"));
    	printf("%d\n", file_needs_conversion("test.txt\0"));
	printf("%d\n", file_needs_conversion("test.md\0"));
	printf("%d\n", file_needs_conversion("new.md\0"));


    //printf(USAGE);
    struct Arguments *arguments = parse_arguments(argc, argv); //takes the arguments in the structure
    if (arguments->status != OK) {  
        fprintf(stderr,"failed to read arguments\n");
        return arguments->status;
    }else
    {
        
        // All good
        printf("\n");
        print_args(arguments);
        printf("\n");


        int pid;
        pid = fork();

        if (pid == -1) {
            perror("Fork Error");
        }
        // child process because return value zero 
        else if (pid == 0){
        
            printf("Hello from Child!\n"); 
            // Pandoc will run here.
            
            //calling pandoc

            // argv array for: ls -l
            // Just like in main, the argv array must be NULL terminated.
            // try to run ./a.out -x -y, it will work
            char* output =replaceWord(arguments->files[0].filename,".md",".html");
            char * ls_args[] = { "pandoc" , arguments->files[0].filename, "-o", output, NULL};
            //                    ^ 
            //  use the name ls
            //  rather than the
            //  path to /bin/ls

            // Little explaination
            // The primary difference between execv and execvp is that with execv you have to provide the full path to the binary file (i.e., the program). 
            // With execvp, you do not need to specify the full path because execvp will search the local environment variable PATH for the executable.
            execvp(   ls_args[0],     ls_args);   
        }
        // parent process because return value non-zero.   
        else{

            printf("Hello from Parent!\n"); 
        }
    }
    

    return 0;
}




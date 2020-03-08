/**
 * @author yassine Hasnaoui (HASY04089702)
 * @author Philippe (LEBP11129502)
*/

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
#include <dirent.h>
#include <sys/inotify.h>
#include <signal.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))
#define USAGE "\n\
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
                              \n\
  -w,                         Avec l'option -w, automd2h bloque et surveille les modifications des fichiers \n\
                              et des répertoires passés en argument. Lors de la modification d'un fichier source,\n\
                              celui-ci est automatiquement reconverti. Si dans un répertoire surveillé un fichier .md \n\
                              apparait, est modifié, est déplacé ou est renommé, celui-ci aussi est automatiquement converti.\n\
                              \n\
  -f,                         Par défaut, avec -w, les fichiers ne sont convertis que si une modification future est détectée.\n\
                              Combiné avec -w, l'option -f force la conversion immédiate des fichiers trouvés puis surveille les modifications futures.\n\
"

/**
 * The status of the program.
 */
enum Status
{
    OK,         /**< Everything is alright */
    WRONG_VALUE /**< Wrong value */
};

enum Format
{
    txt,
    markdown,
    html,
	another,
    Directory
};

enum Options
{
    t,
    n,
    r,
    w,
    f,
    no_option,
    Optionerror
};

struct File
{
    /* data */
    enum Format format;
    time_t time;       //the last modification time
    char filename[300]; //The name of the file
};

/**
 * The parsed arguments.
 */
struct Arguments
{
    enum Status status; /**< The status of the parsing */
    enum Options option1, option2, option3, option4;
    bool Default; /**< Default Args  */
    int argv_index;
    int num_files;
    int num_options;
    struct File files[200]; // Array of Files to convert. It can be unlimited
};

struct Directory
{
    char name[300];
};

struct VisitedDirectories
{
    int num_dir_visited;
    struct Directory DirectoriesTable[100];
};

// Check if file exists in Current repository
bool file_exist(char *filePath)
{
    return access(filePath, F_OK) != -1;
};

// Function to replace a string with another
char *replaceWord(const char *s, const char *oldW, const char *newW)
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
/**
 * File Validation functions.
 */

char *get_filename_ext(char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot;
}

bool is_Markdown(char *filename)
{
    return strcmp(get_filename_ext(filename), ".md") == 0;
}

bool is_HTML(char *filename)
{
	return strcmp(get_filename_ext(filename), ".html") == 0;
}

bool is_directory(char *filename)
{
    bool is_directory = false;
    DIR *dir;

    dir = opendir(filename);
    if (dir == NULL) {
       // printf("Couldn't open dir\n");
    } else {
        //printf("Opened dir\n");
        is_directory=true;
    }

    if (dir != NULL)
        closedir(dir);

    return is_directory;
}
/**
 * Option Validation functions.
 */
bool is_Option(char *option)
    {
        return strstr(option, "-") != NULL && strlen(option) == 2;
    }
    bool is_Option_t(char *option)
    {
        return strstr(option, "-t") != NULL;
    }
    bool is_Option_n(char *option)
    {
        return strstr(option, "-n") != NULL;
    }
    bool is_Option_r(char *option)
    {
        return strstr(option, "-r") != NULL;
    }
    bool is_Option_w(char *option)
    {
        return strstr(option, "-w") != NULL;
    }
    bool is_Option_f(char *option)
    {
        return strstr(option, "-f") != NULL;
    }
enum Options option_detection(char *option)
{
    enum Options Detected_option = no_option;
    if (is_Option_t(option))
    {

        Detected_option = t;
    }
    else if (is_Option_n(option))
    {

        Detected_option = n;
    }
    else if (is_Option_r(option))
    {

        Detected_option = r;
    }
    else if (is_Option_w(option))
    {

        Detected_option = w;
    }
    else if (is_Option_f(option))
    {

        Detected_option = f;
    }
    else
    {
        Detected_option = Optionerror;
        fprintf(stderr, "Option detection error");
        return 1;
    }

    return Detected_option;
}

void initialise_Arguments(struct Arguments *arguments)
{
    arguments->option1 = no_option;
    arguments->option2 = no_option;
    arguments->option3 = no_option;
    arguments->option4 = no_option;
    arguments->argv_index = 1;
    arguments->num_files = 0;
    arguments->num_options = 0;
}

void No_arg_Failure(int argc){
    if (argc == 1) {exit(EXIT_FAILURE);}
}


struct Arguments *parse_arguments(int argc, char *argv[])
{
    //fails if no arguments
    No_arg_Failure(argc);


    //printf("Parsing Arguments..\n");
    struct Arguments *arguments = malloc(sizeof(struct Arguments));
    initialise_Arguments(arguments);

    // ---Option detection Part ---
    if (is_Option(argv[1]))
    {
        for (int i = 1; i < argc; i++)
        {
            //counting number of options and incrementing the arg index
            if (is_Option(argv[i]))
            {
                arguments->num_options++;
                arguments->argv_index++;
            }
        }
        //if we have one option as argument
        if (arguments->num_options == 1)
        {
            arguments->option1 = option_detection(argv[1]);
            arguments->status = OK;
        }
        //if we have two option as argument
        else if (arguments->num_options == 2)
        {
            arguments->option1 = option_detection(argv[1]);
            arguments->option2 = option_detection(argv[2]);
            arguments->status = OK;
        }
        else if (arguments->num_options == 3)
        {
            arguments->option1 = option_detection(argv[1]);
            arguments->option2 = option_detection(argv[2]);
            arguments->option3 = option_detection(argv[3]);
            arguments->status = OK;
        }
        else if (arguments->num_options == 4)
        {
            arguments->option1 = option_detection(argv[1]);
            arguments->option2 = option_detection(argv[2]);
            arguments->option3 = option_detection(argv[3]);
            arguments->option4 = option_detection(argv[4]);
            arguments->status = OK;
        }

        // ---File detection Part ---
    }
    //Looping through the rest of arguments (files)
    for (int i = arguments->argv_index; i < argc; i++)
    {
        if (is_Markdown(argv[arguments->argv_index]))
        {

                strncpy(arguments->files[arguments->num_files].filename, argv[arguments->argv_index], sizeof(arguments->files[arguments->num_files].filename));
                arguments->files[arguments->num_files].filename[sizeof(arguments->files[arguments->num_files].filename) - 1] = '\0';
                //printf("You want me to convert a md file (%s) \n",argv[arguments->argv_index]);
                arguments->files[arguments->num_files].format = markdown;
                arguments->status = OK;
                arguments->num_files++;
            
				}
    		else if (is_HTML(argv[arguments->argv_index]))
        {
                //printf("You want me to convert a html file \n");
                strncpy(arguments->files[arguments->num_files].filename, argv[arguments->argv_index], sizeof(arguments->files[arguments->num_files].filename));
                arguments->files[arguments->num_files].filename[sizeof(arguments->files[arguments->num_files].filename) - 1] = '\0';
                arguments->files[arguments->num_files].format = html;
                arguments->status = OK;
                arguments->num_files++;
        }
        else if (is_directory(argv[arguments->argv_index]))
        {
            //printf("You want me to convert files inside (%s) directory \n", argv[arguments->argv_index]);
            strncpy(arguments->files[arguments->num_files].filename, argv[arguments->argv_index], sizeof(arguments->files[arguments->num_files].filename));
            arguments->files[arguments->num_files].filename[sizeof(arguments->files[arguments->num_files].filename) - 1] = '\0';
            arguments->files[arguments->num_files].format = Directory;
            arguments->status = OK;
            arguments->num_files++;
        }
        else
        {
            //printf("You want me to convert a another file \n");
                strncpy(arguments->files[arguments->num_files].filename, argv[arguments->argv_index], sizeof(arguments->files[arguments->num_files].filename));
                arguments->files[arguments->num_files].filename[sizeof(arguments->files[arguments->num_files].filename) - 1] = '\0';
                arguments->files[arguments->num_files].format = another;
                arguments->status = OK;
                arguments->num_files++;

        }
        //next argument
        arguments->argv_index++;
    }

    arguments->status = OK;
    return arguments;
}

void print_args(struct Arguments *arguments)
{
    printf("Arguments  \n");
    switch (arguments->option1)
    {
    case n:
        printf("Arg1 : n \n");
        break;
    case t:
        printf("Arg1 : t \n");
        break;
    case w:
        printf("Arg1 : w \n");
        break;
    case f:
        printf("Arg1 : f \n");
        break;
    case no_option:
        printf("Arg1 : no_option \n");
        break;

    default:
        break;
    }

    switch (arguments->option2)
    {
    case n:
        printf("Arg2 : n \n");
        break;
    case t:
        printf("Arg2 : t \n");
        break;
    case w:
        printf("Arg2 : w \n");
        break;
    case f:
        printf("Arg2 : f \n");
        break;
    case no_option:
        printf("Arg2 : no_option \n");
        break;

    default:
        break;
    }

    switch (arguments->option3)
    {
    case n:
        printf("Arg3 : n \n");
        break;
    case t:
        printf("Arg3 : t \n");
        break;
    case w:
        printf("Arg3 : w \n");
        break;
    case f:
        printf("Arg3 : f \n");
        break;
    case no_option:
        printf("Arg3 : no_option \n");
        break;

    default:
        break;
    }

    switch (arguments->option4)
    {
    case n:
        printf("Arg4 : n \n");
        break;
    case t:
        printf("Arg4 : t \n");
        break;
    case w:
        printf("Arg4 : t \n");
        break;
    case f:
        printf("Arg4 : f \n");
        break;
    case no_option:
        printf("Arg4 : no_option \n");
        break;

    default:
        break;
    }
}

char* concatenate_file_extension(char *filePath){
 	char *newFileName = (char *) malloc(255);
 	if (file_exist(filePath)){
 		if(is_Markdown(filePath)){
			//copy all except .md
			newFileName = replaceWord(filePath, ".md", ".html");
		}
		else if (is_directory(filePath) == false){
			strcpy(newFileName, filePath);
			strcat(newFileName, ".html");
 		}
 	}
 		return newFileName;
}

//Check if converted document has a newer version (option t)
bool has_new_doc_version(time_t sourceFile, time_t destFile)
{
    return sourceFile > destFile;
}

//Check if documents has a new version to convert
bool file_needs_conversion(char *filename)
{
    //printf("analysing %s\n",filename);
    bool convert = false;
    struct stat attrib;
    struct stat newAttrib;

    char *newFileName = concatenate_file_extension(filename);

    //printf("\nChecking if %s needs to be converted\n", filename);
    //Checking if the given file exists and if its a .md
    if (file_exist(filename))
    {
        //getting the file stats
        if (stat(filename, &attrib) == 0)
        {
            //if there is no errors, check if his htmk version exists
            if (file_exist(newFileName))
            {
                //if the html file alredy exists, we proceed
                if (stat(newFileName, &newAttrib) == 0)
                {
                    // Verify if there is a newer version of the source file
                    if (has_new_doc_version(attrib.st_mtime, newAttrib.st_mtime))
                    {
                        //If there is, a convertion is needed
                        convert = true;
                        //printf("..Convertion needed for: %s\n",filename);
                    }
                }
                else
                {
                    //printf("Unable to get file properties.\n");
                }
            }
            else
            {
                //destFile does not exists, need to convert the source file
                //printf("'%s' file do not exists, i can't compare them.\n", newFileName);
                convert = true;
            }
        }
        else
        {
            //printf("Unable to get file properties.\n");
            //printf("Please check whether '%s' file exists.\n", filename);
        }
    }
    else
    {
        convert = false;
        //printf("The file %s does not exists or is not .md\n\n", filename);
    }

    return convert;
};

//pritn all txt and md file in a directory (option n)
void print_current_directory(char *currentDir, bool checkTime)
{
    struct dirent *d;

    //printf("printing current directory...\n");
    DIR *dir = opendir(currentDir);
    if (dir == NULL)
    {
        fprintf(stderr, "Can't access directory\n");
    }
    while ((d = readdir(dir)) != NULL)
    {
        if (is_Markdown(d->d_name))
        {
            if (!checkTime || file_needs_conversion(d->d_name))
            {
                printf("%s\n", d->d_name);
            }
        }
    }
    closedir(dir);
}


void print_arguments_files(struct Arguments *arguments, bool checkTime)
{
    for (int i = 0; i < arguments->num_files; i++)
    {
        if (is_directory(arguments->files[i].filename))
        {
            print_current_directory(arguments->files[i].filename, checkTime);
        }
        else
        {
            printf("%s\n", arguments->files[i].filename);
        }
    }
}

// To review
void free_arguments(struct Arguments *arguments)
{
    free(arguments);
}

void Print_num_Options(struct Arguments *arguments)
{
    printf("Number of options entered :%d \n", arguments->num_options);
}

int Pandoc(char *file)
{
    //printf("Pandoc is trying to convert the file...\n");
    // Forking
    pid_t c_pid;
    c_pid = fork();

    if (c_pid == -1)
    {
        perror("Fork Error");
    }
    // child process because return value zero
    else if (c_pid == 0)
    {
        //Pandoc will run here.
	    char *output = concatenate_file_extension(file);
    	char *ls_args[] = {"pandoc", file, "-o", output, NULL};

            //calling pandoc
            if (file_exist(file))
            {
                execvp(ls_args[0], ls_args);
            }
            else
            {
                //Error Handeler
                perror("ENOENT");
                exit(EXIT_FAILURE);
            }
        //exit(EXIT_SUCCESS);
    }
    else
    {
        //parent
        int status;

        //Wait for the child whose process ID is equal to the value of pid.
        waitpid(c_pid, &status, 0);
        if (WIFEXITED(status))
        {
            int exit_status = WEXITSTATUS(status);
            if (exit_status != EXIT_SUCCESS)
            {
                exit(EXIT_FAILURE);
            }
        }
       // printf("status: %d\n",status);
    }
     printf("pandoc ends with stat 0\n");
    return 0;
}

// Check if the user entered the same option twice.
bool Check_Duplicates(enum Options OptionArray[])
{

    bool DuplicateOption = false;

    for (int i = 0; i < 4 - 1; i++)
    {
        for (int j = i + 1; j < 4; j++)
        {
            if (OptionArray[i] != no_option && OptionArray[i] == OptionArray[j])
            {
                DuplicateOption = true;
            }
        }
    }
    return DuplicateOption;
}
bool if_html_version_exists(char *file)
{
    char *newFileName = concatenate_file_extension(file); //will return file if fails
    bool htmlExists = false;
    //Checking if the html version of a md file exists if its a md file
    if (file_exist(newFileName) && strcmp(file, newFileName) != 0)
    {
        htmlExists = true;
    }
    return htmlExists;
}

//Convert all md (ASKED SPECIFICALLY) files inside Directory if there is no html version of them
int Convert_Directory(char *Dir, bool checktime)
{
    DIR *Directory;
    struct dirent *entry;
    struct stat filestat;

    //printf("I am converting (%s) Directory\n", Dir);

    Directory = opendir(Dir);
    if (Directory == NULL)
    {
        //perror("Unable to read directory.. i'm leaving\n");
         printf("Convert_Directory ends with stat 0\n");
        return (0); // leave
    }

    /* Read directory entries */
    while ((entry = readdir(Directory)))
    {
        char fullname[257];
        sprintf(fullname, "%s/%s", Dir, entry->d_name);
        stat(fullname, &filestat);

        if (S_ISDIR(filestat.st_mode))
        {
            //printf("%4s: %s\n", "Dir", fullname);
        }
        else
        {
            //printf("%4s: %s\n", "File", fullname);
            //its a file
		        if(is_Markdown(fullname) && (file_needs_conversion(fullname) || checktime == false)){
                    //printf("%s needs to be converted\n",fullname);
		            if (Pandoc(fullname) == 1){return 1;}
		        }
            
        }
    }
    closedir(Directory);
    printf("Convert_Directory ends with stat 0\n");
    return (0);
}

bool no_options_entered(enum Options OptionArray[])
{
    return OptionArray[0] == no_option && OptionArray[1] == no_option && OptionArray[2] == no_option && OptionArray[3] == no_option;
}

// The following functions checks if a specific option is in the arguments structure

bool Option_n(struct Arguments *arguments)
{
    return arguments->option1 == n || arguments->option2 == n || arguments->option3 == n || arguments->option4 == n;
}

bool Option_t(struct Arguments *arguments)
{
    return arguments->option1 == t || arguments->option2 == t || arguments->option3 == t || arguments->option4 == t;
}

bool Option_w(struct Arguments *arguments)
{
    return arguments->option1 == w || arguments->option2 == w || arguments->option3 == w || arguments->option4 == w;
}

bool Option_f(struct Arguments *arguments)
{
    return arguments->option1 == f || arguments->option2 == f || arguments->option3 == f || arguments->option4 == f;
}

// Checks if the given Directory has been visited
bool Dir_is_Visited(char *Dir, struct VisitedDirectories *Directories)
{
    bool isVisited = false;
    for (int i = 0; i < Directories->num_dir_visited; i++)
    {
        printf("comparing %s and %s, i = %d\n", Dir, Directories->DirectoriesTable[i].name, i);
        if (strcmp(Dir, Directories->DirectoriesTable[i].name) == 0)
        {
            isVisited = true;
        }
    }
    return isVisited;
}

//Need to fork this function
void Delete_Child(pid_t c_pid_To_Delete, int sec)
{

    // It will be watching for an event in the current Directory for a certain amount of time
    time_t endwait;
    time_t start = time(NULL);
    time_t seconds = sec; // end loop after this time has elapsed
    endwait = start + seconds;

    while (start < endwait)
    {

        sleep(1); // sleep 1s.
        start = time(NULL);
    }
    printf("*****************Deleted..*************\n");
    //killing the child after a certain delay.
    kill(c_pid_To_Delete, SIGKILL);
}
// Listen in the current directories
int watch(char *Dir)
{

    //printf("starting watching..\n");
    int length, i = 0;
    int fd;
    int wd[2];
    char buffer[BUF_LEN];

    // Initialise inotify for the current Directory
    fd = inotify_init();
    if (fd < 0)
    {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    //Adding to the watch list
    wd[0] = inotify_add_watch(fd, Dir, IN_CREATE | IN_MODIFY);
    while (true)
    {
        struct inotify_event *event;

        length = read(fd, buffer, BUF_LEN);
        if (length < 0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        event = (struct inotify_event *)&buffer[i];

        if (event->len)
        {
            if (event->wd == wd[0])
                printf("In %s\n", Dir);
            else
                continue;
            //if (event->mask & IN_CREATE)
            //{
              //  if (event->mask & IN_ISDIR)
              //  {
               //     printf("The directory %s was created.\n", event->name);
               // }
               // else
              //  {
               //     printf("The file %s was created.\n", event->name);
               // }
            //}
						if(event->mask & IN_MODIFY){
								//if (Pandoc(event->name) == 1){
                 // 		return 1;
              	//}
						}
      	}
    }
    (void)inotify_rm_watch(fd, wd[0]);
    (void)close(fd);
    return 0;
}

int Watch_fork(char *Dir, struct VisitedDirectories *Directories)
{
    //Only watch the current directory if its not visited
    if (Dir_is_Visited(Dir, Directories) == true)
    {
        //Already visited, no need to watch it again
        printf("%s is already visited\n", Dir);
        //leave
        return 0;
    }
    else
    {
        //not vsited, Add it to the visited Directories table.
        printf("adding %s Dir to the list at the positon : %d\n", Dir, Directories->num_dir_visited);
        struct Directory directory;
        strncpy(directory.name, Dir, sizeof(directory.name));
        directory.name[sizeof(directory.name) - 1] = '\0';
        Directories->DirectoriesTable[Directories->num_dir_visited] = directory;
        Directories->num_dir_visited++;
    }
    //---------------------------------------------------------
    // This code is executed only if its an unvisited directory
    pid_t c_pid;
    c_pid = fork();

    if (c_pid == 0)
    {
        //the child will be watching this unvisited directory...
        watch(Dir);
    }
    else if (c_pid > 0) //parent
    {
        //Deleting the child after ? secondes
        Delete_Child(c_pid, 10);
    }
    else
    {
        //error: The return of fork() is negative
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}
bool RecursiveSearch(char *Dir, bool AddWatcher, struct VisitedDirectories *Directories)
{
    printf("Visited dir : %d\n", Directories->num_dir_visited);
    //Directory stuff
    DIR *Directory;
    struct dirent *entry;
    struct stat filestat;

    printf("I am Reading %s Directory\n", Dir);
    // Open the current directory
    Directory = opendir(Dir);
    if (Directory == NULL)
    {
        //error
        perror("Unable to read directory.. i'm leaving\n");
        return (1); // leave
    }

    //Adding a watcher in the current Directory of the recursion
    if (AddWatcher)
    {
        Watch_fork(Dir, Directories);
    }

    /* Read directory entries */
    while ((entry = readdir(Directory)))
    {
        char fullname[257];
        sprintf(fullname, "%s/%s", Dir, entry->d_name);
        stat(fullname, &filestat);

        //Checking if we are dealing with a file or a directory
        if (S_ISDIR(filestat.st_mode))
        {
            //its a dir
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) // to not infinite loop
            {
                // Recursion happens here
                printf("\n*Entering a subDirectory* : %s \n", entry->d_name);
                RecursiveSearch(fullname, AddWatcher, Directories);
                printf("\n*Leaving a subDirectory*\n");
            }
        }
        else
        {
            //its a file
        }
    }
    return true;
}

void Observe(bool Immediate_Convertion)
{
    printf("\nStarting to observe Sub Directories ...\n");

    struct VisitedDirectories Directories;
    Directories.num_dir_visited = 0;

    //This will converte immediatly any files (option f)
    if (Immediate_Convertion)
    {
        RecursiveSearch(".", false, &Directories);
    }

    while (RecursiveSearch(".", true, &Directories))
    {
        printf("\nsleeping...\n");
        sleep(2);
    }
}
// launching the program with no option entered.
int launch_with_no_options(struct Arguments *arguments){
        for (int i = 0; i < arguments->num_files; i++)
        {
            //   if the current argument is a file
            if (is_directory(arguments->files[i].filename)==false)
            {
                if (Pandoc(arguments->files[i].filename) == 1)
                {
                    return 1;
                }
            }
            //   if the current argument is a Directory
            else if (is_directory(arguments->files[i].filename))
            {
                if (Convert_Directory(arguments->files[i].filename,false) == 1)
                {
                    return 1;
                }
            }
        }
        return 0;
}

// launching the program with options
int launch_with_options(struct Arguments *arguments,enum Options *option,enum Options *next_option, int *option_index){
        switch (*option)
        {
        case t:
            // OPTION -T -N
             if (*next_option == n){
                 *next_option = no_option;
                 *option_index++; // because we already considered the next option (which is n)

                 //t combined with n
                for (int file = 0; file < arguments->num_files; file++)
                {
                    //printf("file name : %s \n", arguments->files[file].filename);
                    if (file_needs_conversion(arguments->files[file].filename))
                    {
                        printf("%s\n",arguments->files[file].filename);
                    }
                    else
                    {
                        //no need to be converted
                        //  printf("no convertion needed for %s \n", arguments->files[file].filename);
                    }
                } 

             }else// OPTION T
             {
                 //printf("option t detected\n");                 
                for (int file = 0; file < arguments->num_files; file++)
                {
                    if (is_directory(arguments->files[file].filename)==false )
                    {
                        //file Need to be converted
                        //printf("%s needs to be converted .\n", arguments->files[file].filename);
                        if (file_needs_conversion(arguments->files[file].filename))
                        {
                            if(Pandoc(arguments->files[file].filename) == 1){return 1;}
                        }
                    }
                    else if(is_directory(arguments->files[file].filename)==true)
                    {
                        //printf("DIR %s needs to be converted .\n", arguments->files[file].filename);
                        //elements of the directory needs to be converted, needs to checktime
                        // convertir juste les md avec une nouvelle version par rapport au html ou sans html du tout
                        Convert_Directory(arguments->files[file].filename,true);
                    }else
                    {
                        //no need to be converted
                        //  printf("no convertion needed for %s \n", arguments->files[file].filename);
                    }
                    
                }             
            }     
            break;

        case n:

            if (*next_option == t)
            {
                ///printf("\nOption n combined with t Detected.\n");
                print_arguments_files(arguments, true);
                *option_index++; // because we already considered the next option (which is t)
            }
            else
            {
                //printf("\nOption n Detected.\n");
                print_arguments_files(arguments, false);
            }
            break;

        case r:
            printf("\nStarting Recursive Research..\n");
            RecursiveSearch(".", false, NULL);
            break;

        case w:

            if (*next_option == f)
            {
                printf("\nOption w combined with f Detected...Immediate convertion\n");
                *option_index++; // because we already considered the next option (which is f)
                //Observe(true);
            }
            else
            {
								for (int file = 0; file < arguments->num_files; file++)
                {
									watch(arguments->files[file].filename);
								}
                //printf("\nOption w Detected.\n");
                //Observe(false);
            }

            break;

        case Optionerror:
            //fprintf(stderr, "Option parsing failed\n");
            arguments->status = WRONG_VALUE;
            //exit(EXIT_FAILURE);
            break;

        default:
            break;
        }
        printf("launch_with_options ends with stat 0\n");
    return 0;
}

int lauchProgram(struct Arguments *arguments)
{
    //Array of options
    enum Options OptionArray[5];
    OptionArray[0] = arguments->option1;
    OptionArray[1] = arguments->option2;
    OptionArray[2] = arguments->option3;
    OptionArray[3] = arguments->option4;
    OptionArray[4] = no_option;

    if (Check_Duplicates(OptionArray))
    {
        //fprintf(stderr, "Error duplicates options.\n");
        return 1;
    }

    //if no option is entered, we convert files entered if there is no html version of them
    if (no_options_entered(OptionArray))
    {
        if (launch_with_no_options(arguments)==1)
        {
            return 1;
        }
        return 0;
    }

    //if there are options
    //looping through the options // 4 options max
    for (int index_option = 0; index_option < 4; index_option++)
    {
        if( launch_with_options(arguments,&OptionArray[index_option],&OptionArray[index_option+1],&index_option) == 1)
        {return 1;}
    }
    printf("lauchProgram ends with stat 0\n");
    return 0;
}

int main(int argc, char *argv[])
{
    //printf("\nStarting the program... \n");
    //printf(USAGE);
    struct Arguments *arguments = parse_arguments(argc, argv); //takes the arguments in the structure
    if (arguments->status != OK)
    {
        //fprintf(stderr, "failed to read arguments\n");
        return 1;
    }
    else
    {
        // All good
        //print_args(arguments);
        //Print_num_Options(arguments);
        if (lauchProgram(arguments) == 1)
        {  
            return 1;
        }
    }
    free_arguments(arguments);
    printf("MAIN ends with stat 0\n");
    return 0;
}

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
#include <linux/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
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
    time_t time;    //the last modification time
    char *filename; //The name of the file
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
    int num_directories;
    int num_options;
    struct File files[20]; // Array of Files to convert. It can be unlimited
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

bool is_HTML(char *filename)
{
    return strstr(filename, ".html") != NULL;
}
bool is_Markdown(char *filename)
{
    return strstr(filename, ".md") != NULL;
}
bool is_txt(char *filename)
{
    return strstr(filename, ".txt") != NULL;
}
bool Filename_is_Valide(char *filename)
{
    return is_txt(filename) || is_Markdown(filename) || is_HTML(filename);
}

bool is_directory(char *filename)
{
    DIR *d;
    d = opendir(filename);
    return d;
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
        /* code */
        Detected_option = t;
    }
    else if (is_Option_n(option))
    {
        /* code */
        Detected_option = n;
    }
    else if (is_Option_r(option))
    {
        /* code */
        Detected_option = r;
    }
    else if (is_Option_w(option))
    {
        /* code */
        Detected_option = w;
    }
    else if (is_Option_f(option))
    {
        /* code */
        Detected_option = f;
    }
    else
    {
        Detected_option = Optionerror;
        fprintf(stderr, "Option detection error");
        exit(1);
    }

    return Detected_option;
}

//  Note: les option -x sont entree en premier, ensuite les noms de fichiers
//  NomProgramme -options(4options Max) NomsFichiers(Unlimited)
//  ex automd2h foo.txt -˛> foo.txt.html
void initialise_Arguments(struct Arguments *arguments)
{
    arguments->option1 = no_option;
    arguments->option2 = no_option;
    arguments->option3 = no_option;
    arguments->option4 = no_option;
    arguments->argv_index = 1;
    arguments->num_files = 0;
    arguments->num_directories = 0;
    arguments->num_options = 0;
}
struct Arguments *parse_arguments(int argc, char *argv[])
{

    printf("Parsing Arguments..\n");

    struct Arguments *arguments = malloc(sizeof(struct Arguments));
    initialise_Arguments(arguments);

    // ---Option detection Part ---
    if (is_Option(argv[1]))
    {
        for (int i = 1; i < argc; i++)
        {
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
            /* code */
        }
        else if (arguments->num_options == 4)
        {
            /* code */
        }

        //next argv
        //xs arguments->argv_index++;

        // ---File detection Part ---
    }
    //Looping through the rest of arguments (files)
    for (int i = arguments->argv_index; i < argc; i++)
    {
        if (Filename_is_Valide(argv[arguments->argv_index]))
        {

            if (is_Markdown(argv[arguments->argv_index]))
            {
                printf("You want me to convert a md file (%s) \n",argv[arguments->argv_index]);
                arguments->files[arguments->num_files].filename = argv[arguments->argv_index];
                arguments->files[arguments->num_files].format = markdown;
                arguments->num_files++;
            }

            if (is_HTML(argv[arguments->argv_index]))
            {
                printf("You want me to convert a html file \n");
                arguments->files[arguments->num_files].filename = argv[arguments->argv_index];
                arguments->files[arguments->num_files].format = html;
                arguments->num_files++;
            }

            if (is_txt(argv[arguments->argv_index]))
            {
                printf("You want me to convert a txt file \n");
                arguments->files[arguments->num_files].filename = argv[arguments->argv_index];
                arguments->files[arguments->num_files].format = txt;
                arguments->num_files++;
            }
        }
        else if (is_directory(argv[arguments->argv_index]))
        {
            printf("You want me to convert files inside (%s) directory \n", argv[arguments->argv_index]);
            arguments->files[arguments->num_files].filename = argv[arguments->argv_index];
            arguments->files[arguments->num_files].format = Directory;
            arguments->num_files++;
        }
        else
        {
		perror("ENOENT");
		exit(0);
            //arguments->status = WRONG_VALUE;

        }
        //next argument
        arguments->argv_index++;
    }

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

//Check if converted document has a newer version (option t)
bool has_new_doc_version(time_t sourceFile, time_t destFile)
{
    return sourceFile > destFile;
}

//Check if documents has a new version to convert
bool file_needs_conversion(char *filename)
{
    bool convert = false;
    struct stat attrib;
    struct stat newAttrib;
    char *newFileName = replaceWord(filename, ".md", ".html");

    printf("\nChecking if %s needs to be converted\n", filename);
    if (file_exist(filename) && is_Markdown(filename))
    {

        if (stat(filename, &attrib) == 0)
        {
            //ok
            if (file_exist(newFileName))
            { //if the converted file alredy exists
                printf("here\n");
                if (stat(newFileName, &newAttrib) == 0)
                {
                    if (has_new_doc_version(attrib.st_mtime, newAttrib.st_mtime))
                    {
                        convert = true;
                        printf("..Convertion needed\n");
                    }
                    else
                    {
                        printf("..no convertion needed\n");
                    }
                }
                else
                {
                    printf("Unable to get file properties.\n");
                }
            }
            else
            {
                printf("'%s' file do not exists, i can't compare them.\n", newFileName);
            }
        }
        else
        {
            printf("Unable to get file properties.\n");
            printf("Please check whether '%s' file exists.\n", filename);
        }
    }
    else
    {
        convert = false;
        printf("The file %s does not exists or is not .md\n\n", filename);
    }

    return convert;
};

//pritn all txt and md file in a directory (option n)
void print_current_directory(char *currentDir, bool checkTime)
{
    struct dirent *d;

    printf("printing current directory...\n");
    DIR *dir = opendir(currentDir);
    if (dir == NULL)
    {
        fprintf(stderr, "Can't access directory\n");
    }
    while ((d = readdir(dir)) != NULL)
    {
        if (is_Markdown(d->d_name) || is_txt(d->d_name))
        {
            if (!checkTime || file_needs_conversion(d->d_name))
            {
                printf("%s\n", d->d_name);
            }
        }
    }
    closedir(dir);
    printf("\n");
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

void Pandoc(char *file)
{

    printf("Pandoc is trying to convert the file...\n");

    // Forking
    pid_t pid;
    pid = fork();

    if (pid == -1)
    {
        perror("Fork Error");
    }
    // child process because return value zero
    else if (pid == 0)
    {

        //printf("Hello from Child!\n");
        // Pandoc will run here.

        //calling pandoc
        // argv array for: ls -l
        // Just like in main, the argv array must be NULL terminated.
        // try to run ./a.out -x -y, it will work
        char *output = replaceWord(file, ".md", ".html");
        char *ls_args[] = {"pandoc", file, "-o", output, NULL};
        //                    ^
        //  use the name ls
        //  rather than the
        //  path to /bin/ls

        // Little explaination
        // The primary difference between execv and execvp is that with execv you have to provide the full path to the binary file (i.e., the program).
        // With execvp, you do not need to specify the full path because execvp will search the local environment variable PATH for the executable.
        execvp(ls_args[0], ls_args);

        //Error Handeler
        fprintf(stdout, "pandoc failed\n");
        exit(1);
    }
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

bool if_html_version_exists(const char *file)
{
    char *MardownVersion = replaceWord(file, ".md", ".html");
    return file_exist(MardownVersion);
}

int Convert_Directory( char *Dir){

    DIR *Directory;
    struct dirent *entry;
    struct stat filestat;

    printf("I am converting (%s) Directory\n", Dir);

    Directory = opendir(Dir);
    if (Directory == NULL)
    {
        perror("Unable to read directory.. i'm leaving\n");
        return (1); // leave
    }

    /* Read directory entries */
    while ((entry = readdir(Directory)))
    {
        char fullname[257];
        sprintf(fullname, "%s/%s", Dir, entry->d_name);
        stat(fullname, &filestat);

        if (S_ISDIR(filestat.st_mode))
        {

            printf("%4s: %s\n", "Dir", fullname);
        }
        else
        {
            //its a file
            printf("%4s: %s\n", "File", fullname);
            if (is_Markdown(fullname) && if_html_version_exists(fullname) == false)
            {
                Pandoc(fullname);
            }
        }
    }
    closedir(Directory);
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

void watch(char *Dir){
    printf("starting watching..\n");
            int length, i = 0;
            int fd;
            int wd[2];
            char buffer[BUF_LEN];

            fd = inotify_init();

            if ( fd < 0 ) {
                perror( "inotify_init" );
            }
            wd[0] = inotify_add_watch( fd, Dir, IN_CREATE);

            while (true){
                struct inotify_event *event;

                length = read( fd, buffer, BUF_LEN );  

                if ( length < 0 ) {
                    perror( "read" );
                } 

                event = ( struct inotify_event * ) &buffer[ i ];

                if ( event->len ) {
                    if (event->wd == wd[0]) printf("In %s\n", Dir);
                    else continue;
                    if ( event->mask & IN_CREATE ) {
                        if ( event->mask & IN_ISDIR ) {
                            printf( "The directory %s was created.\n", event->name ); 
                        }
                        else {
                            printf( "The file %s was created.\n", event->name );
                        }
                    }
                }
            }
            ( void ) inotify_rm_watch( fd, wd[0] );
            ( void ) close( fd );
}

void Watch_fork(char *Dir){

    pid_t c_pid, pid;
    int status;

    c_pid = fork(); //duplicate

    if (c_pid == 0)
    {
        watch(Dir);
    }
    else if (c_pid > 0)
    {
        //parent

    }
    else
    {
        //error: The return of fork() is negative
        perror("fork failed");
        _exit(2); //exit failure, hard
    }

}
bool RecursiveSearch(char *Dir, bool AddWatcher)
{
    //Directory stuff
    DIR *Directory;
    struct dirent *entry;
    struct stat filestat;

    //inotify
    int wd[10];

    printf("I am Reading %s Directory\n", Dir);

    Directory = opendir(Dir);
    if (Directory == NULL)
    {
        perror("Unable to read directory.. i'm leaving\n");
        return (1); // leave
    }
    Watch_fork(Dir);
    /* Read directory entries */
    while ((entry = readdir(Directory))){
        char fullname[257];
        sprintf(fullname, "%s/%s", Dir, entry->d_name);
        stat(fullname, &filestat);
        if (S_ISDIR(filestat.st_mode)){
            //its a dir
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) // to not infinite loop
            {
                // Recursion
                printf("\n*Entering a subDirectory* : %s \n",entry->d_name);
                RecursiveSearch(fullname, AddWatcher);
                printf("\n*Leaving a subDirectory*\n");
            }

        }else
        {
            //its a file
        }
    
    }
    return true;
}

void Observe(bool Immediate_Convertion)
{
    printf("\nStarting to observe Sub Directories ...\n");

    pid_t c_pid, pid;
    int status;

    c_pid = fork(); //duplicate

    if (c_pid == 0)
    {
        //This will converte immediatly any files
        if (Immediate_Convertion)
        {
            RecursiveSearch(".", false);
        }
        
        while (RecursiveSearch(".", true))
        {
            printf("\nsleeping...\n");
            sleep(5);
        }
    }
    else if (c_pid > 0)
    {
        //parent

        //waiting for child to terminate
        pid = wait(&status);

        if (WIFEXITED(status))
        {
            printf("Parent: Child exited with status: %d\n", WEXITSTATUS(status));
        }
    }
    else
    {
        //error: The return of fork() is negative
        perror("fork failed");
        _exit(2); //exit failure, hard
    }
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
        fprintf(stderr, "Error duplicates options.\n");
        exit(1);
    }
                

    if (no_options_entered(OptionArray))
    {
        for (int i = 0; i < arguments->num_files; i++)
        {

                if (arguments->files[i].format != Directory &&if_html_version_exists(arguments->files[i].filename) == false)
                {
                    Pandoc(arguments->files[i].filename);
                }else if (arguments->files[i].format == Directory )
                {
                    /* code */
                    Convert_Directory(arguments->files[i].filename);
                }
                
            
        }

        return 0;
    }

    //looping through the options
    for (int i = 0; i < 4; i++)
    {

        switch (OptionArray[i])
        {
        case t:
            // Avec l'option -t. La date de dernière modification
            //des fichiers est utilisée pour savoir s'il faut reconvertir.
            // Si le fichier source est plus récent que le ficher .html cible associé,
            //ou si le fichier .html cible n'existe pas, alors il y a conversion.
            //Si la date est identique ou si le fichier .html cible est plus récent,
            //alors il n'y a pas de conversion.
            for (int file = 0; i < arguments->num_files; i++)
            {
                printf("\nOption t Detected.\n");
                printf("file name : %s \n", arguments->files[file].filename);
                if (file_needs_conversion(arguments->files[file].filename))
                {

                    printf("%s needs to be converted again.\n", arguments->files[file].filename);
                    Pandoc(arguments->files[file].filename);
                }
                else
                {
                    printf("no convertion needed for %s \n", arguments->files[file].filename);
                }
            }
            break;

        case n:
            //L'option -n désactive l'utilisation de pandoc,
            //à la place, la liste des chemins des fichiers sources à convertir sera affichée (un par ligne).
            //Combiné avec -n, l'option -t n'affiche que les fichiers sources effectivement à convertir.
            //if combined with t
            if (OptionArray[i + 1] == t)
            {
                printf("\nOption n combined with t Detected.\n");
                print_current_directory(".", true);
                i++; // because we already considered the next option (which is t)
            }
            else
            {
                printf("\nOption n Detected.\n");
                print_current_directory(".", false);
            }
            break;

        case r:
            printf("\nStarting Recursive Research..\n");
            RecursiveSearch(".", false);
            break;

        case w:
            
            if (OptionArray[i + 1] == f)
            {
                printf("\nOption w combined with f Detected...Immediate convertion\n");
                //Observe(true);
            }else
            {
                printf("\nOption w Detected.\n");
                Observe(false);
            }
            
            
            break;

        case Optionerror:
            fprintf(stderr, "Option parsing failed\n");
            break;

        default:
            break;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    printf("\nStarting the program... \n");

    //printf(USAGE);
    struct Arguments *arguments = parse_arguments(argc, argv); //takes the arguments in the structure
    if (arguments->status != OK)
    {
        fprintf(stderr, "failed to read arguments\n");
        return arguments->status;
    }
    else
    {

        // All good
        printf("\n");
        print_args(arguments);
        printf("\n");

        Print_num_Options(arguments);
        lauchProgram(arguments);
    }

    free_arguments(arguments);
    return 0;
}

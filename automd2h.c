#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
// sys calls
#include <sys/types.h>
#include <sys/wait.h>

# define OPTION_MAX_LENGHT 3
# define ONE_ARGUMENT 2
# define TWO_ARGUMENT 3
# define USAGE "\n\
Usage: [-h|--help] [-r|--num-rows VALUE] [-c|--num-cols VALUE]\n\
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
    char filename[];        //The name of the file
};


/**
 * The parsed arguments.
 */
struct Arguments {
    enum Status status;             /**< The status of the parsing */
    bool Default;                   /**< Default Args  */
    int num_files;
    int num_directories;
    char option1,option2;            // options
    struct File files[];             // Array of Files to convert
};



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


//  Note: les option -x sont entree en premier, ensuite les noms de fichiers

// ex automd2h foo.txt -> foo.txt.html
struct Arguments *parse_arguments(int argc, char *argv[]) {

    struct Arguments *arguments = malloc(sizeof(struct Arguments));
    char option1[OPTION_MAX_LENGHT];
    char option2[OPTION_MAX_LENGHT];

    /**
     * Option parsing
     * if the lenght of argv is equal to 2, its an option, else its a filename.
     * Im using only a max of 2 options as reference
     */

    if (argc == TWO_ARGUMENT && strlen(argv[1]) == 2 && strlen(argv[2]) == 2)
    {
        strcpy(option1, argv[1]);
        strcpy(option2, argv[2]);
        arguments->option1 =option1[1];
        arguments->option2 =option2[1];

        arguments->status = OK;
    }else if (argc == ONE_ARGUMENT && strlen(argv[1]) == 2 )
    {
        strcpy(option1, argv[1]);
        arguments->option1 =option1[1];
        arguments->status = OK;
    }else if (Filename_is_Valide(argv[1]))
    {
        /**
        * File parsing
        * if the lenght of argv >2 and contains a valide file format
        */

        if (is_Markdown(argv[1]))
        {
            printf("You want me to convert a md file \n");
            
        }

        if (is_HTML(argv[1]))
        {
            printf("You want me to convert a html file \n");
            
        }

        if (is_txt(argv[1]))
        {
            printf("You want me to convert a txt file \n");
            
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

// To review
void free_arguments(struct Arguments *arguments) {
    free(arguments);
}

int main(int argc, char *argv[])
{
    //printf(USAGE);
    struct Arguments *arguments = parse_arguments(argc, argv); //takes the arguments in the structure
    if (arguments->status != OK) {  //if it fails
        fprintf(stderr,"failed to read arguments\n");
        return arguments->status;
    }
    print_args(arguments);

    return 0;
}




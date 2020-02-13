#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
// sys calls
#include <sys/types.h>
#include <sys/wait.h>

# define OPTION_MAX_LENGHT 3
# define ONE_ARGUMENT 2
# define TREE_ARGUMENT 4
#define USAGE "\n\
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
};


/**
 * The parsed arguments.
 */
struct Arguments {
    enum Status status;             /**< The status of the parsing */
    bool Default;                   /**< Default Args  */
    int num_files;
    int num_directories;
    char option1,option2;
    
};

struct Arguments *parse_arguments(int argc, char *argv[]) {
    
    struct Arguments *arguments = malloc(sizeof(struct Arguments));
    char option1[OPTION_MAX_LENGHT];
    char option2[OPTION_MAX_LENGHT];

    if (argc>ONE_ARGUMENT && argc<TREE_ARGUMENT)
    {
        strcpy(option1, argv[1]);
        strcpy(option2, argv[2]);
        arguments->option1 =option1[1];
        arguments->option2 =option2[1];

        arguments->status = OK;
    }else if (argc == ONE_ARGUMENT)
    {
        strcpy(option1, argv[1]);
        arguments->option1 =option1[1];
        arguments->status = OK;
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
    printf("Arguments : \n");
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
        return arguments->status;
    }
    print_args(arguments);

    return 0;
}




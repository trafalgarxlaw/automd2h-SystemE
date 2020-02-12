#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


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
};

struct Arguments *parse_arguments(int argc, char *argv[]) {
    struct Arguments *arguments = malloc(sizeof(struct Arguments));

    // Default argument
    char option = 'n';

    // Parse options
    switch (option)
    {
    case 'n':
        /* code */
        break;
    
    default:
        break;
    }

    return arguments;
}

void print_args(int argc, char const *argv[]){
    printf("Arguments : \n");
    for (int i = 1; i < argc; i++)
    {
        /* code */
        printf("%s\n",argv[i]);
    }

}

void free_arguments(struct Arguments *arguments) {
    free(arguments);
}

int main(int argc, char const *argv[])
{
    printf(USAGE);
    // struct Arguments *arguments = parse_arguments(argc, argv); //takes the arguments in the structure
    //     if (arguments->status != OK) {  //if it fails
    //     return arguments->status;
    //     }
    print_args(argc,argv);

    return 0;
}




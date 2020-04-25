#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    pid_t p = fork();

    if (p==-1)
    {
        perror("fork")

    }else if (p == 0)
    {// nous somme dans le processus fils

        //execution de la commande ls (Deuxieme arguement represent 
        //le "mode" sous lequel la commande est appele)
        execlp("ls","ls","-a",NULL);
    }else
    { //nous somme dans le processus pere

        int status;
        // le system met a jour la variable status une fois que l'attente du processus fils se termine
        wait(&status);
        printf("status: %d\n",status);

    }
    return 0;
}

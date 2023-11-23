#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

#include "Tracer.h"

int main( int argc, char *argv[] )
{
    if ( argc < 2 ) {
        std::cout << "usage : " << argv[0] << " command" << std::endl;
        return -1;
    }
    char **arg = &argv[1];
    pid_t pid;
    pid = fork();
    if ( pid < 0 ) {
        std::cerr << "failed to fork" << std::endl;
        return -1;
    }

    if ( pid != 0 ) { // parent
        Tracer *pt = new Tracer( pid );
		pt->StartTrace();
        std::cout << "pid : " << pid << std::endl;
        int status;
        waitpid( pid, &status, 0 );
        std::cout << "\n-- Trace is done --\n" << std::endl;
        pt->StopTrace();
    }
    else {
        //pause();
        sleep(1);
        std::cout << "\n-- Trace start --\n" << std::endl;
        execvp( arg[0], (char**)arg );
    }
	return 0;
}

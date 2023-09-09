#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#include "ProcessorTracer.h"
#include "Decoder.h"
#include "PrintSourceCode.h"

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
        ProcessorTracer *pt = new ProcessorTracer( pid, true );
        std::cout << "pid : " << pid << std::endl;
        pt->InitCapture();
        int status;
        waitpid( pid, &status, 0 );
        std::cout << "trace is done" << std::endl;
        pt->StopTrace();
        PrintSourceCode *psc = new PrintSourceCode( pt->GetProcPath() );
        Decoder *d = new Decoder( pt->GetAux(), pt->GetAuxSize(), pt->GetProcPath(), pt->GetRawFileList(), psc );
        std::cout << "bin : " << pt->GetProcPath() << std::endl;
        d->InitDecoder();
        d->Decode();
        pt->SaveAuxFile();
        pt->SaveMapsFile();
    }
    else {
        //pause();
        sleep(1);
        std::cout << "trace start" << std::endl;
        execvp( arg[0], (char**)arg );
    }
    return 0;
}

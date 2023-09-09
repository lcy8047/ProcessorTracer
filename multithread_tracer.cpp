#include <iostream>

#include "MultiThreadTracer.h"

void usage( void )
{
    std::cout << "1. Start capture" << std::endl;
    std::cout << "2. Stop capture" << std::endl;
    std::cout << "3. Decode" << std::endl;
}

int main( void )
{
    pid_t pid;

    std::cout << "pid : ";
    std::cin >> pid;

    ThreadFilterList *_thread_filter = new ThreadFilterList;
    //_thread_filter->insert( "input_thread_comm");       // 431182

    
    MultiThreadTracer *mtt = new MultiThreadTracer( pid, _thread_filter );
    mtt->InitTracer();
    while ( true ) {
        usage();
        std::cout << ">";
        int select;
        std::cin >> select;
        switch ( select ) {
            case -1:
                return 0;
            case 1:
            {
                mtt->StartTrace();
                break;
            }
            case 2:
            {
                mtt->StopTrace();
                mtt->SaveAuxData();
                mtt->SaveMapsData();
                break;
            }
            case 3:
            {
                mtt->Decode();
                break;
            }
        }
    }
    return 0;
}
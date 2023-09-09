#include <iostream>
#include <unistd.h>

#include "Decoder.h"
#include "ProcessorTracer.h"
#include "PrintSourceCode.h"
#include "MultiThreadTracer.h"

void usage( void )
{
    std::cout << "0. Get bin path" << std::endl;
    std::cout << "1. Start capture" << std::endl;
    std::cout << "2. Stop capture" << std::endl;
    std::cout << "3. Save aux data" << std::endl;
    std::cout << "4. Decode" << std::endl;
}

int main( int argc, char *argv[] ) 
{
    ProcessorTracer *pt = nullptr;
    MultiThreadTracer *mtt = nullptr;
    Decoder *d = nullptr;
    pid_t pid;
    bool is_multi_thread = false;
    std::string binary;
    
    while ( true ) {
        switch ( getopt( argc, argv, "mPp:f:" ) )
        {
            case 'm':
            {
                is_multi_thread = true;
                continue;
            }
            case 'p':
            {
                pid = std::stoi( optarg );
                continue;
            }
            case 'f':
            {
                binary = optarg;
                continue;
            }
            default:
            {
                break;
            }
        }
        break;
    }
    PrintSourceCode *psc;
    if ( is_multi_thread ) {
        mtt = new MultiThreadTracer( pid );
    }
    else {
        pt = new ProcessorTracer( pid );
    }
    while ( true ) {
        usage();
        std::cout << ">";
        int select;
        std::cin >> select;
        switch ( select ) {
            case -1:
                return 0;
            case 0:
            {
                break;
            }
            case 1:
            {
                if ( is_multi_thread ) {
                    mtt->InitTracer();
                    mtt->StartTrace();
                }
                else {
                    pt->InitCapture();
                    pt->StartTrace();
                }
                break;
            }
            case 2:
            {
                if ( is_multi_thread ) {
                }
                pt->StopTrace();
                psc = new PrintSourceCode( pt->GetProcPath() );
                break;
            }
            case 3:
            {
                pt->SaveAuxFile();
                pt->SaveMapsFile();
                break;
            }
            case 4:
            {
                if ( pt == nullptr ) {
                    std::cerr << "Not traced yet" << std::endl;
                    break;
                }
                d = new Decoder( pt->GetAux(), pt->GetAuxSize(), pt->GetProcPath(), pt->GetRawFileList(), psc );
                d->InitDecoder();
                d->Decode();
                break;
            }
            case 5:
            {
                d = new Decoder( 0, 0, "NULL", nullptr, psc );
                d->SetDecoderByFile("/home/lab/lab/intern_work/ProcessorTracerByIntelPT/test/perf.data-aux-idx1.bin");
                d->Decode();
                break;
            }
            case 6:
            {
                d = new Decoder( 0, 0, pt->GetProcPath(), pt->GetRawFileList(), psc );
                d->SetDecoderByFile("/home/lab/lab/intern_work/ProcessorTracerByIntelPT/build/aux.bin");
                d->Decode();
                break;
            }
            default:
            {
                std::cout << "wrong command" << std::endl;
                break;
            }
        }
    }
    
    
    return 0;
}
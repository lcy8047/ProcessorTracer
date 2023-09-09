#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "Decoder.h"
#include "PrintSourceCode.h"
#include "TypeDef.h"

void usage( void )
{
    std::cerr << "decode_tool -a <IPT traced file> -m <maps_file>" << std::endl;
}

int main( int argc, char *argv[] )
{
    std::string aux_file_path;
    std::string data_file_path;
    std::string maps_file_path;
    while ( true ) {
        switch ( getopt( argc, argv, "a:d:m:p" ) )
        {
            case 'a':
            {
                aux_file_path = optarg;
                continue;
            }
            case 'd':
            {
                data_file_path = optarg;
                continue;
            }
            case 'm':
            {
                maps_file_path = optarg;
                continue;
            }
            default:
            {
                break;
            }
        }
        break;
    }
    if ( maps_file_path.empty() || aux_file_path.empty() ) {
        std::cerr << "maps_file_path is empty" << std::endl;
        usage();
    }

    std::vector<raw_file> *_raw_file_list = new std::vector<raw_file>;

    std::ifstream ifs( maps_file_path );
    if ( !ifs.is_open() ) {
        std::cerr << maps_file_path << " open failed" << std::endl;
        return -1;
    }
    uint64_t file_count;
    ifs >> file_count;
    for ( uint64_t i = 0; i < file_count; ++ i ) {
        std::string path;
        uint64_t base;
        uint64_t end;
        ifs >> path >> base >> end;
        _raw_file_list->push_back( { path, base, end } );
    }

    std::string bin_file = _raw_file_list->at(0).path;
    PrintSourceCode *psc = new PrintSourceCode( bin_file );
    //Decoder( *aux, aux_size, bin_path, *raw_file_list, *psc )
    Decoder *d = new Decoder( nullptr, 0, bin_file, _raw_file_list, psc );
    d->SetDecoderByFile( aux_file_path );
    d->Decode();
    return 0;
}
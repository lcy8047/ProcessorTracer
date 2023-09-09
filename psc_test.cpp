#include <string>
#include <unistd.h>

#include "PrintSourceCode.h"

int main( int argc, char *argv[] )
{
    std::string bin_file_path;
    while ( true ) {
        switch ( getopt( argc, argv, "b:" ) )
        {
            case 'b':
            {
                bin_file_path = optarg;
                continue;
            }
            default:
            {
                break;
            }
        }
        break;
    }
    PrintSourceCode *psc = new PrintSourceCode( bin_file_path );
    psc->PrintAllTable();
    return 0;
}
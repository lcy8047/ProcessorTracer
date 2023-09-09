#include <iostream>

#include "PrintSourceCode.h"

void PrintSourceCode::PrintAllTable( void ) const
{
    std::cout << "highest pc : " << std::hex << "0x" << _highest_pc << std::dec << std::endl;
    std::cout << "lowest pc : " << std::hex << "0x" << _lowest_pc << std::dec << std::endl;
    for ( size_t i = 0; i < _src_info_table.size(); ++ i ) {
        if ( ! _src_info_table[i].src_file.empty() ) {
            const src_info &s = _src_info_table[i];
            std::cout << std::hex << "0x" << i << std::dec << " : " << s.src_file << " : " << s.line_num << std::endl;
        }
    }
}

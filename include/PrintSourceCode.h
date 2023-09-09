#ifndef PRINTSOURCECODE_H_
#define PRINTSOURCECODE_H_

#include <cstdint>
#include <libdwarf/libdwarf.h>
#include <string>
#include <unordered_map>
#include <vector>

struct src_info {
    std::string src_file;
    uint64_t line_num;
    std::string func_name;
};

class PrintSourceCode {
public:
    PrintSourceCode( std::string bin_path )
        : _highest_pc ( 0 )
        , _lowest_pc ( __UINT64_MAX__ )
    {
        init( bin_path );
    }
    
    ~PrintSourceCode( void )
    {
        _src_info_table.clear();
    }

    // PrintSourceCode.cpp
    src_info &  GetSrcInfo      ( uint64_t target_addr, uint64_t base );
    src_info &  GetSrcInfo      ( uint64_t pc );
    std::string GetSrcLine      ( std::string &src_file, uint64_t &src_lineno );
    void        PrintSrcLine    ( uint64_t target_addr, uint64_t base );
    void        PrintSrcLine    ( uint64_t pc );

    // Debug.cpp
    void        PrintAllTable   ( void ) const;

private:
    void        init            ( std::string file_path );

    std::vector<src_info> _src_info_table;
    std::string _prev_src;
    uint64_t _prev_line;
    Dwarf_Addr _highest_pc;
    Dwarf_Addr _lowest_pc;
};

#endif /* PRINTSOURCECODE_H_*/
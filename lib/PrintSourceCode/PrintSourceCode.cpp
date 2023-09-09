#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>

#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libdwarf/libdwarf.h>
//#include <libdwarf/dwarf.h>

#include "PrintSourceCode.h"

void PrintSourceCode::init( std::string file_path )
{
    Dwarf_Error error;
    Dwarf_Debug dbg;

    int res;

    res = dwarf_init_path( file_path.c_str(), nullptr, 0, DW_GROUPNUMBER_ANY, 
                    nullptr, nullptr, &dbg, &error );
    
    if ( res != DW_DLV_OK ) {
        std::cerr << "init path is failed" << std::endl;
        std::cerr << "errno : " << dwarf_errno( error ) << ", msg : " << dwarf_errmsg( error ) << std::endl;
        dwarf_dealloc_error( dbg, error );
        return ;
    }

    bool is_info = true;
    // set low - high pc and make table
    for ( int cu_count = 0; ; ++ cu_count ) {
        res = dwarf_next_cu_header_d(dbg, is_info, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, &error);
        if ( res == DW_DLV_NO_ENTRY ) {
            //std::cout << "cu header ret : " << ret << std::endl;
            break;
        }
        if ( res == DW_DLV_ERROR ) {
            std::cerr << "next cu is failed" << std::endl;
            std::cerr << "errno : " << dwarf_errno( error ) << ", msg : " << dwarf_errmsg( error ) << std::endl;
            break;
        }
        Dwarf_Die cu_die = 0;

        res = dwarf_siblingof_b( dbg, 0, is_info, &cu_die, NULL );
        if ( res != DW_DLV_OK ) {
            std::cerr << "siblingof is failed" << std::endl;
            std::cerr << "errno : " << dwarf_errno( error ) << ", msg : " << dwarf_errmsg( error ) << std::endl;
            dwarf_dealloc_die( cu_die );
            continue;
        }
        enum Dwarf_Form_Class form_class;
        Dwarf_Addr high_pc;
        Dwarf_Addr low_pc;
        res = dwarf_lowpc( cu_die, &low_pc, &error );
        if ( res == DW_DLV_OK ) {
            if ( low_pc < _lowest_pc ) {
                _lowest_pc = low_pc;
            }
            res = dwarf_highpc_b( cu_die, &high_pc, NULL, &form_class, &error );
            if ( res == DW_DLV_OK ) {
                if ( form_class == DW_FORM_CLASS_CONSTANT ) {
                    high_pc += low_pc;
                }
                if ( high_pc > _highest_pc ) {
                    _highest_pc = high_pc;
                }
            }
        }
    }
    _src_info_table.resize( _highest_pc - _lowest_pc + 1 );

    for ( int cu_count = 0; ; ++ cu_count ) {
        res = dwarf_next_cu_header_d(dbg, is_info, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr, nullptr, &error);
        if ( res == DW_DLV_NO_ENTRY ) {
            //std::cout << "cu header ret : " << ret << std::endl;
            break;
        }
        if ( res == DW_DLV_ERROR ) {
            std::cerr << "next cu is failed" << std::endl;
            std::cerr << "errno : " << dwarf_errno( error ) << ", msg : " << dwarf_errmsg( error ) << std::endl;
            break;
        }
        Dwarf_Die cu_die = 0;

        res = dwarf_siblingof_b( dbg, 0, is_info, &cu_die, NULL );
        if ( res != DW_DLV_OK ) {
            std::cerr << "siblingof is failed" << std::endl;
            std::cerr << "errno : " << dwarf_errno( error ) << ", msg : " << dwarf_errmsg( error ) << std::endl;
            dwarf_dealloc_die( cu_die );
            continue;
        }

        Dwarf_Line_Context line_context;
        Dwarf_Unsigned version;
        Dwarf_Small table_count;
        res = dwarf_srclines_b( cu_die, &version, &table_count, &line_context, &error );
        if ( res != DW_DLV_OK ) {
            std::cerr << "srclines is failed" << std::endl;
            std::cerr << "errno : " << dwarf_errno( error ) << ", msg : " << dwarf_errmsg( error ) << std::endl;
            dwarf_srclines_dealloc_b( line_context );
            dwarf_dealloc_die( cu_die );
            break;
        }
        std::string func_name;
        Dwarf_Line *lines;
        Dwarf_Signed line_count; 
        res = dwarf_srclines_from_linecontext( line_context, &lines, &line_count, &error );
        if ( res != DW_DLV_OK ) {
            std::cerr << "srclines is failed" << std::endl;
            std::cerr << "errno : " << dwarf_errno( error ) << ", msg : " << dwarf_errmsg( error ) << std::endl;
            dwarf_srclines_dealloc_b( line_context );
            dwarf_dealloc_die( cu_die );
            break;
        }
        
        Dwarf_Addr prev_addr;
        Dwarf_Line prev_line = 0;
        for ( int i = 0; i < line_count; ++ i ) {
            Dwarf_Unsigned lineno;
            char *linesrc;
            Dwarf_Addr addr;
            bool is_ok = true;
            res = dwarf_lineno( lines[i], &lineno, &error );
            if ( res != DW_DLV_OK ) {
                is_ok = false;
            }
            res = dwarf_linesrc( lines[i], &linesrc, &error );
            if ( res != DW_DLV_OK ) {
                is_ok = false;
            }
            res = dwarf_lineaddr( lines[i], &addr, &error );
            if ( res != DW_DLV_OK ) {
                is_ok = false;
            }
            if ( ! is_ok ) {
                continue;
            }
            if ( prev_line ) {
                struct src_info s{ linesrc, lineno, func_name };
                for ( Dwarf_Addr addr_it = prev_addr; addr_it < addr; ++ addr_it ) {
                    _src_info_table[addr - _lowest_pc] = s;
                } 
            }
            //std::cerr << "[" << name << "]" << std::hex << "0x" << addr << std::dec << linesrc << ":" << lineno << std::endl;

            Dwarf_Bool is_lineend;
            res = dwarf_lineendsequence(lines[i], &is_lineend, NULL);
            if ( res == DW_DLV_NO_ENTRY ) {
                continue;
            }
            if ( is_lineend ) {
                prev_line = 0;
            } else {
                prev_addr = addr;
                prev_line = lines[i];
            }
        }
        dwarf_srclines_dealloc_b( line_context );
        dwarf_dealloc_die( cu_die );
    }
    dwarf_dealloc_error( dbg, error );
    dwarf_finish( dbg );
}

src_info &PrintSourceCode::GetSrcInfo( uint64_t target_addr, uint64_t base )
{
    
    Dwarf_Addr pc = target_addr - base;
    if ( base == 0x400000 || base == 0x8000000 ) {
        pc += base;
    }

    return _src_info_table[pc - _lowest_pc];
}

src_info &PrintSourceCode::GetSrcInfo( uint64_t pc )
{
    return _src_info_table[pc - _lowest_pc];
}

std::string PrintSourceCode::GetSrcLine( std::string &src_file, uint64_t &src_lineno )
{
    std::ifstream code_file_stream( src_file );
    std::string code_line;
    for ( uint64_t i = 1; i <= src_lineno; ++ i ) {
        getline( code_file_stream, code_line );
    }
    code_file_stream.close();
    return code_line;
}

void PrintSourceCode::PrintSrcLine( uint64_t target_addr, uint64_t base )
{

    Dwarf_Addr pc = target_addr - base;
    if ( base == 0x400000 || base == 0x8000000 ) {
        pc += base;
    }

    struct src_info src = _src_info_table[pc - _lowest_pc];
    if ( src.func_name.empty() && src.src_file.empty() && src.line_num == 0) {
        return;
    }

    if ( _prev_src.compare( src.src_file ) == 0 && _prev_line == src.line_num ) {
        //return;
    }

    std::ifstream code_file_stream( src.src_file );
    std::string code_line;
    for ( uint64_t i = 1; i <= src.line_num; ++ i ) {
        getline( code_file_stream, code_line );
    }
    code_file_stream.close();
    
    for( size_t i = 0; i < code_line.length(); ++ i ) {
        if ( code_line[i] == '\t' ) {
            code_line.replace( i, 1, "    " );
        }
    }

    while( code_line.back() == ' ' ) {
        code_line.pop_back();
    }
    //std::cout << linesrc << ":" << lineno << "\t" << code_line << std::endl;
    std::cout << std::left << std::setw(82) << code_line << src.src_file << ":" << std::dec << src.line_num << std::endl;
    _prev_src = src.src_file;
    _prev_line = src.line_num;

}

void PrintSourceCode::PrintSrcLine( uint64_t pc )
{
    struct src_info src = _src_info_table[pc - _lowest_pc];
    if ( src.func_name.empty() && src.src_file.empty() && src.line_num == 0) {
        return;
    }

    if ( _prev_src.compare( src.src_file ) == 0 && _prev_line == src.line_num ) {
        //return;
    }

    std::ifstream code_file_stream( src.src_file );
    std::string code_line;
    for ( uint64_t i = 1; i <= src.line_num; ++ i ) {
        getline( code_file_stream, code_line );
    }
    code_file_stream.close();
    
    for( size_t i = 0; i < code_line.length(); ++ i ) {
        if ( code_line[i] == '\t' ) {
            code_line.replace( i, 1, "    " );
        }
    }

    while( code_line.back() == ' ' ) {
        code_line.pop_back();
    }
    //std::cout << linesrc << ":" << lineno << "\t" << code_line << std::endl;
    std::cout << std::left << std::setw(82) << code_line << src.src_file << ":" << std::dec << src.line_num << std::endl;
    _prev_src = src.src_file;
    _prev_line = src.line_num;
}

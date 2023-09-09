#include <fstream>
#include <iostream>
#include <intel-pt.h>
#include <linux/perf_event.h>
#include <string>
#include <sstream>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <vector>

#include "ProcessorTracer.h"
#include "TypeDef.h"

ProcessorTracer::ProcessorTracer( pid_t pid, bool is_comm, pid_t tid )
    : _raw_file_list ( new std::vector<raw_file> )
    , _is_comm       ( is_comm )
    , _pid           ( pid )
    , _tid           ( tid )
{
    if ( tid == 0 ) {
        _tid = pid;
    }
    if ( !is_comm ) {
        char buf[1024];
        ssize_t len;

        std::string exe_link = "/proc/" + std::to_string( _pid ) + "/exe";

        len = readlink( exe_link.c_str(), buf, sizeof( buf )-1 );
        if ( len == -1 ) {
            _bin_path = "cannot get path";
        }
        else {
            buf[len] = '\0';
            _bin_path = buf;
        }
        std::cout << "Binary Path : " << _bin_path << std::endl;

        std::string maps_path = "/proc/" + std::to_string( _pid ) + "/maps";
        std::ifstream ifs( maps_path );
        if ( !ifs.is_open() ) {
            std::cerr << "open failed : " << maps_path << std::endl;
            return ;
        }

        std::string s;
        std::string prev_path;
        uint64_t base;
        uint64_t end;
        while ( getline( ifs, s ) ) {
            if ( s.find("/") == std::string::npos ) {
                continue;
            }
            std::string path = s.substr( s.find("/"), s.size() - s.find("/") );
            if ( path != prev_path ) {
                std::string base_str = s.substr( 0, s.find("-") );
                base = std::stoull( base_str, 0, 16 );
            }
            prev_path = path;
            if ( s.find( "r-xp" ) != std::string::npos ) {
                //std::cout << s << std::endl;
                std::string end_str = s.substr( s.find("-") + 1, s.find(" ") - s.find("-") - 1 );
                end = std::stoull( end_str, 0, 16 );
                _raw_file_list->push_back( { path, base, end } );
            }
        }
        ifs.close();

        std::string comm_path = "/proc/" + std::to_string( _pid ) + "/comm";
        std::ifstream ifs_comm( comm_path );
        if ( !ifs_comm.is_open() ) {
            std::cerr << comm_path << " open failed" << std::endl;
            return; 
        }
        ifs_comm >> _process_comm;
        ifs_comm.close();
    
        comm_path = "/proc/" + std::to_string( _pid ) + "/task/" + std::to_string( _tid ) + "/comm";
        std::ifstream ifs_th_comm( comm_path );
        if ( !ifs_th_comm.is_open() ) {
            std::cerr << comm_path << " open failed" << std::endl;
            return; 
        }
        ifs_th_comm >> _thread_comm;
        ifs_th_comm.close();
    }
}

uint8_t* ProcessorTracer::GetAux( void )
{
    return _aux;
}

uint64_t ProcessorTracer::GetAuxSize( void )
{
    struct perf_event_mmap_page *header;
    header = ( struct perf_event_mmap_page * )_base;
    return header->aux_size;
}

int ProcessorTracer::GetIntelPtPmuValue( std::string path )
{
    std::string value;
    std::ifstream ifs( path );
    if ( !ifs.is_open() ) {
        std::cerr << "open failed : " << path << std::endl;
        exit( -1 );
    }
    ifs >> value;
    size_t found = value.find( ":" );
    return std::stoi( value.substr( found+1, value.size() - found - 1 ) );
}

uint32_t ProcessorTracer::GetIntelPtType( void )
{
    uint32_t type;
    // std::ifstream ifs( "/sys/devices/intel_pt/type" );
    std::ifstream ifs( "/sys/bus/event_source/devices/intel_pt/type" );
    if ( !ifs.is_open() ) {
        std::cerr << "/sys/bus/event_source/devices/intel_pt/type open failed" << std::endl;
        exit( -1 );
    }
    ifs >> type;
    ifs.close();
    return type;
}

pid_t ProcessorTracer::GetPid( void )
{
    return _pid;
}

std::string ProcessorTracer::GetProcPath( void )
{
    return _bin_path;
}

std::vector<raw_file>* ProcessorTracer::GetRawFileList( void )
{
    return _raw_file_list;
}

std::string ProcessorTracer::GetSavedAuxFileName( void )
{
    return _saved_aux_file_name;
}

void ProcessorTracer::InitCapture( void )
{
    // init attr
    
    memset( &_attr, 0, sizeof( _attr ) );
    _attr.config         = MakeConfig();
    _attr.size           = sizeof( _attr );
    _attr.type           = GetIntelPtType();
    _attr.mmap           = 1;
    _attr.exclude_kernel = 1;
    _attr.task           = 1;

    // If use binary, then set enable_on_exec 1.
    if ( _is_comm ) {
        _attr.enable_on_exec = 1;
        _attr.disabled       = 1;
    }
    
    // call perf_event_open syscall
    //  struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags);
    _fd = syscall( SYS_perf_event_open, &_attr, _tid, -1, -1, 0 );
    std::cout << "fd : " << _fd << std::endl;
    if ( _fd == -1 ) {
        std::cerr << "perf_event_open is failed : " << _tid << std::endl;
        return ;
    }

    // setting AUX and DATA area
    long PAGESIZE = sysconf( _SC_PAGESIZE );
    struct perf_event_mmap_page *header;

    _base = (uint8_t*) mmap( NULL, ( 1 + 128 ) * PAGESIZE, PROT_WRITE, MAP_SHARED, _fd, 0 );
    if ( _base == MAP_FAILED ) {
        std::cerr << "base : mmap failed : " << errno << std::endl;
        exit( -1 );
    }

    header = ( struct perf_event_mmap_page * )_base;
    _data = (uint8_t *)_base + header->data_offset;

    header->aux_offset = header->data_offset + header->data_size;
    header->aux_size   = 128 * 128 * PAGESIZE;
    if ( _thread_comm == "wq:manager" || _thread_comm == "wq:rate_ctrl" ) {
        header->aux_size *= 32;
    }

    if ( _thread_comm == "mavlink_rcv_if1" )
    {
        header->aux_size *= 32;
    }

    _aux = (uint8_t*) mmap( NULL, header->aux_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, header->aux_offset );
    if ( _aux == MAP_FAILED ) {
        std::cerr << "aux : mmap failed : " << errno << std::endl;
        return ;
    }
    ioctl(_fd, PERF_EVENT_IOC_RESET, 0);
}

uint32_t ProcessorTracer::MakeConfig( void )
{
    uint32_t config = 0;
    // first setting
    config |= 0b1 << parseBit( "pt" ); 
    config |= 0b1 << parseBit( "tsc" );
    config |= 0b1 << parseBit( "noretcomp" );
    config |= 0b1 << parseBit( "branch" );
    //config |= 0b11 << parseBit( "psb_period" );
    //config |= 0b1 << parseBit( "mtc" );
    //config |= 6 << parseBit( "mtc_period" );
    config |= 0b1 << parseBit( "cyc" );
    config |= 0 << parseBit( "cyc_thresh" );

    return config;
}

void *ProcessorTracer::MapFile(char *fn, size_t *size)
{
    const int PAGESIZE = sysconf(_SC_PAGESIZE);
	int fd = open(fn, O_RDWR);
	if (fd < 0)
		return NULL;
	struct stat st;
	void *map = (void *)-1L;
	if (fstat(fd, &st) >= 0) {
		*size = st.st_size;
		map = mmap(NULL, round_up(st.st_size, PAGESIZE),
			   PROT_READ|PROT_WRITE,
			   MAP_PRIVATE, fd, 0);
	}
	close(fd);
	return map != (void *)-1L ? map : NULL;
}

int ProcessorTracer::parseBit( std::string term )
{   
    std::string target_file = "/sys/bus/event_source/devices/intel_pt/format/" + term;
    std::ifstream ifs( target_file );
    if ( !ifs.is_open() ) {
        std::cerr << target_file + " open failed" << std::endl;
        exit( -1 );
    }
    std::string config;
    ifs >> config;
    ifs.close();

    std::string bit;
    if ( config.find( "-" ) != std::string::npos ) {
        bit = config.substr( config.find(":") + 1, config.find( "-" ) - config.find( ":" ) - 1 );
    }
    else {
        bit = config.substr( config.find(":") + 1, 2 );
    }

    return std::stoi( bit );
}

void ProcessorTracer::SaveAuxFile( void )
{
    struct perf_event_mmap_page *header;
    header = ( struct perf_event_mmap_page * )_base;
    std::string filename = "aux.bin";
    if ( !_process_comm.empty() ) {
        filename = _process_comm + "_" + filename;
    }
    if ( !_thread_comm.empty() ) {
        filename = _thread_comm + "_" + filename;
    }
    filename = std::to_string( _pid ) + "_" + filename;
    filename = std::to_string( _tid ) + "_" + filename;
    if ( filename.find( ":" ) != std::string::npos ) {
        filename.replace( filename.find( ":" ), 1, "_" );
    }
    FILE *aux_fp = fopen( filename.c_str(), "w" );
    if ( aux_fp == NULL )
    {
        std::cerr << filename << " open error : " << strerror( errno ) << std::endl;
        exit( 1 );
    }
    std::cout << "save " << filename << ".." << std::endl;
    fwrite( _aux, sizeof( char ), header->aux_size, aux_fp );
    std::cout << "done" << std::endl;
    fclose( aux_fp );
    // std::ofstream ofs_aux( filename, std::ios_base::binary );
    // if ( !ofs_aux.is_open() ) {
    //     std::cerr << filename << " open error : " << strerror( errno ) << std::endl;
    //     exit( 1 );
    // }
    // std::cout << "save " << filename << ".." << std::endl;
    // ofs_aux.write( (const char *)_aux, header->aux_size );
    // ofs_aux.close();
    // std::cout << "done" << std::endl;
    _saved_aux_file_name = filename;
}

void ProcessorTracer::SaveDataFile( void )
{
    struct perf_event_mmap_page *header;
    header = ( struct perf_event_mmap_page * )_base;

    std::string filename = std::to_string( _pid ) + "_data.bin";
    std::ofstream ofs_data( filename, std::ios_base::binary );
    if ( !ofs_data.is_open() ) {
        std::cerr << "data.bin open error" << std::endl;
        exit( 1 );
    }
    ofs_data.write( (const char *)_data, header->data_size );
    ofs_data.close();
}

void ProcessorTracer::SaveMapsFile( void )
{
    std::string filename = std::to_string( _pid ) + "_maps_data";
    //std::string filename = "maps_data";
    std::ofstream ofs_maps( filename, std::ios_base::binary );
    if ( !ofs_maps.is_open() ) {
        std::cerr << "maps_data open error" << std::endl;
        exit( 1 );
    }
    ofs_maps << _raw_file_list->size() << std::endl;
    for ( auto it : *_raw_file_list ) {
        ofs_maps << it.path << std::endl;
        ofs_maps << it.base << std::endl;
        ofs_maps << it.end << std::endl;
    }
    ofs_maps.close();
}

void ProcessorTracer::SetFilter( std::string path )
{
    if ( path.empty() ) {
        path = _bin_path;
    }
    std::string filter = "filter * @ " + path;
    int ret;
    ret = ioctl(_fd, PERF_EVENT_IOC_SET_FILTER, filter.c_str());
    if ( ret == 0 ) {
        std::cerr << "set filter failed : " << filter << std::endl;
        return ;
    }
}

void ProcessorTracer::StartTrace( void )
{
    ioctl(_fd, PERF_EVENT_IOC_ENABLE, 0);
}

void ProcessorTracer::StopTrace( void )
{
    ioctl(_fd, PERF_EVENT_IOC_DISABLE, 0);
    for ( int i = 0; i < 8; ++ i ) {
        std::cout << std::hex << (int)_aux[i] << " " << std::dec;
    }
    std::cout << std::endl;
    
    if ( _is_comm ) {
        // save maps data
        uint8_t *data_addr = _data + 16;
        int size = 0;
        struct mmap map;
        struct raw_file raw;
        while ( true ) {
            memcpy( &map, data_addr, sizeof( struct mmap ) );
            size = sizeof( struct mmap );
            if ( map.tid != ( uint32_t )_pid ) {
                std::cerr << "end of data" << std::endl;
                std::cerr << ( const char * ) data_addr + size << std::endl;
                break;
            }
            raw.path = ( const char * ) data_addr + size;
            size += raw.path.size();
            size += 8 - ( raw.path.size() ) % 8; // 8 bytes padding
            data_addr += size;
            if ( raw.path[0] != '/' ) { 
                continue;
            }
            raw.base = map.addr - map.pgoff;
            raw.end = map.addr + map.len;
            _raw_file_list->push_back( raw );
        }
        if ( _raw_file_list->empty() ) {
            std::cout << "data is empty" << std::endl;
        }
        else {
            _bin_path = _raw_file_list->at(0).path;
        }
    }
}

void ProcessorTracer::SetPerfDisable( void )
{
    ioctl( _fd, PERF_EVENT_IOC_DISABLE, 0 );
}
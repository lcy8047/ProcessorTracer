#include "Decoder.h"
#include "PrintSourceCode.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

void Decoder::InitDecoder( void )
{
	std::cout << "[init decoder]" << std::endl;

    pt_config_init( &_config );
    
    _iscache = pt_iscache_alloc( NULL );
    if ( !_iscache ) {
        std::cerr << "init error : " << pt_errstr( pt_errcode( -pte_nomem ) ) << std::endl;
        goto err;
    }

    _image = pt_image_alloc(NULL);
    if ( !_image ) {
        std::cerr << "Failed to allocate image" << std::endl;
        goto err;
    }

	loadRawFilesToCache();

    // load trace data
    _config.begin = _aux;
    _config.end   = _aux + _aux_size;
    
    // decoder alloc
    _decoder = pt_insn_alloc_decoder( &_config );
    if ( !_decoder ) {
        std::cerr << "Failed to create decoder" << std::endl;
        goto err;
    }

    int error_code;
    error_code = pt_insn_set_image( _decoder, _image );
    if ( error_code < 0 ) {
        std::cerr << "Failed to set image" << std::endl;
        goto err;
    }

	return;
err:
    pt_insn_free_decoder( _decoder );
    pt_iscache_free( _iscache );
    delete _config.begin;
}

int Decoder::loadRawFilesToCache( void )
{
	int isid, errcode;

    for ( auto raw : *_raw_file_list ) {
        isid = pt_iscache_add_file( _iscache, raw.path.c_str(), 0, UINT64_MAX, raw.base );
        if (isid < 0) {
            std::cerr << "Failed to add << " << raw.path << " at 0x" << std::hex << raw.base
                      << pt_errstr( pt_errcode( isid ) ) << std::dec;
            return -1;
        }

        errcode = pt_image_add_cached(_image, _iscache, isid, NULL);
        if (errcode < 0) {
            std::cerr << "Failed to add << " << raw.path << " at 0x" << std::hex << raw.base
                      << pt_errstr( pt_errcode( errcode ) ) << std::dec;
            return -1;
        }
		std::cout << raw.path << " : " << std::hex << raw.base << " ~ " << raw.end << " loaded" << std::endl << std::dec;

        if ( _bin_path == raw.path ) {
            _bin_base = raw.base;
            _bin_end = raw.end;
        }
    }

	return 0;
}

void Decoder::ClearDecoder( void )
{
    pt_insn_free_decoder( _decoder );
    pt_iscache_free( _iscache );
    delete _config.begin;
}

void Decoder::Decode( void )
{
	if ( !_decoder ) {
		std::cerr << "decoder is not initialized" << std::endl;
		return;
	}

	int status;
    status = pt_insn_sync_forward( _decoder );
    if ( status < 0 ) {
        if ( status == -pte_eos ) {
			std::cerr << "end of trace" << std::endl;
            return;
        }
        else {
            std::cerr << "error : " << std::dec << status << std::endl;
            return;
        }
    }

    while ( true ) {
        while ( status & pts_event_pending ) {
            struct pt_event event;

            status = pt_insn_event( _decoder, &event, sizeof( event ) );
            if ( status < 0 )
                break;
        }
        if ( status < 0 ) {
            break;
        }
        if ( status & pts_eos ) {
            std::cerr << "end of trace" << std::endl;
            break;
        }
        struct pt_insn insn;
        status = pt_insn_next( _decoder, &insn, sizeof( insn ) );
        //std::cout << "insn_next : " << std::dec << status << std::endl;
        if ( status < 0 ) {
            if ( status == -pte_eos ) {
                printSrcCode( &insn );
                break;
            }
            else { 
                std::cout << "error : " << std::dec << status << std::endl;
                return;
            }
        }
        printSrcCode( &insn );
    }

	if ( !_decoded_bin_code ) {
		std::cout << "[decoder] bin code was not traced" << std::endl;
	}
}

void Decoder::printSrcCode( const struct pt_insn *insn )
{
	uint64_t ip = insn->ip;
	
	for ( auto raw : *_raw_file_list ) {
		if ( raw.base <= ip && ip <= raw.end ) {
			std::string path = raw.path;
			uint64_t base = raw.base;
			if ( path == _bin_path ) {
				//std::cout << "\n" << path << " offset : 0x" << std::hex << ip - base << std::endl;
				_psc->PrintSrcLine( ip, base );
				_decoded_bin_code = true;
			}
		}
        break;
	}
}

void Decoder::SetDecoderByFile( std::string aux_file_path )
{
	std::ifstream ifs( aux_file_path , std::ifstream::binary );
	if ( !ifs.is_open() ) {
		std::cerr << "aux file open failed" << std::endl;
		return;
	}
	ifs.seekg( 0, std::ios::end );
	_aux_size = ifs.tellg();
	_aux = new uint8_t[_aux_size];
	ifs.seekg( 0, std::ios::beg );
	ifs.read( (char *)_aux, _aux_size );
    ifs.close();

	InitDecoder();
}

void Decoder::AppendToTracedList( const struct pt_insn *insn )
{
    uint64_t ip = insn->ip;
	
	for ( auto raw : *_raw_file_list ) {
		if ( raw.base <= ip && ip <= raw.end ) {
			std::string path = raw.path;
			uint64_t base = raw.base;
			if ( path == _bin_path ) {
                src_info &srcinfo = _psc->GetSrcInfo( ip, base );
				if ( _traced_list->find( srcinfo.src_file ) == _traced_list->end() ) {
                    _traced_list->emplace( srcinfo.src_file, new std::set<uint64_t> );
                }
                _traced_list->at( srcinfo.src_file )->insert( srcinfo.line_num );
			}
            break;
		}
	}
}

void Decoder::DecodeForLLVM( void )
{
    if ( !_decoder ) {
		std::cerr << "decoder is not initialized" << std::endl;
		return;
	}

	int status;
    status = pt_insn_sync_forward( _decoder );
    if ( status < 0 ) {
        if ( status == -pte_eos ) {
			std::cerr << "end of trace" << std::endl;
            return;
        }
        else {
            std::cerr << "error : " << std::dec << status << std::endl;
            return;
        }
    }

    while ( true ) {
        while ( status & pts_event_pending ) {
            struct pt_event event;

            status = pt_insn_event( _decoder, &event, sizeof( event ) );
            if ( status < 0 )
                break;
        }
        if ( status < 0 ) {
            break;
        }
        if ( status & pts_eos ) {
            std::cerr << "end of trace" << std::endl;
            break;
        }
        struct pt_insn insn;
        status = pt_insn_next( _decoder, &insn, sizeof( insn ) );
        //std::cout << "insn_next : " << std::dec << status << std::endl;
        if ( status < 0 ) {
            if ( status == -pte_eos ) {
                AppendToTracedList( &insn );
                break;
            }
            else { 
                std::cout << "error : " << std::dec << status << std::endl;
                return;
            }
        }
        AppendToTracedList( &insn );
    }

	if ( !_decoded_bin_code ) {
		std::cout << "[decoder] bin code was not traced" << std::endl;
	}
}

void Decoder::saveSrcCode( const struct pt_insn *insn )
{
	uint64_t ip = insn->ip;
	
	for ( auto raw : *_raw_file_list ) {
		if ( raw.base <= ip && ip <= raw.end ) {
			std::string path = raw.path;
			uint64_t base = raw.base;
			if ( path == _bin_path ) {
				src_info &srcinfo = _psc->GetSrcInfo( ip, base );
                std::string src_line = _psc->GetSrcLine( srcinfo.src_file, srcinfo.line_num );
                if ( src_line.empty() ) {
                    //std::cerr << "base : " << base << std::endl;
                    //std::cerr << "not found line : 0x" << std::hex << ip << std::endl;
                    continue;
                }
                std::string src_file = srcinfo.src_file.substr( src_file.rfind('/') );
                std::ofstream ofs( "result_trace.txt", std::ios_base::app );
                ofs << std::left << std::setw(82) << src_line << src_file << ":" << std::dec << srcinfo.line_num << std::endl;
                ofs.close();
                // save to file
			}
		}
        break;
	}
}

void Decoder::DecodeForSaving( void )
{
	if ( !_decoder ) {
		std::cerr << "decoder is not initialized" << std::endl;
		return;
	}

	int status;
    status = pt_insn_sync_forward( _decoder );
    if ( status < 0 ) {
        if ( status == -pte_eos ) {
			std::cerr << "end of trace" << std::endl;
            return;
        }
        else {
            std::cerr << "error : " << std::dec << status << std::endl;
            return;
        }
    }

    while ( true ) {
        while ( status & pts_event_pending ) {
            struct pt_event event;

            status = pt_insn_event( _decoder, &event, sizeof( event ) );
            if ( status < 0 )
                break;
        }
        if ( status < 0 ) {
            break;
        }
        if ( status & pts_eos ) {
            std::cerr << "end of trace" << std::endl;
            break;
        }
        struct pt_insn insn;
        status = pt_insn_next( _decoder, &insn, sizeof( insn ) );
        //std::cout << "insn_next : " << std::dec << status << std::endl;
        if ( status < 0 ) {
            if ( status == -pte_eos ) {
                saveSrcCode( &insn );
                break;
            }
            else { 
                std::cout << "error : " << std::dec << status << std::endl;
                return;
            }
        }
        saveSrcCode( &insn );
    }

	if ( !_decoded_bin_code ) {
		std::cout << "[decoder] bin code was not traced" << std::endl;
	}
}
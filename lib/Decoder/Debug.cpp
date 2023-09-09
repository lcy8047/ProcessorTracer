#include "Decoder.h"
//////////////////////
// public functions //
//////////////////////

void Decoder::DecodeForDebug( void )
{
    std::cout << "> Decode For Debug <" << std::endl;
    if ( !_decoder ) {
		std::cerr << "decoder is not initialized" << std::endl;
		return;
	}

	int status;
    status = pt_insn_sync_forward( _decoder );
    if ( status < 0 ) {
        if ( status == -pte_eos ) {
			std::cerr << "sync : end of trace" << std::endl;
            return;
        }
        else {
            std::cerr << "sync : error : " << std::dec << status << std::endl;
            return;
        }
    }

    while ( true ) {
        while ( status & pts_event_pending ) {
            struct pt_event event;

            status = pt_insn_event( _decoder, &event, sizeof( event ) );
            if ( status < 0 ) {
                if ( status == -pte_eos ) {
                    std::cerr << "insn event eos" << std::endl;
                    break;
                }
                else {
                    std::cerr << "inst event error : " << std::dec << status << std::endl;
                    break;
                }
            }
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

        if ( status < 0 ) {
            if ( status == -pte_eos ) {
                handleForDebug( &insn );
                break;
            }
            else {
                std::cerr << "inst next error : " << std::dec << status << std::endl;
                return;
            }
        }
        handleForDebug( &insn );
    }
    std::cout << "loop list len : " << std::dec << _loop_list->size() << std::endl;
    
    pt_insn_free_decoder( _decoder );
    pt_iscache_free( _iscache );
    delete _config.begin;
}

void Decoder::PrintDebugInfo( void ) const
{
    for ( auto file_name : _debug_info.executed_src_file_list ) {
        std::cout << file_name << std::endl;
    }
}

///////////////////////
// private functions //
///////////////////////

void Decoder::handleForDebug( const struct pt_insn *insn )
{
    uint64_t ip = insn->ip;

    if ( _bin_base <= ip && ip <= _bin_end ) {

        std::string path = _bin_path;
        uint64_t base = _bin_base;

        src_info &srcinfo = _psc->GetSrcInfo( ip, base );
        //std::cout << srcinfo.func_name << std::endl;
        _debug_info.executed_src_file_list.insert( srcinfo.src_file );
    }
}
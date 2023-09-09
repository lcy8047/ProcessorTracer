#ifndef DECODER_H_
#define DECODER_H_

#include <intel-pt.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "PrintSourceCode.h"
#include "TypeDef.h"

using LoopInfoList  = std::vector<loop_info*>;
using TracedListMap = std::map<std::string, std::set<uint64_t>*>;

struct DebugInfo {
    std::set<std::string> executed_src_file_list;
};

class Decoder {
public:
    Decoder( uint8_t *aux, size_t aux_size, std::string bin_path, 
            std::vector<raw_file> *raw_file_list, PrintSourceCode *psc )
        : _aux              ( aux )
        , _aux_size         ( aux_size )
        , _bin_path         ( bin_path )
        , _blk_count        ( 0 )
        , _decoded_bin_code ( false )
        , _psc              ( psc )
        , _raw_file_list    ( raw_file_list )
        , _traced_list      ( new TracedListMap )
    {}
    
    ~Decoder()
    {
        pt_insn_free_decoder( _decoder );
        pt_iscache_free( _iscache );
        delete _config.begin;
    }

    TracedListMap* GetTracedList( void ) {
        return _traced_list;
    }
    
    std::vector<loop_info*>* GetLoopList( void ) {
        return _loop_list;
    }

    // Debug.cpp
    void    DecodeForDebug      ( void );
    void    PrintDebugInfo      ( void ) const;

    // Decoder.cpp
    void    AppendToTracedList  ( const struct pt_insn *insn );
    void    ClearDecoder        ( void );
    void    Decode              ( void );
    void    DecodeForLLVM       ( void );
    void    DecodeForSaving     ( void );
    void    InitDecoder         ( void );
    void    SetDecoderByFile    ( std::string aux_file_path );

private:
    // Debug.cpp
    void    handleForDebug      ( const struct pt_insn *insn );

    // Decoder.cpp
    int     loadRawFilesToCache ( void );
    void    makeRawFileList     ( void );
    void    printSrcCode        ( const struct pt_insn *insn );
    void    saveSrcCode         ( const struct pt_insn *insn );

    uint8_t *                   _aux;
    size_t                      _aux_size;
    uint64_t                    _bin_base;
    uint64_t                    _bin_end;
    std::string                 _bin_path;
    uint64_t                    _blk_count;
    struct pt_config            _config;
    DebugInfo                   _debug_info;
    bool                        _decoded_bin_code;
    pt_insn_decoder *           _decoder;
    pt_image *                  _image;
    pt_image_section_cache *    _iscache;
    PrintSourceCode *           _psc;
    std::vector<raw_file> *     _raw_file_list;
    TracedListMap *             _traced_list;
};

#endif /* DECODER_H_ */
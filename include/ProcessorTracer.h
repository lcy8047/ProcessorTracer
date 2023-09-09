#ifndef PROCESSORTRACER_H_
#define PROCESSORTRACER_H_

#include <cstddef>
#include <cstdint>
#include <linux/perf_event.h>
#include <string>
#include <vector>

#include "TypeDef.h"

#define round_up(x, y) (((x) + (y) - 1) & ~((y) - 1))
#define BIT(n)  (1UL << (n))

class ProcessorTracer {
public:
                            ProcessorTracer     ( pid_t pid, bool is_comm=false, pid_t tid=0 );
    uint8_t*                GetAux              ( void );
    uint64_t                GetAuxSize          ( void );
    int                     GetIntelPtPmuValue  ( std::string path );
    uint32_t                GetIntelPtType      ( void );
    pid_t                   GetPid              ( void );
    std::string             GetProcPath         ( void );
    std::vector<raw_file>*  GetRawFileList      ( void );
    std::string             GetSavedAuxFileName ( void );
    void                    InitCapture         ( void );
    uint32_t                MakeConfig          ( void );
    void*                   MapFile             ( char *fn, size_t *size );
    void                    ResetRawFiles       ( void );
    void                    SaveAuxFile         ( void );
    void                    SaveDataFile        ( void );
    void                    SaveMapsFile        ( void );
    void                    SetFilter           ( std::string path="" );
    void                    SetPerfDisable      ( void );
    void                    StartTrace          ( void );
    void                    StopTrace           ( void );    

private:
    int                     parseBit            ( std::string term );
    uint8_t                 *_aux;
    uint8_t                 *_base;
    uint8_t                 *_data;
    std::vector<raw_file>   *_raw_file_list;
    struct perf_event_attr  _attr;
    std::string             _bin_path;    
    int                     _fd;
    bool                    _is_comm;
    pid_t                   _pid;
    std::string             _process_comm;
    std::string             _saved_aux_file_name;
    std::string             _thread_comm;
    pid_t                   _tid;
    
};

#endif /* PROCESSORTRACER_H_ */
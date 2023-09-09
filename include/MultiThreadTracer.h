#ifndef MULTIHREADTRACER_H_
#define MULTIHREADTRACER_H_

#include <map>
#include <set>
#include <unistd.h>
#include <vector>

#include "Decoder.h"
#include "ProcessorTracer.h"

struct ThreadInfo {
    pid_t tid;
    std::string comm;
};

using ThreadFilterList  = std::set<std::string>;
using ThreadList        = std::vector<ThreadInfo>;
using TracerList        = std::vector<ProcessorTracer *>;

class MultiThreadTracer {
public:
    MultiThreadTracer( pid_t pid, ThreadFilterList *thread_filter = new ThreadFilterList )
        : _thread_filter    ( thread_filter )
        , _traced_list      ( new TracedListMap )
        , _pid              ( pid )
    {}
    void InitTracer     ( void );
    void StartTrace     ( void );
    void StopTrace      ( void );
    void Decode         ( void );
    void DecodeForLLVM  ( void );
    void SaveAuxData    ( void );
    void SaveMapsData   ( void );
    TracedListMap *GetTracedList( void )
    {
        return _traced_list;
    }
private:
    ThreadFilterList *  _thread_filter;
    TracedListMap *     _traced_list;
    pid_t               _pid;
    TracerList          _pts;
    ThreadList          _threads;
};

#endif /* MULTIHREADTRACER_H_ */
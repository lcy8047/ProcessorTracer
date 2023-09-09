#include <iostream>
#include <filesystem> // c++17
#include <fstream>
#include <string>

#include "MultiThreadTracer.h"
#include "ProcessorTracer.h"
#include "Decoder.h"
#include "PrintSourceCode.h"

void MultiThreadTracer::InitTracer( void )
{
    std::string task_path = "/proc/" + std::to_string( _pid ) + "/task";
    const std::filesystem::path tasks{ task_path };

    for ( auto const& dir_entry : std::filesystem::directory_iterator{ tasks } ) {
        std::string task = dir_entry.path().string();
        size_t offset = task.find_last_of( '/' );
        pid_t tid = std::stoi( task.substr( offset + 1 ) );

        std::string comm_path = task_path + "/" + std::to_string( tid ) + "/comm";
        std::ifstream ifs( comm_path );
        if ( !ifs.is_open() ) {
            std::cerr << comm_path << " open error" << std::endl;
            continue;
        }
        std::string comm;
        ifs >> comm;        

        if ( _thread_filter->find( comm ) != _thread_filter->end() ) {
            continue;
        }
        _threads.push_back( { tid, comm } );
    }

    for ( auto thr : _threads ) {
        std::cout << thr.tid << " : " << thr.comm << std::endl;
    }
}

void MultiThreadTracer::StartTrace( void )
{
    for ( size_t i = 0; i < _threads.size(); ++ i ) {
        _pts.push_back( new ProcessorTracer( _pid, false, _threads[i].tid ) );
    }

    for ( size_t i = 0; i < _threads.size(); ++ i ) {
        _pts[i]->InitCapture();
    }

    for ( size_t i = 0; i < _threads.size(); ++ i ) {
        _pts[i]->StartTrace();
    }
}

void MultiThreadTracer::StopTrace( void )
{
    for ( size_t i = 0; i < _threads.size(); ++ i ) {
        _pts[i]->SetPerfDisable();
    }

    for ( size_t i = 0; i < _threads.size(); ++ i ) {
        _pts[i]->StopTrace();
    }
}

void MultiThreadTracer::Decode( void )
{
    PrintSourceCode *psc = new PrintSourceCode( _pts[0]->GetProcPath() );
    int count = 0;
    for ( auto pt : _pts ) {
        std::cout << "-------------------------------------------" << std::endl;
        Decoder *d = new Decoder( pt->GetAux(), pt->GetAuxSize(), pt->GetProcPath(), pt->GetRawFileList(), psc );
        d->InitDecoder();
        if ( _threads[count].comm == "wq:manager" ) {
            d->DecodeForSaving();
        }
        else {
            //d->Decode();
        }
        count ++;
    }
}

void MultiThreadTracer::DecodeForLLVM( void )
{
    PrintSourceCode *psc = new PrintSourceCode( _pts[0]->GetProcPath() );
    for ( auto pt : _pts ) {
        std::cout << "-------------------------------------------" << std::endl;
        Decoder *d = new Decoder( pt->GetAux(), pt->GetAuxSize(), pt->GetProcPath(), pt->GetRawFileList(), psc );
        d->InitDecoder();
        d->DecodeForLLVM();
        for ( auto it : *d->GetTracedList() ) {
            if ( _traced_list->find( it.first ) == _traced_list->end() ) {
                _traced_list->emplace( it );
            }
            else {
                for ( auto line : *it.second ) {
                    _traced_list->at( it.first )->insert( line );
                }
            }
        }
    }
}

void MultiThreadTracer::SaveAuxData( void )
{
    for ( auto pt : _pts ) {
        pt->SaveAuxFile();
    }
}

void MultiThreadTracer::SaveMapsData( void )
{
    _pts[0]->SaveMapsFile();
}
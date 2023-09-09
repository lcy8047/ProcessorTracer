#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#include <linux/perf_event.h>
#include <string>
#include <vector>

struct raw_file {
    std::string path;
    uint64_t    base;
    uint64_t    end;
};

struct mmap {
    struct perf_event_header header;
    uint32_t    pid;
    uint32_t    tid;
    uint64_t    addr;
    uint64_t    len;
    uint64_t    pgoff;
};

struct loop_info {
    std::vector<uint64_t> *ips;
    uint64_t timestamp;
    std::string module_name;
};

#endif /* TYPEDEF_H_ */
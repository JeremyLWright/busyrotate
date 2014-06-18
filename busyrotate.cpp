#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <libgen.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <queue>
#include "log_pattern.hpp"
#include "inotify_fd.hpp"
#include "rotateutils.hpp"
#include <algorithm>




/**
 * Files are ordered by log.XXX.YYY where XXX is the boot count and YYY is the sequence number
 * Files are rotated by incrementing the YYY within a single boot count.
 * The oldest file is then the least XXX and the least YYY
 *
 * busyrotate assumes the system time is corrupt, and starts over at each
 * power cycle.
 *
 */
log_pattern find_oldest_file(std::string dir)
{
    std::priority_queue<log_pattern,
        std::vector<log_pattern>,
        std::greater<log_pattern>> q;

    dirlist(dir, pusher(q));
    auto t = q.top();
    while(t.is_active)
    {
        q.pop();
        t = q.top();
    }
    return t;
}

log_pattern find_greatest_index(std::string dir, size_t boot_count)
{
    std::vector<log_pattern> d;
    dirlist(dir, std::back_inserter(d));
    auto it = std::remove_if(std::begin(d), std::end(d),
            [&](log_pattern const & a){ return a.boot_count != boot_count; });
    d.resize(it - std::begin(d));
        

    return *std::max_element(std::begin(d), std::end(d));
}

void rotate(std::string dir, size_t boot_count)
{
    log_pattern m = find_greatest_index(dir, boot_count);
    std::stringstream ss;
    ss << m.prefix << "." << m.boot_count;
    log_pattern new_name(m);
    new_name.is_active = false;
    ++new_name.sequence_number;
    rename( (dir+"/"+ss.str()).c_str(), (dir+"/"+new_name.name()).c_str());
}

void delete_oldest(std::string dir)
{
    auto f = find_oldest_file(dir);
    remove((dir+"/"+f.name()).c_str());
}

int main(int argc, char const * argv[])
{

    std::string const dir(argv[1]);
    size_t const max_dir_size = std::stoi(argv[2]);
    size_t const max_file_size = std::stoi(argv[3]);


    if(max_dir_size < max_file_size)
        throw std::logic_error("Files size are inconsistant. max_dir must be > max_file");

    size_t const boot_count = std::stoi(argv[4]);
    log_pattern default_log;
    default_log.boot_count = boot_count;
    default_log.prefix = "log";
    std::stringstream ss;
    ss << dir << '/' << default_log.active_name();
    std::string const full_path = ss.str();
    
    std::cout << "Managing " << full_path << " in dir " << dir << " with maxsize " << max_file_size << " and " << max_dir_size << '\n';
    
    while(1) 
    {
        { //Open a new scope for RAII. We want the ifd destructor to close the file descriptors. 
            inotify_fd ifd(full_path, IN_CLOSE);
            ifd.wait();
        }

        if(file_size(full_path) > max_file_size)
        {
            std::cout << "File is too big. Rotating\n";
            rotate(dir, boot_count);
        }
        while(dir_size(dir) > max_dir_size)
        {
            std::cout << "Directory is too big. Rotating\n";
            delete_oldest(dir);
        }
    }
}

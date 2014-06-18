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
#include <unistd.h>



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
        
    if(d.size() == 0)
        throw std::out_of_range("No file available.");
        
    return *std::max_element(std::begin(d), std::end(d));
}

void rotate(std::string dir, size_t boot_count)
{
    try 
    {
        log_pattern m = find_greatest_index(dir, boot_count);
        std::stringstream ss;
        ss << m.prefix << '.' << std::setfill('0') << std::setw(6) << m.boot_count;
        log_pattern new_name(m);
        new_name.is_active = false;
        ++new_name.sequence_number;
        rename( (dir+"/"+ss.str()).c_str(), (dir+"/"+new_name.name()).c_str());
    } 
    catch(std::out_of_range const & e)
    {
        //File not found.
        //Nothing to rotate.
    }
}

void delete_oldest(std::string dir)
{
    auto f = find_oldest_file(dir);
    remove((dir+"/"+f.name()).c_str());
}

void help(std::ostream& o)
{
    o << "Usage: busyrotate [OPTIONS]\n";
    o << "\n";
    o << "System log rotation utility\n";
    o << "\n";
    o << "	-O DIR		Directory to manage. (default:/var/log/messages)\n";
    o << "	-s SIZE		Max size (KB) of file before rotation. (default: 200KB)\n";
    o << "	-d SIZE		Max size (KB) of directory before rotation. (default: 2000KB)\n";
    o << "\t-n NUM      \tNumber to identify this boot count. (required)\n";
    o << "	-f          \tRotate a single file and exit.\n";
    o << "	-g          \tReturn the log file name and exit.\n";
    o << "\n";
    o << "Example\n";
    o << "# Manage the /var/log/messages directory rotate. Rotate files on 10 MB, and keep directory under 256 MB\n";
    o << "busyrotate -O /var/log/messages -b 262144 -s 10240 -n $BOOT_COUNT &\n";
    o << "\n";
    o << "# Rotate a single log with the given boot count\n";
    o << "busyrotate -O /var/log/messages -n $BOOT_COUNT -f \n";
}

int main(int argc, char * const argv[])
{
    std::string dir("/var/log/messages");

    size_t max_dir_size = 2000*1024;
    size_t max_file_size = 200*1024;

    if(max_dir_size < max_file_size)
        throw std::logic_error("Files size are inconsistant. max_dir must be > max_file");

    size_t boot_count = 0;
    bool boot_count_set = false;
    bool single_rotate_mode = false;
    bool output_filename_mode = false;
    int opt;
    const char * opts = "O:s:d:n:fg";
    while((opt = getopt(argc, argv, opts)) != -1)
    {
        switch(opt)
        {
            case 'O':
                dir = std::string(optarg);
                break;
            case 's':
                max_file_size = std::stoi(optarg)*1024;
                break;
            case 'd':
                max_dir_size = std::stoi(optarg)*1024;
            case 'n':
                boot_count = std::stoi(optarg);
                boot_count_set = true;
                break;
            case 'f':
                single_rotate_mode = true;
                break;
            case 'g':
                output_filename_mode = true;
                break;
            default:
                help(std::cerr);
                exit(EXIT_FAILURE);
                break;
        }

    }
    if(!boot_count_set)
    {
        std::cerr << "Invalid configuration.\n";
        help(std::cerr);
        exit(EXIT_FAILURE);
    }


    log_pattern default_log;
    default_log.boot_count = boot_count;
    default_log.prefix = "log";
    std::stringstream ss;
    ss << dir << '/' << default_log.active_name();
    std::string const full_path = ss.str();
    

    if(single_rotate_mode)
    {
        rotate(dir, boot_count);
    }
    else if(output_filename_mode)
    {
        std::cout << full_path << '\n';
    }
    else
    {
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
}

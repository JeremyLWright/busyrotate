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

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

size_t file_size(std::string f)
{
    struct stat sb;
    int exists;
    size_t total_size = 0;
    exists = stat(f.c_str(), &sb);
    if (exists < 0) {
        std::cerr << "Couldn't stat " << f << '\n';
    } else {
        total_size = sb.st_size;
    }
    std::cout << f << " size " << total_size << '\n';
    return total_size;
}

size_t dir_size(std::string dir)
{

    DIR *d;
    struct dirent *de;
    size_t total_size;
    struct dirent entry;
    if(dir == ".." || dir == ".")
        return 0;

    std::cout << "Dir: " << dir << "\n";
    d = opendir(dir.c_str());
    if (d == NULL) {
        perror("prsize");
        exit(1);
    }

    total_size = 0;

    for(readdir_r(d, &entry, &de); de != NULL; readdir_r(d, &entry, &de))
    {
        switch(de->d_type)
        {
            case DT_DIR:
                total_size += dir_size(de->d_name);
                break;
            case DT_REG:
                total_size += file_size(dir + "/" + std::string(de->d_name));
                break;
        }
    }
    closedir(d);
    std::cout << dir << " size " << total_size << '\n';
    return total_size;
}

struct inotify_fd {
    inotify_fd(std::string filename, uint32_t flags):
        fd(-1),
        length(-1),
        wd(-1)
    {
        /*creating the INOTIFY instance*/
        fd = inotify_init();

        /*checking for error*/
        if ( fd < 0 ) {
            perror( "inotify_init" );
        }

        while(wd < 0)
        {
            wd = inotify_add_watch( fd, filename.c_str(), flags);
            sleep(1); //Keep looking for the file. (busybox syslog will recreate it soon)
        }
    }

    ~inotify_fd()
    {
        /*removing the “/tmp” directory from the watch list.*/
        inotify_rm_watch( fd, wd );

        /*closing the INOTIFY instance*/
        close( fd );
    }

    void wait()
    {
        length = -1;
        while(length < 0)
        {
            length = read( fd, buffer, EVENT_BUF_LEN );
        
            /*checking for error*/
            if ( length < 0 ) {
                perror( "read" );
            }  
        }
    }

    int fd;
    int length;
    int wd;
    char buffer[EVENT_BUF_LEN];


};


/**
 * Files are ordered by log.XXX.YYY where XXX is the boot count and YYY is the sequence number
 * Files are rotated by incrementing the YYY within a single boot count.
 * The oldest file is then the least XXX and the least YYY
 *
 * busyrotate assumes the system time is corrupt, and starts over at each
 * power cycle.
 */
std::string find_oldest_file(std::string dir)
{
    std::queue<log_pattern> logs;
    return ""; 
}

void rotate(std::string dir)
{
}

void delete_oldest(std::string dir)
{
}

int main(int argc, char const * argv[])
{
    std::string raw_filename(argv[1]);
    std::string raw_dirname(argv[1]);
    std::string const filename(argv[1]);
    std::string const dir(dirname(const_cast<char*>(argv[1])));
    size_t const maximum_size = std::stoi(argv[2]);
    size_t const max_file_size = std::stoi(argv[3]);


    std::cout << "Managing " << filename << " in dir " << dir << " with maxsize " << maximum_size << '\n';
    
    while(1) 
    {
        {
            inotify_fd ifd(filename, IN_CLOSE);
            ifd.wait();
        }
        std::cout << "Wake up.\n";

        if(file_size(filename) > max_file_size)
        {
            std::cout << "File is too big. Rotating\n";
            rotate(dir);
        }
        if(dir_size(dir) > maximum_size)
        {
            std::cout << "Directory is too big. Rotating\n";
            delete_oldest(dir);
        }
    }
}

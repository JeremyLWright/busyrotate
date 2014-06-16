/*This is the sample program to notify us for the file creation and file deletion takes place in “/tmp” directory*/
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

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


size_t dir_size(std::string directory)
{

    DIR *d;
    struct dirent *de;
    struct stat buf;
    int exists;
    size_t total_size;

    d = opendir(directory.c_str());
    if (d == NULL) {
        perror("prsize");
        exit(1);
    }

    total_size = 0;

    for (de = readdir(d); de != NULL; de = readdir(d)) {
        exists = stat(de->d_name, &buf);
        if (exists < 0) {
            fprintf(stderr, "Couldn't stat %s\n", de->d_name);
        } else {
            total_size += buf.st_size;
        }
    }
    closedir(d);
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
            std::cout << "Directory is too big. Rotating\n";
            rotate(dir);
        }
    }
}

#pragma once
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <libgen.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>


#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )


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

        //busybox syslog opens and closes the log file once per second. This
        //allows one to delete the log file, and syslog will create a new one.
        //This introduces a race condition for us. If we just rotated out the
        //file, syslog hasn't create the new log file yet. We don't want to
        //create the file, since syslog is really responsible for doing so.
        //Instead we wait for the file to be created.
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

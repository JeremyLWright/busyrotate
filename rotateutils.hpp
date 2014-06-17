#pragma once
#include <vector>
#include <string>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <libgen.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

template <typename OutputIterator>
void dirlist(std::string dir, OutputIterator i)
{

    DIR *d;
    struct dirent *de;
    struct dirent entry;
    if(dir == ".." || dir == ".")
        return;

    d = opendir(dir.c_str());
    if (d == NULL) {
        perror("prsize");
        exit(1);
    }


    for(readdir_r(d, &entry, &de); de != NULL; readdir_r(d, &entry, &de))
    {
        switch(de->d_type)
        {
            case DT_DIR:
                break;
            case DT_REG:
                *i = de->d_name;
                break;
        }
    }
    closedir(d);
    return;
}

#include "rotateutils.hpp"
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>
#include <queue>
#include <algorithm>

size_t dir_size(std::string dir)
{

    DIR *d;
    struct dirent *de;
    size_t total_size;
    struct dirent entry;
    if(dir == ".." || dir == ".")
        return 0;

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
    return total_size;
}



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
    return total_size;
}

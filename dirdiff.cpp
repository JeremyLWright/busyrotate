#include <vector>
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
#include <iostream>
#include <string>
#include <queue>
#include <algorithm>


std::vector<std::string> dirlist(std::string dir)
{

    std::vector<std::string> v;
    DIR *d;
    struct dirent *de;
    struct dirent entry;
    if(dir == ".." || dir == ".")
        return v;

    std::cout << "Dir: " << dir << "\n";
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
                v.emplace_back(basename(de->d_name));
                break;
        }
    }
    closedir(d);
    return v;
}


int main(int argc, const char *argv[])
{
    std::cout << "calculate difference of 2 directories\n";

    std::string dir1(argv[1]);
    std::string dir2(argv[2]);

    std::vector<std::string>::iterator it;

    auto d1 = dirlist(dir1);
    std::sort(std::begin(d1), std::end(d1));
    std::cout << "d1\n";
    for(auto& v : d1)
    {
        std::cout << v << '\n';
    }
    auto d2 = dirlist(dir2);
    std::sort(std::begin(d2), std::end(d2));
    std::cout << "d2\n";
    for(auto& v : d2)
    {
        std::cout << v << '\n';
    }
    std::vector<std::string> diff(std::max(d1, d2));

    it = std::set_symmetric_difference(std::begin(d1), std::end(d1),
                std::begin(d2), std::end(d2), std::begin(diff));

    std::cout << "Diff\n";
    for(auto& v : diff)
    {
        std::cout << v << '\n';
    }

    return 0;
}

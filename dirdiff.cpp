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
#include <iterator>

#include "rotateutils.hpp"


int main(int argc, const char *argv[])
{
    std::string dir1(argv[1]);
    std::string dir2(argv[2]);

    std::vector<std::string> d1;
    std::vector<std::string> d2;

    dirlist(dir1, std::back_inserter(d1));
    std::sort(std::begin(d1), std::end(d1));

    dirlist(dir2, std::back_inserter(d2));
    std::sort(std::begin(d2), std::end(d2));

    std::vector<std::string> diff;

    std::set_difference(
            std::begin(d1), 
            std::end(d1),
                std::begin(d2), 
                std::end(d2), 
                std::back_inserter(diff)
                );
    
    for(auto& i : diff)
        std::cout << i << '\n';


    return 0;
}

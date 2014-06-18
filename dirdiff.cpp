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
#include <unistd.h>

#include "rotateutils.hpp"

void help(std::ostream& o)
{
    o << "Usage: dirdiff [OPTIONS]\n";
    o << "\n";
    o << "File name set difference utility.\n";
    o << "\n";
    o << " -a DIR    Directory a. output = set(a) - set(b)\n";
    o << " -b DIR    Directory b. output = set(a) - set(b)\n";
    o << " -p str    Optional String to strip from file names.\n";
}

std::string strip_pattern(std::string& s, std::string& pattern)
{
    size_t i = s.find(pattern);
    if(i != std::string::npos)
    {
        return s.erase(i, pattern.size());
    }
    else
    {
        return s;
    }
}

int main(int argc, char * const argv[])
{
    std::string dir1;
    std::string dir2;
    std::string pattern("");

    std::vector<std::string> d1;
    std::vector<std::string> d2;

    const char * opts = "a:b:p:";
    int opt;
    while((opt = getopt(argc, argv, opts)) != -1)
    {
        switch(opt)
        {
            case 'a':
                dir1 = std::string(optarg);
                break;
            case 'b':
                dir2 = std::string(optarg);
                break;
            case 'p':
                pattern = std::string(optarg);
                break;
            default:
                help(std::cerr);
                exit(EXIT_FAILURE);
                break;
        }
    }

    dirlist(dir1, std::back_inserter(d1));
    std::vector<std::string> a;
    auto stripper = std::bind(strip_pattern, std::placeholders::_1, pattern);

    if(pattern != "")
    {
        std::transform(std::begin(d1), std::end(d1),
                std::back_inserter(a), stripper);
    }
    else
    {
        a = d1;
    }

    dirlist(dir2, std::back_inserter(d2));
    std::vector<std::string> b;
    if(pattern != "")
    {
        std::transform(std::begin(d2), std::end(d2),
                std::back_inserter(b), stripper);

    }
    else
    {
        b = d2;
    }

    std::vector<std::string> diff;

    std::sort(std::begin(a), std::end(a));
    std::sort(std::begin(b), std::end(b));
    std::set_difference(
            std::begin(a), 
            std::end(a),
            std::begin(b), 
            std::end(b), 
            std::back_inserter(diff)
            );

    for(auto& i : diff)
        std::cout << i << '\n';


    return 0;
}

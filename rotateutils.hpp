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
#include <iterator>

size_t dir_size(std::string dir);
size_t file_size(std::string f);

template <typename Container> 
class push_iterator : 
    public std::iterator<std::output_iterator_tag,void,void,void,void>
{
public:
    explicit push_iterator(Container &c) : container(c) {}

    push_iterator &operator*() {return *this;}
    push_iterator &operator++() {return *this;}
    push_iterator &operator++(int) {return *this;}

    push_iterator &operator=(typename Container::const_reference value)
    {
         container.push(value);
         return *this;
    }
private:
    Container &container;
};

template <typename Container> 
push_iterator<Container> pusher( Container & c ) 
{ 
    return push_iterator<Container>( c ); 
}

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
        perror("dir_list");
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

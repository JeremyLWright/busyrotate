#pragma once
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <iomanip>

namespace {

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}
}

struct log_pattern {

    log_pattern():
        prefix(""),
        boot_count(0),
        sequence_number(0),
        is_active(false)
    {
    }
    
    log_pattern(std::string f):
        prefix(""),
        boot_count(0),
        sequence_number(0),
        is_active(false)
    {
        std::vector<std::string> v;
        if(log_pattern::is_log_file(f, v))
        {
            prefix = v[0];
            boot_count = std::stoi(v[1]);
            sequence_number = std::stoi(v[2]);
        }
        else if(v.size() == 2)
        {
            prefix = v[0];
            boot_count = std::stoi(v[1]);
            is_active = true; //This is the active rotation file
        }
        else
        {
            throw std::logic_error("log is not properly formed");
        }
        
    }

    log_pattern(char* f):
        prefix(""),
        boot_count(0),
        sequence_number(0),
        is_active(false)
    {
        std::vector<std::string> v;
        if(log_pattern::is_log_file(f, v))
        {
            prefix = v[0];
            boot_count = std::stoi(v[1]);
            sequence_number = std::stoi(v[2]);
        }
        else if(v.size() == 2)
        {
            prefix = v[0];
            boot_count = std::stoi(v[1]);
            sequence_number = 0;
            is_active = true; //This is the active rotation file
        }
        else
        {
            throw std::logic_error("log is not properly formed");
        }
    }

    static bool is_log_file(std::string f, std::vector<std::string>& v)
    {
        v = split(f, '.');
        if(v.size() != 3)
            return false;
        return true;
    }

    std::string prefix;
    size_t boot_count;
    size_t sequence_number;
    bool is_active;

    std::string name() const
    {
        if(is_active)
            return active_name();
        else
        {
            std::stringstream ss;
            ss << active_name() <<  '.' << std::setfill('0') << std::setw(4) << sequence_number;
            return ss.str();
        }
    }
    std::string active_name() const
    {
        std::stringstream ss;
        ss << prefix << '.' << std::setfill('0') << std::setw(6) << boot_count;
        return ss.str();
    }

};

//Did a come before b?
//a.5 will come before a.7
//a.5.12 will come before a.5.13
inline bool operator<(log_pattern const & a, log_pattern const & b)
{
    if(a.boot_count < b.boot_count)
        return true;
    else if(a.boot_count == b.boot_count)
    {
        if(a.sequence_number < b.sequence_number)
            return true;
        //Always return false for equal values
    }
    return false;
}

inline bool operator==(log_pattern const & a, log_pattern const & b)
{
    return a.prefix == b.prefix && a.boot_count == b.boot_count && a.sequence_number == b.sequence_number;
}

inline bool operator> (const log_pattern& lhs, const log_pattern& rhs){return rhs < lhs;}
inline bool operator<=(const log_pattern& lhs, const log_pattern& rhs){return !(lhs > rhs);}
inline bool operator>=(const log_pattern& lhs, const log_pattern& rhs){return !(lhs < rhs);}

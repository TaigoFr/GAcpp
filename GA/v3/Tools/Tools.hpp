// By Tiago Franca
/*
.hpp for group of useful functions
 - universal printing functions that receive arbitrary number of arguments of arbitrary types
 	print: print all arguments and \n at end
 	printOS: same as print, but for a different ostream other than std::cout
 	printn: print all arguments with \n between all (same for printnOS)
 	printt: print all arguments with \t betwwen all and \n at end (same for printtOS)
 	printno: print all arguments with nothing else
 - errorMsg (print error message, generate exception and exit)
 - file operations
*/
#pragma once

#include <iostream>
#include <string>
#include <ostream>

template < typename ... ARGS >
std::ostream& printOS(std::ostream& os, ARGS && ... args){
    (void)std::initializer_list< int >{ ( os << args, 0 ) ... };
    os << std::endl;
    return os;
}

template < typename ... ARGS >
void printno(ARGS && ... args){
    (void)std::initializer_list< int >{ ( std::cout << args, 0 ) ... };
}

template < typename ... ARGS >
void print(ARGS && ... args){
    printno(args...);
    std::cout << std::endl;
}

template < typename ... ARGS >
std::ostream& printnOS(std::ostream& os, ARGS && ... args){
    (void)std::initializer_list< int >{ ( os << args << std::endl, 0 ) ... };
    return os;
}

template < typename ... ARGS >
void printn(ARGS && ... args){
    (void)std::initializer_list< int >{ ( std::cout << args << std::endl, 0 ) ... };
}

template < typename ... ARGS >
std::ostream& printtOS(std::ostream& os, ARGS && ... args){
    (void)std::initializer_list< int >{ ( os << args << "\t", 0 ) ... };
    os << std::endl;
    return os;
}

template < typename ... ARGS >
void printt(ARGS && ... args){
    (void)std::initializer_list< int >{ ( std::cout << args << "\t", 0 ) ... };
    std::cout << std::endl;
}

template < typename ... ARGS >
void errorMsg(const std::string& msg, ARGS && ... args){
    print(msg,args...);
    throw std::runtime_error(msg.c_str());
    exit(1);
}
void errorMsg(const std::string& msg);

bool fileExists(const std::string& path);
std::string checkFileName(const std::string& name, const std::string& extension);
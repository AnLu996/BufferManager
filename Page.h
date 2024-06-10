#ifndef PAGE_H
#define PAGE_H

#include <iostream>
#include <string>

class Page {
public:
    int id;
    Page(int id) : id(id) {}
    void save() {} 
};

#endif //PAGE_H
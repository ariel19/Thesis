#ifndef ELFADDINGMETHODS_H
#define ELFADDINGMETHODS_H

#include <core/adding_methods/wrappers/daddingmethods.h>

class ELFAddingMethods : public DAddingMethods
{
public:
    ELFAddingMethods(ELF *f);
    ~ELFAddingMethods();
};

#endif // ELFADDINGMETHODS_H

#include <core/adding_methods/wrappers/linux/daddingmethods.h>
#include <core/adding_methods/wrappers/linux/asmcodegenerator.h>

template <typename RegistersType>
bool DAddingMethods::secure_elf(ELF &elf, const InjectDescription<RegistersType> &inject_desc) {
    return true;
}

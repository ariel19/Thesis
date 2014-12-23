#include <core/adding_methods/wrappers/linux/daddingmethods.h>
#include <core/adding_methods/wrappers/linux/asmcodegenerator.h>

template <typename RegistersType>
bool DAddingMethods::secure_elf(ELF &elf, const InjectDescription<RegistersType> &inject_desc) {

    // 1. generate code for handler
    // 2. generate code for debugger detection method
    // 3. merge code
    // 4. compile code
    // 5. add jmp in binary
    // 6. secure elf file

    return true;
}

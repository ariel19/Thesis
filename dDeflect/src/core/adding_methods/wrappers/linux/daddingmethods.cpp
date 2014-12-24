#include <core/adding_methods/wrappers/linux/daddingmethods.h>
#include <core/adding_methods/wrappers/linux/asmcodegenerator.h>

DAddingMethods::DAddingMethods() {
    placeholder_id = {
        { PlaceholderTypes::PARAM_PRE,          "(?^_^"     },
        { PlaceholderTypes::PARAM_POST,         "^_^?)"     },
        { PlaceholderTypes::PLACEHOLDER_PRE,    "(rsj?^_^"  },
        { PlaceholderTypes::PLACEHOLDER_POST,   "^_^?rsj)"  }
    };
}

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

template <typename RegistersType>
bool DAddingMethods::wrapper_gen_code(Wrapper<RegistersType> *wrap, QString &code) {
    if (!wrap)
        return false;

    // check if ret is in used registers (if it is remove)
    if (wrap->used_regs.indexOf(wrap->ret) != -1)
        wrap->used_regs.removeAll(wrap->ret);

    // generate push registers
    code.append(AsmCodeGenerator::push_regs<RegistersType>(wrap->used_regs));

    // fill params
    uint64_t filled_params = fill_params(code, wrap->params);

    // generate pop registers
    code.append(AsmCodeGenerator::pop_regs<RegistersType>(wrap->used_regs));

    return true;
}

uint64_t DAddingMethods::fill_params(QString &code, const QMap<QString, QString> &params) {
    uint64_t cnt = 0;
    foreach (QString param, params) {
        QString plc_param(QString("%1%2%3").arg(placeholder_id[PlaceholderTypes::PARAM_PRE], param,
                                                placeholder_id[PlaceholderTypes::PARAM_POST]));
        // look in string for plc_param cahnge it to value from dictionary
    }
    return cnt;
}

uint64_t DAddingMethods::fill_placeholders(QString &code) {
    uint64_t cnt = 0;
    // TODO: fill placeholders in code
    return cnt;
}

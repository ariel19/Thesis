#ifndef PEADDINGMETHODS_H
#define PEADDINGMETHODS_H

#include <QProcess>

#include <core/adding_methods/wrappers/daddingmethods.h>
#include <core/file_types/pefile.h>

class PEAddingMethods : public DAddingMethods
{
private:
    std::default_random_engine gen;

    template <typename Register, typename T>
    bool generateParametersLoadingCode(PEFile &pe, BinaryCode<Register> &code, T getFunctionsCodeAddr, QMap<Register,
                                       QString> params, QMap<QByteArray, uint64_t> &ptrs, T threadCodePtr);

    template <typename Register>
    bool generateActionConditionCode(PEFile &pe, BinaryCode<Register> &code, uint64_t action, Register cond, Register act);

    template <typename Register>
    bool injectEpCode(PEFile &pe, QList<uint64_t> &epMethods, QMap<QByteArray, uint64_t> &codePointers, QList<uint64_t> &relocations);

    template <typename Register>
    bool injectTlsCode(PEFile &pe, QList<uint64_t> &tlsMethods, QMap<QByteArray, uint64_t> &codePointers, QList<uint64_t> &relocations);

    template <typename Register>
    bool injectTrampolineCode(PEFile &pe, QList<uint64_t> &tramMethods, QMap<QByteArray, uint64_t> &codePointers,
                              QList<uint64_t> &relocations, QByteArray text_section, uint32_t text_section_offset, uint8_t codeCoverage);

    template <typename Register>
    uint64_t generateThreadCode(PEFile &pe, QList<Wrapper<Register>*> wrappers, QMap<QByteArray, uint64_t> &ptrs, uint16_t sleepTime, QList<uint64_t> &relocations);


    template <typename Register>
    uint64_t generateCode(PEFile &pe, Wrapper<Register> *w, QMap<QByteArray, uint64_t> &ptrs, QList<uint64_t> &relocations, bool isTlsCallback = false);

    void getFileOffsetsFromOpcodes(QStringList &opcodes, QList<uint32_t> &fileOffsets, uint32_t baseOffset);

    template <typename Register>
    BinaryCode<Register> generateTrampolineCode(uint64_t realAddr, uint64_t wrapperAddr);

    template <typename Register>
    uint8_t getRandomRegister();

public:
    PEAddingMethods();
    ~PEAddingMethods();

    template <typename Register>
    bool injectCode(PEFile &pe, QList<InjectDescription<Register>*> descs, uint8_t codeCoverage);
};

#endif // PEADDINGMETHODS_H

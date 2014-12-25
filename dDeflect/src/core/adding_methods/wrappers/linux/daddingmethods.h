#ifndef DADDINGMETHODS_H
#define DADDINGMETHODS_H

#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QDebug>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include <algorithm>

#include <core/file_types/elffile.h>

#define CMSIZE   (sizeof(CallingMethodNames)/sizeof(*CallingMethodNames))
/*
 *  Tablice potrzebne do znajdowania nazw enumów
 */
static const char* Registers_x86_names[] = {
    "EAX",
    "EBX",
    "ECX",
    "EDX",
    "ESI",
    "EDI",
    "EBP",
    "ESP"
};

static const char* Registers_x64_names[] = {
    "RAX",
    "RBX",
    "RCX",
    "RDX",
    "RSI",
    "RDI",
    "RBP",
    "RSP",
    "R8",
    "R9",
    "R10",
    "R11",
    "R12",
    "R13",
    "R14",
    "R15"
};
static const char* CallingMethodNames[]{
    "OEP",
    "Thread",
    "Trampoline"
};
class DAddingMethods {

public:
    /**
     * @brief Typy możliwości wstrzyknięcia kodu.
     */
    enum class CallingMethod {
        OEP,
        Thread,
        Trampoline
    };

    /**
     * @brief Rejestry dla architektury x86.
     */
    enum class Registers_x86 {
        EAX,
        EBX,
        ECX,
        EDX,
        ESI,
        EDI,
        EBP,
        ESP
    };


    /**
     * @brief Rejestry dla architektury x64.
     */
    enum class Registers_x64 {
        RAX,
        RBX,
        RCX,
        RDX,
        RSI,
        RDI,
        RBP,
        RSP,
        R8,
        R9,
        R10,
        R11,
        R12,
        R13,
        R14,
        R15
    };

    /**
     * @brief Klasa bazowa reprezentująca opakowanie dla kawałków kodu.
     */
    template <typename RegistersType>
    struct Wrapper {
        /**
         * @brief names zwraca tablice nazw rejestrów
         * @return  jeśli parametr wzorca jest typu Registers_x64 to zwracamy tablice nazw rejestrów x64 bitowych w p.p. rejestrów x86
         */
        const char** names(){
            const bool isX64 = std::is_same<RegistersType, Registers_x64>::value;
            return ( isX64 ) ? Registers_x64_names : Registers_x86_names;
        }
        /**
         * @brief size zwraca ilość rejestrów w architekturze dla której jest napisany wrapper. Konkretnie zwraca rozmiar tablicy Registers_x[86|64]_names
         * @return
         */
        size_t size(){
            const bool isX64 = std::is_same<RegistersType, Registers_x64>::value;
            return ( isX64 ) ? (sizeof(Registers_x64_names)/sizeof(*Registers_x64_names)) : (sizeof(Registers_x86_names)/sizeof(*Registers_x86_names));
        }

        QList<RegistersType> used_regs;
        QMap<QString, QString> params;
        RegistersType ret;
        QString code;
        Wrapper<RegistersType> *ddetec_handler;

        /**
         * @brief read this zostaje wczytany z pliku json
         * @param json objekt json'a z którego czytamy
         */
        void read(const QJsonObject & json){

            // used_regs

            QJsonArray regList = json["used_regs"].toArray();
            for(int i = 0; i< regList.size(); ++i){

                const char* regStr = regList[i].toString().toLocal8Bit().constData();
                RegistersType reg = static_cast<RegistersType>(std::distance(names(), std::find(names(), names() + size(), regStr)));

                used_regs.append(reg);
            }

            // params

            QMap<QString,QVariant> tempMap = json["params"].toObject().toVariantMap();

            foreach(QString key, tempMap.keys()){
                params.insert(key, tempMap.value(key).toString());
            }

            // ret

            const char* retName = json["ret"].toString().toLocal8Bit().constData();
            ret = static_cast<RegistersType>(std::distance(names(), std::find(names(), names() + size(), retName)));

            // code

            code = json["code"].toString();

            // ddetec_handler

            ddetec_handler = nullptr;


        }
        /**
         * @brief write zapisujemy obiekt this do pliku json
         * @param json obiekt do którego zapisujemy
         */
        void write(QJsonObject &json) const{

            // used_regs
            QJsonArray array;

            const char** tab = ( std::is_same<RegistersType, Registers_x64>::value ) ? Registers_x64_names : Registers_x86_names;

            for(int i = 0; i< used_regs.size(); ++i){
                array.append(QJsonValue(QString( tab[static_cast<int>(used_regs[i])])));
            }
            json["used_regs"]=array;

            // params
            QJsonObject par;
            foreach(QString key, params.keys()){
                par.insert(key, QJsonValue(params.value(key)));
            }
            json["params"] = par;

            // ret

            json["ret"] = QString(tab[static_cast<int>(ret)]);

            // code

            json["code"] = code;
        }
    };


    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia nowego wątku.
     */
    template <typename RegistersType>
    class ThreadWrapper : public Wrapper<RegistersType> {
        Wrapper<RegistersType> *thread_action;
    };

    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia nowego punktu wejściowego.
     */
    template <typename RegistersType>
    class OEPWrapper : public Wrapper<RegistersType> {
        Wrapper<RegistersType> *oep_action;
    };


    /**
     * @brief Klasa reprezentująca opakowanie dla tworzenia tramplin w funkcjach bibliotecznych.
     */
    template <typename RegistersType>
    class TrampolineWrapper : public Wrapper<RegistersType> {

    };

    /**
     * @brief Klasa opisująca metodę wstrzykiwania kodu.
     */
    template <typename RegistersType>
    struct InjectDescription {
        CallingMethod cm;
        Wrapper<RegistersType> *adding_method;
    public:
        /**
         * @brief read wczytuje obiekt InjectDescription z obiektu json
         * @param json referencja do obiektu z którego czytamy
         */
        void read(const QJsonObject &json){

           Wrapper<RegistersType> * w = new Wrapper<RegistersType>();

           const char* cmString = json["cm"].toString().toLocal8Bit().constData();
           cm = static_cast<CallingMethod>(std::distance(CallingMethodNames, std::find(CallingMethodNames, CallingMethodNames + CMSIZE,cmString)));

           QJsonValue jv = json["adding_method"].toObject();
           if(jv.isObject()){
                w->read(jv.toObject());
                adding_method = w;
           }
           else{
               delete w;
               w = nullptr;
           }

        }
        /**
         * @brief write zapisujemy obiekt InjectDescription do pliku json
         * @param json referencja do obiektu do którego piszemy
         */
        void write(QJsonObject &json) const{
            QString s(CallingMethodNames[static_cast<int>(cm)]);
            json["cm"] = s;
            QJsonObject o;
            adding_method->write(o);
            json["adding_method"]=o;

        }
    };

    /**
     * @brief Konstruktor.
     */
    DAddingMethods() {}

    /**
     * @brief Metoda zabezpiecza plik, podany jako argument za pomocą wyspecyfikowanej metody.
     * @param elf plik do zabezpieczania.
     * @param inject_desc opis metody wstrzykiwania kodu.
     * @return True, jeżeli operacja się powiodła, False w innych przypadkach.
     */
    template <typename RegistersType>
    bool secure_elf(ELF &elf, const InjectDescription<RegistersType> &inject_desc);
};


//template <typename RegistersType>
//QDebug operator<<(QDebug d, const DAddingMethods::Wrapper<RegistersType> &w)
//{
//     d<<"Code: "<< w.code << " Params:" << w.params <<" w.ret: "<<static_cast<int>(w.ret);//<<" used regs: " << static_cast< QList<int> >(w.used_regs);
//     return d;
//}
#endif // DADDINGMETHODS_H

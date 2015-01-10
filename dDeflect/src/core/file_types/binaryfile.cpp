#include "binaryfile.h"

#include <chrono>

BinaryFile::BinaryFile(QByteArray _data) :
    parsed(false),
    b_data(_data),
    gen(std::chrono::system_clock::now().time_since_epoch().count())
{

}

BinaryFile::~BinaryFile()
{

}

QByteArray BinaryFile::getData()
{
    return b_data;
}


#include "binaryfile.h"

BinaryFile::BinaryFile(QByteArray _data) :
    parsed(false),
    b_data(_data)
{

}

BinaryFile::~BinaryFile()
{

}

QByteArray BinaryFile::getData()
{
    return b_data;
}


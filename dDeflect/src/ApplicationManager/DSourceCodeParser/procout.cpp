#include "procout.h"

ProcOut::ProcOut (QObject *parent /* = NULL */):
QObject(parent)
{}

void
ProcOut::readyRead()
{
//  if (!g_process)
//    return;

//  QTextStream out(stdout);
//  out << g_process->readAllStandardOutput() << endl;
}

void
ProcOut::finished()
{
  QCoreApplication::exit (0);
}

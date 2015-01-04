#ifndef PROCOUT_H
#define PROCOUT_H


#include <QObject>
#include <QProcess>
#include <QTextStream>
#include <QCoreApplication>

extern QProcess *g_process;

class ProcOut : public QObject
{
  Q_OBJECT
public:
  ProcOut (QObject *parent = NULL);
  virtual ~ProcOut() {};

public slots:
  void readyRead();
  void finished();
};


#endif // PROCOUT_H

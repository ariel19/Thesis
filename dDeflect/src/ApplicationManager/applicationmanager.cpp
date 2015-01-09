#include "applicationmanager.h"
#include "DSourceCodeParser/dsourcecodeparser.h"

ApplicationManager::ApplicationManager(QObject *parent) :
    QObject(parent), jsonParser("/home/jsk/code/Thesis/dDeflect/src/core/detection/dsc/"), sourceParser(), m_targetPath("Choose a C++ source file or an executive file.")
{
    // TODO: dodać obsługę listy metod 64 bitowych
    QDir wrappers("/home/jsk/code/Thesis/dDeflect/src/core/detection/dsc/");
    Q_ASSERT(wrappers.exists());
    QDirIterator it("/home/jsk/code/Thesis/dDeflect/src/core/detection/dsc/", QStringList() << "*.json", QDir::Files);

    QFileInfoList files = wrappers.entryInfoList();
    foreach(const QFileInfo &fi, files) {
        QString Path = fi.absoluteFilePath();
        if(fi.completeSuffix()=="json"){
            // TODO
          m_x86methodsList.append(jsonParser.loadInjectDescription<Registers_x86>(fi.fileName()));
        }
    }

    // TODO: Lista do uzupełnienia o wszystkie rozszerzenia, albo stworzyć plik ze stringami i innymi danymi
    sourceExtensionList<<"cpp"<<"cxx"<<"c";
    setState(IDLE);

}

ApplicationManager::~ApplicationManager()
{
    delete sourceParser;
}

void ApplicationManager::setState(ApplicationManager::State state)
{
    m_state = state;
    emit stateChanged(state);
}

ApplicationManager::State ApplicationManager::state() const
{ return m_state; }

QVariantList ApplicationManager::x86MethodsNames()
{

    if(m_x86MethodsNames.empty()){
        foreach(DAddingMethods<Registers_x86>::Wrapper* id, m_x86methodsList){
            if(id==NULL)
                qDebug()<<"Wczytany wrapper == NULL";
            else{
                QVariant* p = new QVariant(QString::fromStdString(id->name.toStdString()));
                m_x86MethodsNames.append(*p);
            }
        }
    }
    return m_x86MethodsNames;
}

void ApplicationManager::fileOpened(QString path)
{
    m_targetPath = path;
    //emit targetPathChanged()
    QFileInfo info(m_targetPath);
    QString extension = info.completeSuffix();

    bool isSource = false;
    foreach(QString ext, sourceExtensionList)
        isSource |= (extension.compare(ext)==0);
    if(isSource)
        setState(SOURCE);
    else
        setState(EXEC);


}

void ApplicationManager::applyClicked(QVariantList methodsChosen)
{
    m_targetPath ="";
    if(methodsChosen.isEmpty()){
        qDebug()<<"Methods List is empty, verify in qml";
        return;
    }

    // TODO: umożliwić wybór funkcji do której chcemy wstrzyknąć
    FIDMapping<Registers_x86> map;
    IDList<Registers_x86> methodsToInsert;
    foreach(QVariant q, methodsChosen){
        int index = q.toInt();
        methodsToInsert.append(m_x86methodsList[index]);
    }
    map["main"] = methodsToInsert;
    qDebug()<<map;
    insertMethods(map);
}

void ApplicationManager::insertMethods(FIDMapping<Registers_x86> Map)
{
    sourceParser->insertMethods(m_targetPath,Map);
}





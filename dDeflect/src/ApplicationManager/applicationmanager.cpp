#include "applicationmanager.h"
#include "DSourceCodeParser/dsourcecodeparser.h"

ApplicationManager::ApplicationManager(QObject *parent) :
    QObject(parent), jsonParser("./injectDescriptions/"), sourceParser(), m_targetPath("Choose a C++ source file or an executive file.")
{
    // TODO: dodać obsługę listy metod 64 bitowych
    QDir descriptionsLocation("./injectDescriptions/");
    Q_ASSERT(descriptionsLocation.exists());
    QDirIterator it("./injectDescriptions/", QStringList() << "*.json", QDir::Files);

    QFileInfoList files = descriptionsLocation.entryInfoList();
    foreach(const QFileInfo &fi, files) {
        QString Path = fi.absoluteFilePath();
        if(fi.completeSuffix()=="json"){
          m_x86methodsList.append(jsonParser.loadInjectDescription(fi.fileName()));
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
        foreach(DAddingMethods::InjectDescription<DAddingMethods::Registers_x86>* id, m_x86methodsList){
            QVariant* p = new QVariant(QString::fromStdString(id->name));
            m_x86MethodsNames.append(*p);
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

    if(methodsChosen.isEmpty()){
        qDebug()<<"Methods List is empty, verify in qml";
        return;
    }

    // TODO: umożliwić wybór funkcji do której chcemy wstrzyknąć
    FIDMapping<DAddingMethods::Registers_x86> map;
    IDList<DAddingMethods::Registers_x86> methodsToInsert;
    foreach(QVariant q, methodsChosen){
        int index = q.toInt();
        methodsToInsert.append(m_x86methodsList[index]);
    }
    map["main"] = methodsToInsert;
    qDebug()<<map;
    insertMethods(map);
    m_targetPath ="";
}

void ApplicationManager::insertMethods(FIDMapping<DAddingMethods::Registers_x86> Map)
{
    sourceParser->insertMethods(m_targetPath,Map);
}



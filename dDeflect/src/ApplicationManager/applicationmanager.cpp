#include "applicationmanager.h"
#include "DSourceCodeParser/dsourcecodeparser.h"

ApplicationManager::ApplicationManager(QObject *parent) :
    QObject(parent), jsonParser("./injectDescriptions/")
{
    // TODO: dodać obsługę listy metod 64 bitowych
    QDir descriptionsLocation("./injectDescriptions/");
    Q_ASSERT(descriptionsLocation.exists());
    QDirIterator it("./injectDescriptions/", QStringList() << "*.json", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        jsonParser.loadInjectDescription(it.fileInfo().fileName());
        m_x86methodsList.append(jsonParser.getInjectionRead());
        it.next();
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

void ApplicationManager::insertMethods(FIDMapping<DAddingMethods::Registers_x86> Map)
{

}



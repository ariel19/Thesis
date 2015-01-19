#include "applicationmanager.h"
#include "DSourceCodeParser/dsourcecodeparser.h"


ApplicationManager::ApplicationManager(QObject *parent) :
    QObject(parent), jsonParser(), sourceParser(), m_targetPath("Choose a C++ source file or an executive file.")
{

    // Metody 32 bitowe
    jsonParser.setPath(DSettings::getSettings().getDescriptionsPath<Registers_x86>());
    qDebug()<<DSettings::getSettings().getDescriptionsPath<Registers_x86>();
    QDir wrappers(DSettings::getSettings().getDescriptionsPath<Registers_x86>());
    Q_ASSERT(wrappers.exists());
    QDirIterator it(DSettings::getSettings().getDescriptionsPath<Registers_x86>(), QStringList() << "*.json", QDir::Files);

    QFileInfoList files = wrappers.entryInfoList();
    foreach(const QFileInfo &fi, files) {
        QString Path = fi.absoluteFilePath();
        if(fi.completeSuffix()=="json"){
          Wrapper<Registers_x86> *w = jsonParser.loadInjectDescription<Registers_x86>(fi.fileName());
          if(w!=NULL){
              m_x86methodsList.append(w);
              Method *m = new Method(w);
              if(w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method)
                m_methodsx86.append(m);
              if(w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler)
                m_handlersx86.append(m);
          }
        }
    }
    // Metody 64 bitowe
    jsonParser.setPath(DSettings::getSettings().getDescriptionsPath<Registers_x64>());
    qDebug()<<DSettings::getSettings().getDescriptionsPath<Registers_x64>();
    QDir wrappers64(DSettings::getSettings().getDescriptionsPath<Registers_x64>());
    Q_ASSERT(wrappers64.exists());
    QDirIterator it64(DSettings::getSettings().getDescriptionsPath<Registers_x64>(), QStringList() << "*.json", QDir::Files);

    QFileInfoList files64 = wrappers64.entryInfoList();
    foreach(const QFileInfo &fi, files64) {
        QString Path = fi.absoluteFilePath();
        if(fi.completeSuffix()=="json"){
            Wrapper<Registers_x64> *w = jsonParser.loadInjectDescription<Registers_x64>(fi.fileName());
            if(w!=NULL){
                m_x64methodsList.append(w);
                Method *m = new Method(w);
                if(w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method)
                  m_methodsx64.append(m);
                if(w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler)
                  m_handlersx64.append(m);
            }
        }
    }
    // TODO: Lista do uzupełnienia o wszystkie rozszerzenia, albo stworzyć plik ze stringami i innymi danymi
    sourceExtensionList<<"cpp"<<"cxx"<<"c";
    setState(IDLE);
    setArchType(X86);

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

void ApplicationManager::setArchType(ApplicationManager::Arch t)
{
    m_archType = t;
    emit archTypeChanged();
}

ApplicationManager::State ApplicationManager::state() const
{ return m_state; }

ApplicationManager::Arch ApplicationManager::archType()
{
    return m_archType;
}

QQmlListProperty<Method> ApplicationManager::x64methods()
{
    return QQmlListProperty<Method>(this,m_methodsx64);
}

QQmlListProperty<Method> ApplicationManager::x86handlers()
{
    return QQmlListProperty<Method>(this,m_handlersx86);
}

QQmlListProperty<Method> ApplicationManager::x64handlers()
{
    return QQmlListProperty<Method>(this,m_handlersx64);
}

QVariantList ApplicationManager::x86MethodsNames()
{

    if(m_x86MethodsNames.empty()){
        foreach(Wrapper<Registers_x86>* id, m_x86methodsList){
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

QQmlListProperty<Method> ApplicationManager::x86methods()
{
    return QQmlListProperty<Method>(this,m_methodsx86);
}


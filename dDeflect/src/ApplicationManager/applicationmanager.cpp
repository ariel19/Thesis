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
    setSys(Linux);

    connect(this,SIGNAL(archTypeChanged()),this,SLOT(updateCurrMethods()));
    connect(this,SIGNAL(currCmChanged()),this,SLOT(updateCurrMethods()));
    connect(this,SIGNAL(sysChanged()),this,SLOT(updateCurrMethods()));

    connect(this,SIGNAL(archTypeChanged()),this,SLOT(updateCurrHandlers()));
    connect(this,SIGNAL(currCmChanged()),this,SLOT(updateCurrHandlers()));
    connect(this,SIGNAL(sysChanged()),this,SLOT(updateCurrHandlers()));

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

void ApplicationManager::setCurrCm(ApplicationManager::CallingMethod cm)
{
    m_currCm = cm;
    emit currCmChanged();
}

void ApplicationManager::setSys(ApplicationManager::System s)
{
    m_sys = s;
    emit sysChanged();
}

ApplicationManager::State ApplicationManager::state() const
{
    return m_state;
}

ApplicationManager::Arch ApplicationManager::archType() const
{
    return m_archType;
}

ApplicationManager::CallingMethod ApplicationManager::currCm() const
{
    return m_currCm;
}

ApplicationManager::System ApplicationManager::sys() const
{
    return m_sys;
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

QQmlListProperty<Method> ApplicationManager::currMethods()
{
    return QQmlListProperty<Method>(this,m_currMethods);
}

QQmlListProperty<Method> ApplicationManager::currHandlers()
{
    return QQmlListProperty<Method>(this,m_currHandlers);
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

void ApplicationManager::updateCurrMethods()
{
    foreach(Method* m, m_currMethods)
        delete m;

    m_currMethods.clear();
    if(m_sys==Linux){
        if(m_archType == X86){
            switch(m_currCm){
            case OEP:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::OEP)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux )
                        m_currMethods.append(new Method(w));
                }
                break;
            case Thread:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::Thread)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case Trampoline:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::Trampoline)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case INIT:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::INIT)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case INIT_ARRAY:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::INIT_ARRAY)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case CTORS:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::CTORS)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case TLS:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::TLS)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            default:
                break;

            }
        }else{
            switch(m_currCm){
            case OEP:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::OEP)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case Thread:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::Thread)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case Trampoline:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::Trampoline)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case INIT:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::INIT)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case INIT_ARRAY:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::INIT_ARRAY)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case CTORS:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::CTORS)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case TLS:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::TLS)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            default:
                break;

            }
        }
    } else {
        if(m_archType == X86){
            switch(m_currCm){
            case OEP:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::OEP)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows )
                        m_currMethods.append(new Method(w));
                }
                break;
            case Thread:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::Thread)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currMethods.append(new Method(w));
                }
                break;
            case Trampoline:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::Trampoline)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currMethods.append(new Method(w));
                }
                break;
            case INIT:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::INIT)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currMethods.append(new Method(w));
                }
                break;
            case INIT_ARRAY:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::INIT_ARRAY)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currMethods.append(new Method(w));
                }
                break;
            case CTORS:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::CTORS)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currMethods.append(new Method(w));
                }
                break;
            case TLS:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::TLS)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currMethods.append(new Method(w));
                }
                break;
            default:
                break;

            }
        }else{
            switch(m_currCm){
            case OEP:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::OEP)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case Thread:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::Thread)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case Trampoline:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::Trampoline)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case INIT:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::INIT)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case INIT_ARRAY:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::INIT_ARRAY)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case CTORS:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::CTORS)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            case TLS:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::TLS)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Method
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currMethods.append(new Method(w));
                }
                break;
            default:
                break;

            }
        }
    }
    emit currMethodsChanged();
}

void ApplicationManager::updateCurrHandlers()
{
    foreach(Method* m, m_currHandlers)
        delete m;

    m_currHandlers.clear();
    if(m_sys==Linux){
        if(m_archType == X86){
            switch(m_currCm){
            case OEP:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::OEP)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux )
                        m_currHandlers.append(new Method(w));
                }
                break;
            case Thread:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::Thread)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case Trampoline:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::Trampoline)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case INIT:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::INIT)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case INIT_ARRAY:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::INIT_ARRAY)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case CTORS:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::CTORS)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case TLS:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::TLS)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            default:
                break;

            }
        }else{
            switch(m_currCm){
            case OEP:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::OEP)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case Thread:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::Thread)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case Trampoline:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::Trampoline)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case INIT:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::INIT)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case INIT_ARRAY:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::INIT_ARRAY)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case CTORS:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::CTORS)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case TLS:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::TLS)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            default:
                break;

            }
        }
    } else {
        if(m_archType == X86){
            switch(m_currCm){
            case OEP:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::OEP)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows )
                        m_currHandlers.append(new Method(w));
                }
                break;
            case Thread:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::Thread)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case Trampoline:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::Trampoline)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case INIT:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::INIT)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case INIT_ARRAY:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::INIT_ARRAY)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case CTORS:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::CTORS)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case TLS:
                foreach(Wrapper<Registers_x86>* w, m_x86methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x86>::CallingMethod::TLS)
                            && w->wrapper_type==Wrapper<Registers_x86>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x86>::SystemType::Windows)
                        m_currHandlers.append(new Method(w));
                }
                break;
            default:
                break;

            }
        }else{
            switch(m_currCm){
            case OEP:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::OEP)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case Thread:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::Thread)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case Trampoline:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::Trampoline)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case INIT:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::INIT)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case INIT_ARRAY:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::INIT_ARRAY)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case CTORS:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::CTORS)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            case TLS:
                foreach(Wrapper<Registers_x64>* w, m_x64methodsList){
                    if(w->allowed_methods.contains(DAddingMethods<Registers_x64>::CallingMethod::TLS)
                            && w->wrapper_type==Wrapper<Registers_x64>::WrapperType::Handler
                            && w->system_type == DAddingMethods<Registers_x64>::SystemType::Linux)
                        m_currHandlers.append(new Method(w));
                }
                break;
            default:
                break;

            }
        }
    }
    emit currHandlersChanged();
}

void ApplicationManager::changeList(const QString &methodsName,const QString& handlersName, int index)
{
    if(archType()==X86){
        Wrapper<Registers_x86> *newWrapper;
        Wrapper<Registers_x86> *newHandler;
        foreach(Wrapper<Registers_x86> *w ,m_x86methodsList){
            if(w->name==methodsName){
                newWrapper = new Wrapper<Registers_x86>(*w);
            }
        }
        foreach(Wrapper<Registers_x86> *w ,m_x86methodsList){
            if(w->name==handlersName){
                newHandler = new Wrapper<Registers_x86>(*w);
            }
        }

        if(newWrapper->ret == Registers_x86::None)
            newHandler = nullptr;

        newWrapper->detect_handler = newHandler;
        if(index<x86methodsToInsert.size() && index>=0){
            x86methodsToInsert[index]->adding_method = newWrapper;
        }
        else
            qDebug()<<"index out of bound";
        return;
    }
}

void ApplicationManager::insertNewToList(const QString &name)
{
    if(archType()==X86){
        DAddingMethods<Registers_x86>::InjectDescription *id = new DAddingMethods<Registers_x86>::InjectDescription();
        id->cm = (DAddingMethods<Registers_x86>::CallingMethod)currCm();
        id->change_x_only = false;

        Wrapper<Registers_x86> * w = new Wrapper<Registers_x86>();
        id->adding_method = w;

        x86methodsToInsert.append(id);
        //changeList("Code checksum","Exit",methodsToInsert.size()-1);
    }
}

void ApplicationManager::clearList()
{

}

QQmlListProperty<Method> ApplicationManager::x86methods()
{
    return QQmlListProperty<Method>(this,m_methodsx86);
}


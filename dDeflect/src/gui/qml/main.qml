import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import jsk.components 1.0

ApplicationWindow {
    id:root
    visible: true
    width: 640
    height: 480
    title: qsTr("dDeflect - debugger detection")
    color: "white" //"#5989a6"
    property int appState: Manager.IDLE

    menuBar: MenuBar {
        id: mb
        Menu {
            title: "&File"
            MenuItem {
                text: "Load"
                action: openAction
            }
            MenuItem {
                text: "Close"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: "&Help"
            MenuItem { action: aboutAction }
        }

    }
    SystemPalette { id: sysPal }
    AboutDialog { id: aboutDialog }
    FileDialog {
        id: fileDialog
        nameFilters: [ "C++ files (*.cc *.cpp *.CPP *.c++ *.cp *.cxx)", "Executive files (*.exe)", "All files (*)" ]
        onFileUrlChanged:{
            fileUrlText.text = fileUrl;
            applicationManager.fileOpened(fileDialog.fileUrl);
        }

    }

    toolBar: ToolBar {
        id: toolbar

        style: ToolBarStyle {
            padding {
                left: 8
                right: 8
                top: 3
                bottom: 3
            }

            background: Rectangle {
                implicitWidth: 100
                implicitHeight: 40
                border.color: "#999"
                gradient: Gradient {
                    GradientStop { position: 0 ; color: "#fff" }
                    GradientStop { position: 1 ; color: "#bbcedb" }
                }
            }
        }
        RowLayout {
            id: toolbarLayout
            spacing: 0
            width: parent.width

            ToolButton { id: ob ; action: openAction }
            TextField{
                id: fileUrlText
                readOnly: true
                anchors.fill: parent
                anchors.leftMargin: ob.width*1.5
                height: parent.height
                text: "Choose a C++ source file or an executive file."
            }
        }
    }

    Action {
        id: aboutAction
        text: "About"
        onTriggered: aboutDialog.open()
    }
    Action {
        id: openAction
        text: "&Open"
        shortcut: StandardKey.Open
        iconSource: "../images/document-open.png"
        onTriggered: {
            fileDialog.open();

        }
        tooltip: "Open a file to protect"
    }

    TabView {
        id:frame
        style: touchStyle
        anchors.fill: parent
        //anchors.margins: Qt.platform.os === "osx" ? 12 : 2
        Text{
            //anchors.fill: parent
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            text: appState===Manager.IDLE ? fileUrlText.text :""

        }

    }
    Component {
        id: touchStyle
        TabViewStyle {
            tabsAlignment: Qt.AlignVCenter
            tabOverlap: 0
            frame: Item { }
            tab: Item {
                implicitWidth: control.width/control.count
                implicitHeight: 50
                BorderImage {
                    anchors.fill: parent
                    border.bottom: 8
                    border.top: 8
                    source: styleData.selected ? "../images/tab_selected.png":"../images/tabs_standard.png"
                    Text {
                        anchors.centerIn: parent
                        color: "white"
                        text: styleData.title.toUpperCase()
                        font.pixelSize: 16
                    }
                }
            }
        }
    }

    MethodList{
        id: sourceCodeMethods
        path: "/home/jsk/code/Thesis/build-dDeflect-Desktop_Qt_5_3_GCC_64bit-Debug/qtc_Desktop_Qt_5_3_GCC_64bit-debug/install-root/bin/methods.json"
    }
    property var t:[]
    Connections{
     target: applicationManager
     onStateChanged: {

         // FIXME: przy zmianie stanów EXEC==>SOURCE się wychrzania
         console.log("state changed: "+ applicationManager.state);
         if(appState !== Manager.IDLE && applicationManager.state!== Manager.IDLE){
             for(var i=0; i<3; ++i){
                  execTab.visible = false;
                  obfuscationTab.visible = false;
                  packingTab.visible = false
             }
             for(var j=0; j<3; ++j){
                 frame.removeTab(0);
             }
         }
         if(applicationManager.state === 2){
             if(appState !== 2){
                console.log("SOURCE STATE")
                frame.addTab("SOURCE",sourceTab)
                frame.addTab("PACKING",packingTab)
                frame.addTab("OBFUSCATION",obfuscationTab)
                for(var k=0; k<3; ++k){
                     frame.getTab(k).visible = true;
                }
             }
         } else if(applicationManager.state === 1){
            if(appState !== 1){
                frame.addTab("EXECUTABLE",execTab)
                frame.addTab("PACKING",packingTab)
                frame.addTab("OBFUSCATION",obfuscationTab)
                for(var p=0; p<3; ++p){
                     frame.getTab(p).visible = true;
                }
            }
         } else {
             if(appState!== Manager.IDLE){
                 for(var z=0; z<3; ++z){
                      frame.getTab(z).visible = false;
                 }
                 frame.removeTab(0);
                 frame.removeTab(0);
                 frame.removeTab(0);
             }
         }
         appState = applicationManager.state;
     }
    }

    Component{
        id: execTab

        Tab {
            title: "Executable
"
            visible:true
            RowLayout{
                anchors.fill: parent
                anchors.margins: 12

                TableView {
                    id: methodsChoice
                    anchors.fill: parent
                    anchors.rightMargin: frame.width/2
                    model: sourceCodeMethods.names

                    TableViewColumn {
                        role: "title"
                        title: "Method's Name"
                        //width: 120
                    }


                }
                ColumnLayout{
                    anchors.fill: parent
                    anchors.leftMargin: frame.width/2
                    TextArea{
                        id: methodsDescription
                        anchors.fill: parent
                        anchors.bottomMargin: 40
                        readOnly: true
                        text:{ return methodsChoice===-1?"":sourceCodeMethods.methods[methodsChoice.currentRow].description }
                    }
                    Button{
                        height:50
                        text: "Apply Method"
                        anchors.bottom: parent.bottom

                    }
                }

            }
        }
    }
    Component{
        id: obfuscationTab
        Tab {
            title: "Obfuscation"
            visible: true
            RowLayout{
                anchors.fill: parent
                anchors.margins: 12

                TableView {
                    anchors.fill: parent
                    anchors.rightMargin: frame.width/2
                    model: ["method1", "method2"]
                    TableViewColumn {
                        role: "title"
                        title: "Method's Name"
                        //width: 120
                    }

                }
                ColumnLayout{
                    anchors.fill: parent
                    anchors.leftMargin: frame.width/2
                    TextArea{

                        anchors.fill: parent
                        anchors.bottomMargin: 40
                        text: "The only method:"
                    }
                    Button{
                        height:50
                        text: "Apply Method"
                        anchors.bottom: parent.bottom
                    }
                }
            }
        }
    }
    Component{
        id:packingTab
        Tab {
            title: "Packing"
            visible:true
            RowLayout{
                anchors.fill: parent
                anchors.margins: 12

                TableView {
                    anchors.fill: parent
                    anchors.rightMargin: frame.width/2
                    model: ["method1", "method2"]
                    TableViewColumn {
                        role: "title"
                        title: "Method's Name"
                        //width: 120
                    }

                }
                ColumnLayout{
                    anchors.fill: parent
                    anchors.leftMargin: frame.width/2
                    TextArea{

                        anchors.fill: parent
                        anchors.bottomMargin: 40
                        text: "The only method:"
                    }
                    Button{
                        height:50
                        text: "Apply Method"
                        anchors.bottom: parent.bottom

                    }
                }

            }
        }
    }
    Component {
        id:sourceTab

        Tab{
            title: "Source"
            visible:true
            RowLayout{
                anchors.fill: parent
                anchors.margins: 12

                Component {
                    id: contactDelegate
                    Item {
                        width: 180; height: 20
                        Row {
                            anchors.centerIn: parent.Center
                            CheckBox{
                                onCheckedChanged: {
                                    if(checked===true){
                                        if(t.indexOf(index)===-1){
                                            t.push(index);
                                            console.log(t);
                                        }
                                    }else{
                                        if(t.indexOf(index)!==-1){
                                            var i = t.indexOf(index);
                                            t.splice(i,1);
                                            console.log(t);
                                        }
                                    }
                                }
                            }
                            Text{text: '<b>Name:</b> ' + modelData }
                        }
                    }
                }
                ListView {
                    id: lv
                    anchors.fill: parent
                    anchors.rightMargin: frame.width/2
                    model: applicationManager.x86MethodsNames
                    delegate: contactDelegate

                }
                ColumnLayout{
                    anchors.fill: parent
                    anchors.leftMargin: frame.width/2
                    TextArea{

                        anchors.fill: parent
                        anchors.bottomMargin: 40
                        text: "The only method:"
                    }
                    Button{
                        height:50
                        text: "Apply Method"
                        anchors.bottom: parent.bottom
                        onClicked: {
                            applicationManager.applyClicked(t)
                            applicationManager.state = Manager.IDLE
                            appState = Manager.IDLE
                            fileUrlText.text = "Choose a C++ source file or an executive file."
                        }
                    }
                }
            }
        }
    }
}


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
                id: loaditem
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
        nameFilters: [ "C++ files (*.cc *.cpp *.CPP *.c++ *.cp *.cxx)", "Executive files (*.exe)","All files (*)"]
        onFileUrlChanged:{
            fileUrlText.text = fileUrl;
            applicationManager.fileOpened(fileDialog.fileUrl);
            openAction.enabled = false
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
            spacing: 10
            width: parent.width

            ToolButton { id: ob ; action: openAction }
            TextField{
                id: fileUrlText
                readOnly: true
                anchors.fill: parent
                anchors.leftMargin: ob.width*1.5
                anchors.rightMargin: 150
                height: parent.height
                text: "Choose a C++ source file or an executive file."
            }
            ExclusiveGroup { id: tabPositionGroup }
            RadioButton {
                text: "x86"
                checked: true
                anchors.right: parent.right
                anchors.rightMargin: 50
                exclusiveGroup: tabPositionGroup
                onCheckedChanged:
                {
                    if(checked===true)
                        applicationManager.archType=0//applicationManager.X86;
                }
            }
            RadioButton {
                text: "x64"
                anchors.right: parent.right
                exclusiveGroup: tabPositionGroup
                onCheckedChanged:
                {
                    if(checked===true)
                        applicationManager.archType=1;
                }
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
            text: applicationManager.state===Manager.IDLE ? fileUrlText.text :""

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
         console.log("state changed: "+ applicationManager.state);
         while(frame.count>0)
             frame.removeTab(0);
         switch(applicationManager.state) {
             case 0:
                 break;
             case 1:
                 frame.addTab("EXECUTABLE",execTab)
                 frame.addTab("PACKING",packingTab)
                 frame.addTab("OBFUSCATION",obfuscationTab)
                 break;
             case 2:
                 frame.addTab("EXECUTABLE",execTab)
                 frame.addTab("PACKING",packingTab)
                 frame.addTab("OBFUSCATION",obfuscationTab)
                 break;
             case 3:
                 frame.addTab("SOURCE",sourceTab)
                 frame.addTab("PACKING",packingTab)
                 frame.addTab("OBFUSCATION",obfuscationTab)
                 break;
             default:
                 break;
         }
//         if(applicationManager.state === 3 ){
//             if(appState !== 2){
//                 if(appState===2 || appState=== 1){
//                     frame.removeTab(0);
//                     frame.removeTab(0);
//                     frame.removeTab(0);
//                 }
//                console.log("SOURCE STATE")
//                frame.addTab("SOURCE",sourceTab)
//                frame.addTab("PACKING",packingTab)
//                frame.addTab("OBFUSCATION",obfuscationTab)
//                appState = applicationManager.state;
//             }
//         } else if(applicationManager.state === 1){
//            if(appState !== 1){
//                if(appState===2 || appState=== 1){
//                    frame.removeTab(0);
//                    frame.removeTab(0);
//                    frame.removeTab(0);
//                }
//                frame.addTab("EXECUTABLE",execTab)
//                frame.addTab("PACKING",packingTab)
//                frame.addTab("OBFUSCATION",obfuscationTab)
//                appState = applicationManager.state
//            }
//         } else {
//             if(appState!== Manager.IDLE){
//                 frame.removeTab(0);
//                 frame.removeTab(0);
//                 frame.removeTab(0);
//                 appState = applicationManager.state;
//             }
//         }
     }
    }
    ExecutableTab{
        id: execTab
    }
    SourceCodeTab{
        id: sourceTab
    }

    ObfuscationTab{
        id: obfuscationTab
    }
    PackingTab{
       id:packingTab
    }


}


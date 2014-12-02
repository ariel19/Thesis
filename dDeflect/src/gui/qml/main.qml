import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2


ApplicationWindow {        

    visible: true
    width: 640
    height: 480
    title: qsTr("dDeflect - debugger detection")
    color: "white" //"#5989a6"
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
        nameFilters: [ "C++ files (*.cc *.cpp *.CPP *.c++ *.cp *.cxx)", "Executive files (*.exe)" ]
        onFileUrlChanged:{
            fileUrlText.text = fileUrl;
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
        onTriggered: fileDialog.open()
        tooltip: "Open an image"
    }

    TabView {
        id:frame
        style: touchStyle
        anchors.fill: parent
        //anchors.margins: Qt.platform.os === "osx" ? 12 : 2

        Tab {
            title: "FT(validating)"
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
        Tab {
            title: "inv. method"
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
        Tab {
            title: "Obfuscation"
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
        Tab {
            title: "Packing"
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

}


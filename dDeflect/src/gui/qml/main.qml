import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1

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

                height: parent.height
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
        }
        Tab {
            title: "inv. method"

        }
        Tab {
            title: "Obfuscation"

        }
        Tab {
            title: "Packing"

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


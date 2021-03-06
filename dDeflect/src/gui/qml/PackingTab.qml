import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import jsk.components 1.0

Component{
    id:packingTab
    Tab {
        id: tab
        title: "Packing"
        visible: true
        ColumnLayout{
            anchors.fill: parent

            Text{
                text:"Compression Level"
                Layout.alignment: Qt.AlignCenter
            }

            Slider {
                id: coverage
                minimumValue: 0
                maximumValue: 1
                value: 0.1
                Layout.fillWidth: true
                tickmarksEnabled: true
                stepSize: 0.1

            }
            Text{
                text:"Compression Tuning"
                Layout.alignment: Qt.AlignCenter
            }
            Slider {
                id: slider2
                minimumValue: 0.5
                Layout.fillWidth: true
                tickmarksEnabled: true
                stepSize: 0.25


            }

            Button{
                Layout.alignment: Qt.AlignCenter
                width: 200
                height: 50
                text: "PACK!"
                onClicked: {
                    applicationManager.packClicked(coverage.value*10, slider2.value*4-2)
                    tab.enabled = false
                }
            }
        }
    }
}

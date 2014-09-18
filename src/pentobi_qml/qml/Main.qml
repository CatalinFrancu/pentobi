//-----------------------------------------------------------------------------
/** @file pentobi_qml/qml/Main.qml
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Window 2.0
import Qt.labs.settings 1.0
import pentobi 1.0
import "." as Pentobi
import "Main.js" as Logic

ApplicationWindow {
    id: root

    property bool computerPlays0
    property bool computerPlays1
    property bool computerPlays2
    property bool computerPlays3

    contentItem { implicitWidth: 240; implicitHeight: 320 }
    visible: true
    title: qsTr("Pentobi")
    menuBar: Pentobi.Menu { }
    onClosing: Logic.quit()
    Component.onCompleted: {
        Logic.initGameVariant(boardModel.gameVariant)
        x = Screen.width / 2 - width / 2
        y = Screen.height / 2 - height / 2
    }

    Settings {
        property alias level: playerModel.level
    }

    BoardModel {
        id: boardModel
    }

    PlayerModel {
        id: playerModel

        onComputerPlayed: Logic.onComputerPlayed()
    }

    GameDisplay {
        id: gameDisplay

        focus: true
        anchors.fill: contentItem
        onPlay: Logic.play(pieceModel, gameCoord)
    }

    Pentobi.Message {
        id: message

        fontSize: 0.4 * gameDisplay.pieceAreaSize
        x: (root.width - width) / 2
        y: gameDisplay.y + gameDisplay.pieceSelectorY + (gameDisplay.pieceSelectorHeight - height) / 2
        z: 1
    }

    BusyIndicator {
        id: busyIndicator

        running: false
        height: message.height
        anchors.centerIn: message
        z: 2
    }

    ComputerColorDialog {
        id: computerColorDialog

        gameVariant: boardModel.gameVariant
        anchors.centerIn: contentItem
        z: 3
        onOkClicked: {
            root.computerPlays0 = this.computerPlays0
            root.computerPlays1 = this.computerPlays1
            root.computerPlays2 = this.computerPlays2
            root.computerPlays3 = this.computerPlays3
            Logic.checkComputerMove()
        }
    }

    // Used to delay calls to Logic.checkComputerMove such that if the
    // computer playes several moves in a row, it only starts thinking on the
    // next move when the current move placement animation has finished
    Timer {
        id: checkComputerMoveTimer

        interval: 700
        onTriggered: Logic.checkComputerMove()
    }
}

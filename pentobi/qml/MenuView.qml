//-----------------------------------------------------------------------------
/** @file pentobi/qml/MenuView.qml
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

import QtQuick.Controls
import QtQuick.Window

PentobiMenu {
    title: qsTr("View")

    Action {
        text: qsTr("Appearance")
        onTriggered: appearanceDialog.open()
    }
    PentobiMenuItem {
        visible: isDesktop
        // Invisible menu item still use space in Qt 5.11
        height: visible ? implicitHeight : 0
        text: qsTr("Toolbar")
        checkable: true
        checked: rootWindow.showToolBar
        onTriggered: rootWindow.showToolBar = checked
    }
    PentobiMenuItem {
        action: actionComment
    }
    PentobiMenuItem {
        action: actionFullscreen
    }
}

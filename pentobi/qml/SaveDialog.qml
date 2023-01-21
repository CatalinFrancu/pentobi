//-----------------------------------------------------------------------------
/** @file pentobi/qml/SaveDialog.qml
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

import QtQuick
import "Main.js" as Logic

PentobiFileDialog {
    title: qsTr("Save")
    selectExisting: false
    nameFilterLabels: [ qsTr("Blokus games") ]
    nameFilters: [ [ "*.blksgf" ] ]
    folder: rootWindow.folder
    onAccepted: {
        rootWindow.folder = folder
        Logic.saveFile(Logic.getFileFromUrl(fileUrl), "")
    }
}

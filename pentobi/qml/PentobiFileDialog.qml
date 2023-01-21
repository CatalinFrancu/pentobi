//-----------------------------------------------------------------------------
/** @file pentobi/qml/PentobiFileDialog.qml
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.folderlistmodel
import Qt.labs.platform
import "Main.js" as Logic

PentobiDialog {
    id: root

    property bool selectExisting: true
    property alias name: nameField.text
    property url folder
    property url fileUrl
    property var nameFilterLabels
    property var nameFilters
    readonly property url defaultFolder:
        StandardPaths.writableLocation(StandardPaths.HomeLocation)

    function returnPressed() {
        if (! hasButtonFocus())
            checkAccept()
    }
    function selectNameField() {
        var pos = name.lastIndexOf(".")
        if (pos < 0)
            nameField.selectAll()
        else
            nameField.select(0, pos)
        view.currentIndex = -1
    }

    function selectNameFilter(index) {
        comboBoxNameFilter.currentIndex = index
    }

    signal nameFilterChanged(int index)

    property url _lastFolder

    function isValidName(name) {
        return name.trim().length > 0
                && ! (! selectExisting && name.trim().startsWith("."))
    }

    function checkAccept() {
        if (! isValidName(name))
            return
        folder = folderModel.folder
        fileUrl = folder + "/" + name.trim()
        if (! selectExisting
                && gameModel.checkFileExists(Logic.getFileFromUrl(fileUrl))) {
            Logic.showQuestion(qsTr("Overwrite file?"), accept)
            return
        }
        accept()
    }

    footer: PentobiDialogButtonBox {
        Button {
            enabled: isValidName(name)
            text: selectExisting ? qsTr("Open") : qsTr("Save")
            onClicked: checkAccept()
        }
        ButtonCancel { }
    }
    onOpened: selectNameField()

    Item {
        implicitWidth: Math.max(Math.min(font.pixelSize * 30, maxContentWidth),
                                minContentWidth)
        implicitHeight: Math.min(font.pixelSize * 30, maxContentHeight)

        Shortcut {
            sequence: "Alt+Left"
            onActivated: backButton.onClicked()
        }
        ColumnLayout
        {
            anchors.fill: parent

            TextField {
                id: nameField

                visible: ! selectExisting
                focus: true
                selectByMouse: true
                Layout.fillWidth: true
                Component.onCompleted: nameField.cursorPosition = nameField.length
                onTextEdited: view.currentIndex = -1
            }
            RowLayout {
                Layout.fillWidth: true

                ToolButton {
                    id: backButton

                    property bool hasParent:
                        ! folderModel.folder.toString().endsWith(":///")

                    opacity: hasParent ? 1 : 0.5
                    onClicked:
                        if (hasParent) {
                            _lastFolder = folderModel.folder
                            folderModel.folder = folderModel.parentFolder
                            if (selectExisting)
                                name = ""
                        }
                    icon {
                        name: "filedialog-parent"
                        width: font.pixelSize < 20 ? 16 : font.pixelSize
                        height: font.pixelSize < 20 ? 16 : font.pixelSize
                        color: frame.palette.buttonText
                    }
                }
                Label {
                    text: Logic.getFileFromUrl(folderModel.folder)
                    elide: Text.ElideLeft
                    Layout.fillWidth: true
                }
                ToolButton {
                    visible: ! selectExisting
                    icon {
                        name: "filedialog-newfolder"
                        width: font.pixelSize < 20 ? 16 : font.pixelSize
                        height: font.pixelSize < 20 ? 16 : font.pixelSize
                        color: frame.palette.buttonText
                    }
                    onClicked: {
                        var dialog = newFolderDialog.get()
                        dialog.folder = folderModel.folder
                        dialog.open()
                    }
                }
            }
            Frame {
                id: frame

                padding: 0.1 * font.pixelSize
                focusPolicy: Qt.TabFocus
                Layout.fillWidth: true
                Layout.fillHeight: true
                background: Rectangle {
                    color: frame.palette.base
                    border.color: frame.activeFocus ? frame.palette.highlight : frame.palette.mid
                    radius: 2
                }
                ListView {
                    id: view

                    anchors.fill: parent
                    clip: true
                    model: folderModel
                    boundsBehavior: Flickable.StopAtBounds
                    highlight: Rectangle {
                        // Should logically use palette.highlight, but in
                        // most styles other than the desktop style Fusion,
                        // palette.highlight is a too flashy color.
                        color: isDesktop ? palette.highlight : palette.midlight
                    }
                    highlightMoveDuration: 0
                    focus: true
                    onActiveFocusChanged:
                        if (activeFocus && currentIndex < 0 && count)
                            currentIndex = 0
                    onCurrentIndexChanged:
                        if (currentIndex >= 0
                                && ! folderModel.isFolder(currentIndex))
                            name = folderModel.get(currentIndex, "fileName")
                    delegate: ItemDelegate {
                        width: view.width
                        height: 2 * font.pixelSize
                        focusPolicy: Qt.NoFocus
                        highlighted: ListView.isCurrentItem
                        contentItem: Row {
                            spacing: 0.3 * font.pixelSize
                            leftPadding: 0.2 * font.pixelSize

                            Image {
                                anchors.verticalCenter: parent.verticalCenter
                                visible: folderModel.isFolder(index)
                                width: font.pixelSize < 20 ? 16 : font.pixelSize
                                height: width
                                sourceSize { width: 64; height: 64 }
                                source: "icons/filedialog-folder.png"
                                sourceSize { width: width; height: height }
                            }
                            Label {
                                width: parent.width - parent.spacing - parent.leftPadding
                                text: index < 0 ? "" : fileName
                                anchors.verticalCenter: parent.verticalCenter
                                color: highlighted ?
                                           // See comment at highlight
                                           (isDesktop ? frame.palette.highlightedText
                                                      : frame.palette.buttonText) :
                                           frame.palette.text
                                horizontalAlignment: Text.AlignHLeft
                                verticalAlignment: Text.AlignVCenter
                                elide: Text.ElideMiddle
                            }
                        }
                        onClicked: {
                            if (folderModel.isFolder(index)) {
                                delayedOpenFolderTimer.folderName = fileName
                                delayedOpenFolderTimer.restart()
                            }
                            else {
                                name = fileName
                                if (! selectExisting)
                                    selectNameField()
                            }
                            view.currentIndex = index
                        }
                        onDoubleClicked:
                            if (! folderModel.isFolder(index))
                                checkAccept()
                    }
                    ScrollBar.vertical: ScrollBar { }

                    FolderListModel {
                        id: folderModel

                        folder: root.folder
                        nameFilters: [ root.nameFilter ]
                        showDirsFirst: true
                        onStatusChanged:
                            if (status === FolderListModel.Ready) {
                                var i = folderModel.indexOf(_lastFolder)
                                if (i >= 0)
                                    view.currentIndex = i
                                else
                                    view.currentIndex = -1
                            }
                    }
                    // Open folder with small delay such that the folder name
                    // is visibly highlighted when clicked before it opens. We
                    // can't set view.currentIndex in onPressed, otherwise the
                    // item is unwantedly highlighted when flicking the list.
                    Timer {
                        id: delayedOpenFolderTimer

                        property string folderName

                        interval: 100

                        onTriggered: {
                            if (! folderModel.folder.toString().endsWith("/"))
                                folderModel.folder = folderModel.folder + "/"
                            _lastFolder = ""
                            folderModel.folder = folderModel.folder + folderName
                            if (selectExisting)
                                name = ""
                        }
                    }
                }
            }
            PentobiComboBox {
                id: comboBoxNameFilter

                model: {
                    var result = nameFilterLabels
                    nameFilterLabels.push(qsTr("All files"))
                    return result
                }
                onCurrentIndexChanged: {
                    if (currentIndex < root.nameFilters.length)
                        folderModel.nameFilters = root.nameFilters[currentIndex]
                    else
                        folderModel.nameFilters = [ "*" ]
                    nameFilterChanged(currentIndex)
                }
                Layout.preferredWidth:
                    Math.min(font.pixelSize * 14, maxContentWidth)
                Layout.alignment: Qt.AlignRight
            }
        }
    }
}

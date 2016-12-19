import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0
import "Main.js" as Logic

RowLayout {
    id: root

    property alias title: title.text

    function popupMenu() { menu.popup() }

    spacing: 0

    Label {
        id: title

        Layout.fillWidth: true
        Layout.leftMargin: root.height / 10
        color: theme.androidToolBarTextColor
        elide: Text.ElideRight
    }
    AndroidToolButton {
        imageSource: "icons/pentobi-newgame.svg"
        visible: ! gameModel.isGameEmpty
        onClicked: Logic.newGame()
    }
    AndroidToolButton {
        visible: gameModel.canUndo
        imageSource: "icons/pentobi-undo.svg"
        onClicked: Logic.undo()
    }
    AndroidToolButton {
        imageSource: "icons/pentobi-computer-colors.svg"
        onClicked: Logic.showComputerColorDialog()
    }
    AndroidToolButton {
        visible: ! gameModel.isGameOver
        imageSource: "icons/pentobi-play.svg"
        onClicked: Logic.computerPlay()
    }
    AndroidToolButton {
        imageSource: isAndroid ? "icons/menu.svg" : ""
        menu: menu
    }
    Menu {
        id: menu

        MenuGame { }
        MenuGo { }
        MenuEdit { }
        MenuView { }
        MenuComputer { }
        MenuTools { }
        MenuHelp { }
    }
}

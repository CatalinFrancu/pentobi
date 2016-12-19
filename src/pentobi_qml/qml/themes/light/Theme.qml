import QtQuick 2.0
import QtQuick.Controls.Styles 1.4

QtObject {
    property color androidToolBarTextColor: "black"
    property color backgroundColor: "#E6E5E5"
    property color fontColorScore: "#5A5755"
    property color fontColorCoordinates: "#64615E"
    property color fontColorPosInfo: "#282625"
    property color colorBlue: "#0077D2"
    property color colorYellow: "#EBCD23"
    property color colorRed: "#E63E2C"
    property color colorGreen: "#00C000"
    property color colorStartingPoint: "#767074"
    property color backgroundButtonPressed: Qt.lighter(backgroundColor)
    property color selectionColor: "steelblue"
    property color selectedTextColor: "#EEE"
    property color analyzeBackgroundColor: "white"
    property color analyzeLineColor: "black"
    property color analyzeMiddleLineColor: "grey"
    property real pieceListOpacity: 1
    property real toPlayColorLighter: 0.5

    function getImage(name) { return "themes/light/" + name + ".svg" }
}

//-----------------------------------------------------------------------------
/** @file pentobi/qml/PieceList.qml
    @author Markus Enzenberger
    @copyright GNU General Public License version 3 or later */
//-----------------------------------------------------------------------------

import QtQuick 2.0

Grid {
    id: root

    property var pieces

    signal piecePicked(var piece)

    // We want to show the unplayed pieces in slightly less bright colors
    // than the pieces on the board, but not if colorBackground is light
    // otherwise the contrast to the yellow pieces is not strong enough.
    opacity: 0.94 + 0.06 * theme.colorBackground.hslLightness

    Repeater {
        model: pieces

        MouseArea {
            id: mouseArea

            width: root.width / columns; height: width
            visible: ! modelData.pieceModel.isPlayed
            onClicked: {
                gameDisplay.dropCommentFocus()
                piecePicked(modelData)
            }
            hoverEnabled: isDesktop
            scale: isDesktop && containsMouse
                   && ! playerModel.isGenMoveRunning && ! gameModel.isGameOver
                   && (modelData.pieceModel.color === gameModel.toPlay || setupMode)
                   ? 1.2 : 1.0
            Component.onCompleted: modelData.parentUnplayed = mouseArea

            Behavior on scale { NumberAnimation { duration: 20 } }
        }
    }
}

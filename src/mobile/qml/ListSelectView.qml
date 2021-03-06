/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2014 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License
* version 2.0.
*
**********/

import QtQuick 2.2

Rectangle {
    id: top

    property alias model: listView.model
    property alias currentIndex: listView.currentIndex
    property real leftTextMargin: 10
    property bool highlight: true

    signal itemSelected(int index)

    ListView {
        id: listView

        anchors.fill: parent
        anchors.leftMargin: 3
        anchors.rightMargin: 3
        anchors.bottomMargin: 3
        clip: true
        highlightFollowsCurrentItem: true
        currentIndex: 2

        function selectItem(x, y) {
            var index = listView.indexAt(x+contentX,y+contentY);
            currentIndex = index;
            top.itemSelected(index);
        }

        Rectangle {
            id: scrollbar
            anchors.right: listView.right
            y: listView.visibleArea.yPosition * listView.height
            width: 7
            height: listView.visibleArea.heightRatio * listView.height
            color: "black"
            visible: listView.visibleArea.heightRatio < 0.99
        }

        delegate: Rectangle {
            id: entry

            property bool selected: ListView.isCurrentItem ? true : false
            objectName: "entry"

            color: (highlight && ListView.isCurrentItem) ? "#ffeedd" : "white"
            border.width: buttonMouseArea.pressed ? 5 :1
            border.color: "darkgray"
            width: parent.width
            height: {
                var pixel = btStyle.pixelsPerMillimeterY * 7;
                var uiFont = btStyle.uiFontPointSize * 3.5;
                return Math.max(pixel, uiFont);
            }

            Text {
                id: entryText

                anchors.fill: parent
                anchors.leftMargin: leftTextMargin
                anchors.rightMargin: 10
                anchors.topMargin: 10
                verticalAlignment: Text.AlignVCenter
                text: model.text
                font.pointSize: btStyle.uiFontPointSize
                font.bold: highlight && entry.selected
            }
        }

        MouseArea {
            id: buttonMouseArea

            anchors.fill: listView
            onPressed: {

            }

            onClicked: itemWasSelected()

            function itemWasSelected() {
                listView.selectItem(mouseX, mouseY);
                itemSelected(currentIndex);
            }
        }
    }
}

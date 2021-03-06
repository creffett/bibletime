/*********
*
* In the name of the Father, and of the Son, and of the Holy Spirit.
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2014 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#ifndef CBOOKMARKINDEX_H
#define CBOOKMARKINDEX_H

#include <QTimer>
#include <QToolTip>
#include <QTreeView>


class BTMimeData;
class BtBookmarksModel;
class CSwordModuleInfo;
class QAction;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;
class QMenu;
class QMouseEvent;
class QPaintEvent;

/**
* The widget which manages all bookmarks.
*
* \author The BibleTime team
*/
class CBookmarkIndex : public QTreeView {
    Q_OBJECT

public: /* Types: */

    enum MenuAction {
        NewFolder = 0,
        ChangeFolder,

        EditBookmark,
        SortFolderBookmarks,
        SortAllBookmarks,
        ImportBookmarks,
        ExportBookmarks,
        PrintBookmarks,

        DeleteEntries,

        ActionBegin = NewFolder,
        ActionEnd = DeleteEntries
    };

public: /* Methods: */

    CBookmarkIndex(QWidget * parent = 0);
    virtual ~CBookmarkIndex();

    void initTree();

    /**
    * Saves the bookmarks to disk
    */
    void saveBookmarks();

signals:

    /**
    * Is emitted when a module should be opened,
    */
    void createReadDisplayWindow( QList<CSwordModuleInfo *>, const QString & );


protected:

    /** A hack to get the modifiers. */
    virtual void mouseReleaseEvent(QMouseEvent * event);

    /** Needed to paint an drag pointer arrow. */
    virtual void paintEvent(QPaintEvent * event);

    /** Initialize the SIGNAL<->SLOT connections. */
    void initConnections();

    /** Returns the drag object for the current selection. */
    virtual QMimeData * dragObject();

    /**
    * D'n'd methods are reimplementations from QTreeWidget or its ancestors.
    * In these we handle creating, moving and copying bookmarks with d'n'd.
    */
    virtual void dragEnterEvent( QDragEnterEvent * event );
    virtual void dragMoveEvent( QDragMoveEvent * event );
    virtual void dropEvent( QDropEvent * event );
    virtual void dragLeaveEvent( QDragLeaveEvent * event );

    /** Returns the correct action object for the given type of action. */
    QAction * action(MenuAction type) const;

    /** Reimplementation from QAbstractItemView. Takes care of movable items. */
    virtual void startDrag(Qt::DropActions supportedActions);

    /** Handle mouse moving (mag updates) */
    virtual void mouseMoveEvent(QMouseEvent* event);


protected slots:

    /** Prevents annoying folder collapsing while dropping. */
    void expandAutoCollapsedItem(const QModelIndex & index) {
        expand(index);
    }

    /** Is called when an item was clicked or activated. */
    void slotExecuted( const QModelIndex & index );

    /** Shows the context menu at the given position. */
    void contextMenu(const QPoint&);

    /** Adds a new subfolder to the current item. */
    void createNewFolder();

    /** Opens a dialog to change the current folder. */
    void changeFolder();

    /** Exports the bookmarks from the selected folder. */
    void exportBookmarks();

    /** Changes the current bookmark. */
    void editBookmark();

    /** Sorts the current folder bookmarks. */
    void sortFolderBookmarks();

    /** Sorts all bookmarks. */
    void sortAllBookmarks();

    /** Helps with the extra item. */
    void slotItemEntered(const QModelIndex & index);

    /** Import bookmarks from a file and add them to the selected folder. */
    void importBookmarks();

    /** Deletes the selected entries. */
    void deleteEntries(bool confirm = true);

    /** Prints the selected bookmarks. */
    void printBookmarks();

    /** Slot for the mag update timer. */
    void magTimeout();

private:

    /** Initializes the view. */
    void initView();

    /** Convenience function for creating a new action. */
    QAction * newQAction(const QString & text, const QString & pix, int shortcut, const QObject * receiver,
                         const char * slot, QObject * parent);

    /**
    * Returns true if more than one entry is supported by this action type.
    * Returns false for actions which support only one entry.
    */
    bool isMultiAction(const MenuAction type) const;

    /** A helper function for d'n'd which creates a new bookmark item when drop happens. */
    void createBookmarkFromDrop(QDropEvent * event, const QModelIndex & parentItem, int indexInParent);

    /** \todo document */
    bool enableAction(const QModelIndex & index, MenuAction type) const;

    struct Actions {
        QAction * newFolder;
        QAction * changeFolder;

        QAction * editBookmark;
        QAction * sortFolderBookmarks;
        QAction * sortAllBookmarks;
        QAction * importBookmarks;
        QAction * exportBookmarks;
        QAction * printBookmarks;

        QAction * deleteEntries;
    } m_actions;

    QMenu * m_popup;
    QTimer m_magTimer;
    int m_mouseReleaseEventModifiers;
    QModelIndex m_previousEventItem;
    QPoint m_dragMovementPosition;
    QPoint m_dragStartPosition;
    QPersistentModelIndex m_extraItem;

    /** Pointer to bookmarks model, added for convenience. */
    BtBookmarksModel * m_bookmarksModel;

};

#endif

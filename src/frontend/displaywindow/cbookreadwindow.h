/*********
*
* This file is part of BibleTime's BtActionCollection code, http://www.bibletime.info/.
*
* Copyright 1999-2014 by the BibleTime developers.
* The BibleTime BtActionCollection code is licensed under the GNU General Public License version 2.0.
*
**********/

#ifndef CBOOKREADWINDOW_H
#define CBOOKREADWINDOW_H

#include "frontend/displaywindow/clexiconreadwindow.h"


class BtActionCollection;
class CBookTreeChooser;
class QAction;

class CBookReadWindow: public CLexiconReadWindow {

        Q_OBJECT

    public: /* Methods: */

        inline CBookReadWindow(const QList<CSwordModuleInfo *> & modules, CMDIArea * parent)
            : CLexiconReadWindow(modules, parent)
            , m_treeAction(0)
            , m_treeChooser(0) {}

        virtual void storeProfileSettings(const QString & windowGroup);
        virtual void applyProfileSettings(const QString & windowGroup);
        static void insertKeyboardActions(BtActionCollection * const a);

    public slots:

        /**
        * Refreshes the content of this display window and the content of the keychooser.
        */
        virtual void reload(CSwordBackend::SetupChangedReason reason);

    protected: /* Methods: */

        virtual void initActions();
        virtual void initToolbars();
        virtual void initConnections();
        virtual void initView();
        /** Called to add actions to mainWindow toolbars */
        virtual void setupMainWindowToolBars();

        virtual void setupPopupMenu();

    protected slots:

        /**
         * Reimplementation to take care of the tree chooser.
         */
        virtual void modulesChanged();

    private slots:

        /**
        * Is called when the action was executed to toggle the tree view.
        */
        void treeToggled();

    private: /* Fields: */

        QAction * m_treeAction;
        CBookTreeChooser * m_treeChooser;
};

#endif

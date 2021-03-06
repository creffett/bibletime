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

#include "frontend/btbookshelfdockwidget.h"

#include <QAction>
#include <QActionGroup>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include "backend/managers/cswordbackend.h"
#include "bibletime.h"
#include "bibletimeapp.h"
#include "frontend/btbookshelfview.h"
#include "frontend/btbookshelfwidget.h"
#include "frontend/messagedialog.h"
#include "util/cresmgr.h"
#include "util/geticon.h"


namespace {
const QString groupingOrderKey("GUI/MainWindow/Docks/Bookshelf/grouping");
}

BtBookshelfDockWidget *BtBookshelfDockWidget::m_instance = 0;

BtBookshelfDockWidget::BtBookshelfDockWidget(QWidget *parent, Qt::WindowFlags f)
        : QDockWidget(parent, f)
{
    Q_ASSERT(m_instance == 0);
    m_instance = this;

    setObjectName("BookshelfDock");


    // Setup actions and menus:
    initMenus();

    // Setup tree model:
    m_treeModel = new BtBookshelfTreeModel(groupingOrderKey, this);

    // Get backend model:
    BtBookshelfModel *bookshelfModel = CSwordBackend::instance()->model();

    // Setup bookshelf widgets:
    m_bookshelfWidget = new BtBookshelfWidget(this);
    m_bookshelfWidget->setTreeModel(m_treeModel);
    m_bookshelfWidget->setSourceModel(bookshelfModel);
    m_bookshelfWidget->setItemContextMenu(m_itemContextMenu);
    m_bookshelfWidget->treeView()->setMouseTracking(true); // required for moduleHovered
    /// \bug The correct grouping action is not selected on startup.

    // Setup welcome widgets:
    m_welcomeWidget = new QWidget(this);
    QVBoxLayout *welcomeLayout = new QVBoxLayout;
    m_installLabel = new QLabel(this);
    m_installLabel->setWordWrap(true);
    m_installLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    welcomeLayout->addWidget(m_installLabel, 0, Qt::AlignHCenter | Qt::AlignBottom);
    m_installButton = new QPushButton(this);
    welcomeLayout->addWidget(m_installButton, 0, Qt::AlignHCenter | Qt::AlignTop);
    m_welcomeWidget->setLayout(welcomeLayout);

    // Setup stacked widget:
    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->addWidget(m_bookshelfWidget);
    m_stackedWidget->addWidget(m_welcomeWidget);
    m_stackedWidget->setCurrentWidget(bookshelfModel->moduleList().empty()
                                      ? m_welcomeWidget
                                      : m_bookshelfWidget);
    setWidget(m_stackedWidget);

    // Connect signals:
    connect(m_bookshelfWidget->treeView(), SIGNAL(moduleActivated(CSwordModuleInfo*)),
            this,                          SLOT(slotModuleActivated(CSwordModuleInfo*)));
    connect(m_bookshelfWidget->treeView(), SIGNAL(moduleHovered(CSwordModuleInfo*)),
            this,                          SIGNAL(moduleHovered(CSwordModuleInfo*)));
    connect(m_treeModel, SIGNAL(moduleChecked(CSwordModuleInfo*, bool)),
            this,        SLOT(slotModuleChecked(CSwordModuleInfo*, bool)));
    connect(m_treeModel, SIGNAL(groupingOrderChanged(BtBookshelfTreeModel::Grouping)),
            this,        SLOT(slotGroupingOrderChanged(const BtBookshelfTreeModel::Grouping&)));
    connect(m_bookshelfWidget->showHideAction(), SIGNAL(toggled(bool)),
            m_treeModel,                         SLOT(setCheckable(bool)));
    connect(bookshelfModel, SIGNAL(rowsInserted(const QModelIndex&,int,int)),
            this,           SLOT(slotModulesChanged()));
    connect(bookshelfModel, SIGNAL(rowsRemoved(const QModelIndex&,int,int)),
            this,           SLOT(slotModulesChanged()));
    connect(m_installButton,       SIGNAL(clicked()),
            BibleTime::instance(), SLOT(slotSwordSetupDialog()));

    retranslateUi();
}

void BtBookshelfDockWidget::initMenus() {
    namespace RM = CResMgr::mainIndex;

    m_itemContextMenu = new QMenu(this);
    m_itemActionGroup = new QActionGroup(this);
    connect(m_itemActionGroup, SIGNAL(triggered(QAction*)),
            this,              SLOT(slotItemActionTriggered(QAction*)));

    m_itemOpenAction = new QAction(this);
    m_itemActionGroup->addAction(m_itemOpenAction);
    m_itemContextMenu->addAction(m_itemOpenAction);

    m_itemSearchAction = new QAction(this);
    m_itemSearchAction->setIcon(util::getIcon(RM::search::icon));
    m_itemActionGroup->addAction(m_itemSearchAction);
    m_itemContextMenu->addAction(m_itemSearchAction);

    m_itemEditMenu = new QMenu(this);
    m_itemEditMenu->setIcon(util::getIcon(RM::editModuleMenu::icon));
    m_itemContextMenu->addMenu(m_itemEditMenu);
    m_itemEditPlainAction = new QAction(this);
    m_itemEditPlainAction->setIcon(util::getIcon(RM::editModulePlain::icon));
    m_itemActionGroup->addAction(m_itemEditPlainAction);
    m_itemEditMenu->addAction(m_itemEditPlainAction);

    m_itemEditHtmlAction = new QAction(this);
    m_itemEditHtmlAction->setIcon(util::getIcon(RM::editModuleHTML::icon));
    m_itemActionGroup->addAction(m_itemEditHtmlAction);
    m_itemEditMenu->addAction(m_itemEditHtmlAction);

    m_itemUnlockAction = new QAction(this);
    m_itemUnlockAction->setIcon(util::getIcon(RM::unlockModule::icon));
    m_itemActionGroup->addAction(m_itemUnlockAction);
    m_itemContextMenu->addAction(m_itemUnlockAction);

    m_itemAboutAction = new QAction(this);
    m_itemAboutAction->setIcon(util::getIcon(RM::aboutModule::icon));
    m_itemActionGroup->addAction(m_itemAboutAction);
    m_itemContextMenu->addAction(m_itemAboutAction);

    connect(m_itemContextMenu, SIGNAL(aboutToShow()),
            this,              SLOT(slotPrepareItemContextMenu()));
}

void BtBookshelfDockWidget::retranslateUi() {
    setWindowTitle(tr("Bookshelf"));

    m_itemOpenAction->setText(tr("&Open"));
    m_itemEditMenu->setTitle(tr("&Edit"));
    m_itemEditPlainAction->setText(tr("&Plain text"));
    m_itemEditHtmlAction->setText(tr("&HTML"));
    m_itemUnlockAction->setText(tr("&Unlock..."));
    m_itemAboutAction->setText(tr("&About..."));

    m_installLabel->setText(tr("There are currently no works installed. Please "
                               "click the button below to install new works."));
    m_installButton->setText(tr("&Install works..."));
}

void BtBookshelfDockWidget::slotModuleActivated(CSwordModuleInfo *module) {
    if (!module->isLocked()) {
        emit moduleOpenTriggered(module);
    } else {
        /**
          \todo Implement a better unlock dialog, which could incorporate the following
                warning message. Actually the whole case when the user tries to open a locked
                module needs to be rethought and refactored.
        */
        message::showWarning(this, tr("Warning: Module locked!"),
                             tr("You are trying to access an encrypted module. Please "
                                "provide an unlock key in the following dialog to open the "
                                "module."));

        /// \todo We need to keep the module name because unlocking currently reloads sword.
        const QString moduleName(module->name());

        if (BibleTime::moduleUnlock(module)) {
            // Re-initialize module pointer:
            module = CSwordBackend::instance()->findModuleByName(moduleName);
            Q_ASSERT(module != 0);

            emit moduleOpenTriggered(module);
        }
    }
}

void BtBookshelfDockWidget::slotModuleChecked(CSwordModuleInfo *module, bool c) {
    module->setHidden(!c);
}

void BtBookshelfDockWidget::slotItemActionTriggered(QAction *action) {
    CSwordModuleInfo *module((CSwordModuleInfo*) m_itemContextMenu->property("BtModule").value<void*>());
    if (module == 0) return;

    if (action == m_itemOpenAction) {
        emit moduleOpenTriggered(module);
    }
    else if (action == m_itemSearchAction) {
        emit moduleSearchTriggered(module);
    }
    else if (action == m_itemEditPlainAction) {
        emit moduleEditPlainTriggered(module);
    }
    else if (action == m_itemEditHtmlAction) {
        emit moduleEditHtmlTriggered(module);
    }
    else if (action == m_itemUnlockAction) {
        emit moduleUnlockTriggered(module);
    }
    else if (action == m_itemAboutAction) {
        emit moduleAboutTriggered(module);
    }
}

void BtBookshelfDockWidget::slotPrepareItemContextMenu() {
    void *v = m_itemContextMenu->property("BtModule").value<void*>();
    CSwordModuleInfo *module = static_cast<CSwordModuleInfo*>(v);
    m_itemOpenAction->setEnabled(!module->isLocked());
    m_itemSearchAction->setText(tr("&Search in %1...").arg(module->name()));
    m_itemSearchAction->setEnabled(!module->isLocked());
    m_itemEditMenu->setEnabled(module->isWritable());
    m_itemUnlockAction->setEnabled(module->isLocked());
}

void BtBookshelfDockWidget::slotModulesChanged() {
    const BtBookshelfModel *bookshelfModel = CSwordBackend::instance()->model();
    m_stackedWidget->setCurrentWidget(bookshelfModel->moduleList().empty()
                                      ? m_welcomeWidget
                                      : m_bookshelfWidget);
}

void BtBookshelfDockWidget::slotGroupingOrderChanged(
        const BtBookshelfTreeModel::Grouping &g)
{
    g.saveTo(groupingOrderKey);
    emit groupingOrderChanged(g);
}

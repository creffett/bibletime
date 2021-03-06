/*********
*
* This file is part of BibleTime's source code, http://www.bibletime.info/.
*
* Copyright 1999-2014 by the BibleTime developers.
* The BibleTime source code is licensed under the GNU General Public License version 2.0.
*
**********/

#include "frontend/bookshelfmanager/indexpage/btindexpage.h"

#include <QCheckBox>
#include <QDir>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include "backend/config/btconfig.h"
#include "backend/drivers/cswordmoduleinfo.h"
#include "backend/managers/cswordbackend.h"
#include "bibletimeapp.h"
#include "frontend/bookshelfmanager/btmodulemanagerdialog.h"
#include "frontend/btmoduleindexdialog.h"
#include "util/cresmgr.h"
#include "util/geticon.h"
#include "util/tool.h"


BtIndexPage::BtIndexPage(BtModuleManagerDialog *parent)
        : BtConfigDialog::Page(util::getIcon(CResMgr::bookshelfmgr::indexpage::icon), parent)
{
    QVBoxLayout *vboxLayout = new QVBoxLayout(this);
    QHBoxLayout *hboxLayout;

    m_autoDeleteOrphanedIndicesBox = new QCheckBox(this);
    vboxLayout->addWidget(m_autoDeleteOrphanedIndicesBox);

    m_moduleList = new QTreeWidget(this);
    vboxLayout->addWidget(m_moduleList);

    hboxLayout = new QHBoxLayout();

    QSpacerItem *spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hboxLayout->addItem(spacerItem);

    m_deleteButton = new QPushButton(this);
    hboxLayout->addWidget(m_deleteButton);

    m_createButton = new QPushButton(this);
    hboxLayout->addWidget(m_createButton);

    vboxLayout->addLayout(hboxLayout);

    // configure the list view
    m_moduleList->setHeaderLabels( (QStringList(tr("Work")) << tr("Index size")) ); /// \todo Move to retranslateUi()
    m_moduleList->setRootIsDecorated(true);
    m_moduleList->setColumnWidth(0, util::tool::mWidth(m_moduleList, 20) );
    //m_moduleList->setTextAlignment(1, Qt::AlignRight); see doc...
    m_moduleList->setSortingEnabled(false);

    m_autoDeleteOrphanedIndicesBox->setChecked( btConfig().value<bool>("settings/behaviour/autoDeleteOrphanedIndices", true) );

    // icons for our buttons
    m_createButton->setIcon(util::getIcon(CResMgr::bookshelfmgr::indexpage::create_icon));
    m_deleteButton->setIcon(util::getIcon(CResMgr::bookshelfmgr::indexpage::delete_icon));

    // connect our signals/slots
    connect(m_createButton, SIGNAL(clicked()),
            this,           SLOT(createIndices()));
    connect(m_deleteButton, SIGNAL(clicked()),
            this,           SLOT(deleteIndices()));
    connect(CSwordBackend::instance(), SIGNAL(sigSwordSetupChanged(CSwordBackend::SetupChangedReason)),
            this,                      SLOT(slotSwordSetupChanged()));

    retranslateUi(); // also calls populateModuleList();
}

BtIndexPage::~BtIndexPage() {
    btConfig().setValue("settings/behaviour/autoDeleteOrphanedIndices", m_autoDeleteOrphanedIndicesBox->isChecked() );
}

/** Populates the module list with installed modules and orphaned indices */
void BtIndexPage::populateModuleList() {
    typedef QList<CSwordModuleInfo*>::const_iterator MLCI;

    m_moduleList->clear();

    // populate installed modules
    m_modsWithIndices = new QTreeWidgetItem(m_moduleList);
    m_modsWithIndices->setText(0, tr("Indexed Works"));
    m_modsWithIndices->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
    m_modsWithIndices->setExpanded(true);

    m_modsWithoutIndices = new QTreeWidgetItem(m_moduleList);
    m_modsWithoutIndices->setText(0, tr("Unindexed Works"));
    m_modsWithoutIndices->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsTristate);
    m_modsWithoutIndices->setExpanded(true);

    const QList<CSwordModuleInfo*> &modules(CSwordBackend::instance()->moduleList());
    for (MLCI it(modules.begin()); it != modules.end(); ++it) {
        QTreeWidgetItem* item = 0;

        if ((*it)->hasIndex()) {
            item = new QTreeWidgetItem(m_modsWithIndices);
            item->setText(0, (*it)->name());
            item->setText(1, tr("%1 KiB").arg((*it)->indexSize() / 1024));
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(0, Qt::Unchecked);
        }
        else {
            item = new QTreeWidgetItem(m_modsWithoutIndices);
            item->setText(0, (*it)->name());
            item->setText(1, tr("0 KiB"));
            item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            item->setCheckState(0, Qt::Checked);
        }
    }
}

void BtIndexPage::retranslateUi() {
    setHeaderText(tr("Search Indexes"));

    m_autoDeleteOrphanedIndicesBox->setToolTip(tr("If selected, those indexes which have no corresponding work will be deleted when BibleTime starts"));
    m_autoDeleteOrphanedIndicesBox->setText(tr("Automatically delete orphaned indexes when BibleTime starts"));

    m_deleteButton->setToolTip(tr("Delete the selected indexes"));
    m_deleteButton->setText(tr("Delete"));

    m_createButton->setToolTip(tr("Create new indexes for the selected works"));
    m_createButton->setText(tr("Create..."));

    populateModuleList();
}

/** Creates indices for selected modules if no index currently exists */
void BtIndexPage::createIndices() {
    bool indicesCreated = false;
    QList<const CSwordModuleInfo*> moduleList;

    for (int i = 0; i < m_modsWithoutIndices->childCount(); i++) {
        if (m_modsWithoutIndices->child(i)->checkState(0) == Qt::Checked) {
            CSwordModuleInfo* module = CSwordBackend::instance()->findModuleByName(m_modsWithoutIndices->child(i)->text(0).toUtf8());
            if (module) {
                moduleList.append( module );
                indicesCreated = true;
            }
        }
    }

    //Shows the progress dialog
    if (indicesCreated) {
        BtModuleIndexDialog::indexAllModules(moduleList);
        populateModuleList();
    }
}

/** Deletes indices for selected modules */
void BtIndexPage::deleteIndices() {
    bool indicesDeleted = false;

    for (int i = 0; i < m_modsWithIndices->childCount(); i++) {
        if (m_modsWithIndices->child(i)->checkState(0) == Qt::Checked) {
            CSwordModuleInfo* module = CSwordBackend::instance()->findModuleByName(m_modsWithIndices->child(i)->text(0).toUtf8());
            if (module) {
                module->deleteIndex();
                indicesDeleted = true;
            }
        }
    }

    // repopulate the list if an action was taken
    if (indicesDeleted) {
        populateModuleList();
    }
}

void BtIndexPage::slotSwordSetupChanged() {
    populateModuleList();
}


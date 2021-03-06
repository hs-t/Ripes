#include "processorselectiondialog.h"
#include "ui_processorselectiondialog.h"

#include <QDialogButtonBox>

#include "processorhandler.h"
#include "radix.h"

namespace Ripes {

ProcessorSelectionDialog::ProcessorSelectionDialog(QWidget* parent)
    : QDialog(parent), m_ui(new Ui::ProcessorSelectionDialog) {
    m_ui->setupUi(this);
    setWindowTitle("Select Processor");

    // Initialize top level ISA items
    m_ui->processors->setHeaderHidden(true);
    std::map<ISA, QTreeWidgetItem*> isaItems;
    for (const auto& isa : ISANames) {
        auto* isaItem = new QTreeWidgetItem({isa.second});
        isaItems[isa.first] = isaItem;
        isaItem->setFlags(isaItem->flags() & ~(Qt::ItemIsSelectable));
        m_ui->processors->insertTopLevelItem(m_ui->processors->topLevelItemCount(), isaItem);
    }

    // Initialize processor list
    QTreeWidgetItem* selectedItem = nullptr;

    for (auto& desc : ProcessorRegistry::getAvailableProcessors()) {
        QTreeWidgetItem* processorItem = new QTreeWidgetItem({desc.second.name});
        processorItem->setData(ProcessorColumn, Qt::UserRole, QVariant::fromValue(desc.second.id));
        if (desc.second.id == ProcessorHandler::get()->getID()) {
            auto font = processorItem->font(ProcessorColumn);
            font.setBold(true);
            processorItem->setFont(ProcessorColumn, font);
            selectedItem = processorItem;
        }
        auto* isaItem = isaItems.at(desc.second.isa->isaID());
        isaItem->insertChild(isaItem->childCount(), processorItem);
    }

    connect(m_ui->processors, &QTreeWidget::currentItemChanged, this, &ProcessorSelectionDialog::selectionChanged);

    connect(m_ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    if (selectedItem != nullptr) {
        m_ui->processors->setCurrentItem(selectedItem);
    }
}

RegisterInitialization ProcessorSelectionDialog::getRegisterInitialization() const {
    return m_ui->regInitWidget->getInitialization();
}

ProcessorSelectionDialog::~ProcessorSelectionDialog() {
    delete m_ui;
}

void ProcessorSelectionDialog::selectionChanged(QTreeWidgetItem* current, QTreeWidgetItem*) {
    QVariant selectedItemData = current->data(ProcessorColumn, Qt::UserRole);
    const bool validSelection = selectedItemData.canConvert<ProcessorID>();
    m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(validSelection);
    if (!validSelection) {
        // Something which is not a processor was selected (ie. an ISA). Disable OK button
        return;
    }

    const ProcessorID id = qvariant_cast<ProcessorID>(current->data(ProcessorColumn, Qt::UserRole));
    const auto& desc = ProcessorRegistry::getAvailableProcessors().at(id);

    // Update information widgets with the current processor info
    m_selectedID = id;
    m_ui->name->setText(desc.name);
    m_ui->ISA->setText(desc.isa->name());
    m_ui->description->setPlainText(desc.description);
    m_ui->regInitWidget->processorSelectionChanged(id);

    m_ui->layout->clear();
    for (const auto& layout : desc.layouts) {
        m_ui->layout->addItem(layout.name);
    }
}

Layout ProcessorSelectionDialog::getSelectedLayout() const {
    const auto& desc = ProcessorRegistry::getAvailableProcessors().at(m_selectedID);
    for (const auto& layout : desc.layouts) {
        if (layout.name == m_ui->layout->currentText()) {
            return layout;
        }
    }
    Q_UNREACHABLE();
}

}  // namespace Ripes

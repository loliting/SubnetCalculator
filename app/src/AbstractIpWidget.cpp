// This file is part of Subnet Calculator.
// Copyright (C) 2026 Karol Maksymowicz
//
// Subnet Calculator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, If not,
// see <https://www.gnu.org/licenses/>.

#include "AbstractIpWidget.hpp"
#include "ui_AbstractIpWidget.h"

#include <QtCore/QFile>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QFileDialog>
#include <QtGui/QClipboard>

#include "SaveAsDialog.hpp"
#include "IpInputEventFilter.hpp"

AbstractIpWidget::AbstractIpWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AbstractIpWidget)
{
    ui->setupUi(this);

    ui->subnetsTable->header()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->subnetsTable->header()->setMinimumWidth(3);
    ui->subnetsTable->setColumnWidth(0, 0);

    connect(ui->calculateBtn, &QPushButton::clicked, 
        this, [this]{ _update(true); }); 
    connect(ui->addressInput, &QLineEdit::textChanged, 
        this, [this]{ _update(false); }); 
    connect(ui->cidrInput, &QSpinBox::valueChanged,
        this, [this]{ _update(false); }); 
    connect(ui->subnetCountInput, &QComboBox::currentIndexChanged,
        this, [this]{ _update(false); }); 

    ui->calculateBtn->hide();
    
    // template for all subnetTable's items, so we dont have to prepare the item each time
    itemTemplate = new QTreeWidgetItem();
    itemTemplate->setTextAlignment(0, Qt::AlignRight | Qt::AlignVCenter);
    for(int i = 1; i < 6; ++i)
        itemTemplate->setTextAlignment(i, Qt::AlignCenter);
}

AbstractIpWidget::~AbstractIpWidget() {
    if(itemTemplate) {
        delete itemTemplate;
        itemTemplate = nullptr;
    } 
    delete ui;
}

void AbstractIpWidget::_update(bool buttonSender) {
    ui->subnetsTable->setColumnWidth(0, 0);
    ui->subnetsTable->clear();
    setSaveTableAvaliable(false);

    if(!buttonSender && ui->subnetCountInput->currentIndex() > 8) {
        ui->calculateBtn->show();
        update(false);
        return;
    }
    ui->calculateBtn->setVisible(buttonSender);
    update(true);

    QTreeWidgetItem* rootItem = ui->subnetsTable->topLevelItem(0);
    if(!rootItem) return;
    int subnetCount = rootItem->childCount();
    if(subnetCount < 1) return;

    rootItem->setExpanded(true);
    QFontMetrics subnetIdFontMetrics = ui->subnetsTable->fontMetrics();
    int subnetIdFontWidth = subnetIdFontMetrics.horizontalAdvance(
        QString::number(subnetCount)
    );
    ui->subnetsTable->setColumnWidth(0, 60 + subnetIdFontWidth);
}

void AbstractIpWidget::setSaveTableAvaliable(bool canSaveTable) {
    isSaveTableAvaliable = canSaveTable;
    emit saveTableAvaliable(isSaveTableAvaliable);
}

bool AbstractIpWidget::canSaveTable() {
    return isSaveTableAvaliable;
}

void AbstractIpWidget::saveAsCsvTable() {
    if(ui->subnetsTable->topLevelItemCount() < 1) {
        return;
    }

    bool hasSubnets = ui->subnetsTable->topLevelItem(0)->childCount() > 1;
    SaveAsDialog dialog(hasSubnets, this);
    dialog.exec();
    if(!dialog.result().has_value())
        return;
        
    auto opt = dialog.result().value();

    QStringConverter::Encoding encoding;

    switch (opt.encoding & ~CsvEncoding::BOM_FLAG) {
    case CsvEncoding::UTF8:
        encoding = QStringConverter::Encoding::Utf8;
        break;
    case CsvEncoding::UTF16BE:
        encoding = QStringConverter::Encoding::Utf16BE;
        break;
    case CsvEncoding::UTF16LE:
        encoding = QStringConverter::Encoding::Utf16LE;
    case CsvEncoding::UTF32BE:
        encoding = QStringConverter::Encoding::Utf32BE;
        break;
    case CsvEncoding::UTF32LE:
        encoding = QStringConverter::Encoding::Utf32LE;
        break;
    default:
        assert(0); // Unreachable
        break;
    }

    QFileDialog fileDialog(this, tr("Save table"), QString(), tr("(*.csv)"));
    fileDialog.setFileMode(QFileDialog::FileMode::AnyFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    if(fileDialog.exec() == QDialog::Rejected)
        return;

    QString savePath = fileDialog.selectedFiles()[0];
    QFile f(savePath);
    if(f.open(QIODevice::WriteOnly) == false) {
        QMessageBox::critical(QApplication::activeWindow(),
            tr("Failed to save the table"), 
            tr("Failed to open file: \"") + savePath + "\": " + f.errorString()
        );
        return;
    }
    
    QTextStream stream(&f);
    stream.setEncoding(encoding);
    stream.setGenerateByteOrderMark(opt.encoding & CsvEncoding::BOM_FLAG);

    int columnCount = ui->subnetsTable->columnCount();
    for(int i = 1; opt.saveHeaderNames && i < columnCount; ++i)
        stream << opt.delimiter 
               << ui->subnetsTable->headerItem()->text(i)
               << opt.delimiter
               << ((i == columnCount - 1) ? "\r\n" : opt.separator);
    
    for(int i = 1; opt.saveRootNet && i < columnCount; ++i)
        stream << opt.delimiter
               << ui->subnetsTable->itemAt(0, 0)->text(i)
               << opt.delimiter
               << ((i == columnCount - 1) ? "\r\n" : opt.separator);
    
    if(opt.saveSubnetsSeparator) {
        stream << opt.delimiter
               << tr("Subnets:")
               << opt.delimiter
               << opt.separator;

        for(int i = 2; i < columnCount; ++i)
            stream << opt.delimiter
                   << opt.delimiter 
                   << ((i == columnCount - 1) ? "\r\n" : opt.separator);
    }

    auto items = ui->subnetsTable->itemAt(0, 0);
    for(unsigned i = 0; i < items->childCount(); ++i) {
        auto item = items->child(i);
        for(int i = 1; i < columnCount; ++i)
            stream << opt.delimiter
                   << item->text(i)
                   << opt.delimiter
                   << ((i == columnCount - 1) ? "\r\n" : opt.separator);
    }
    
    stream.flush();
    f.close();

    if(f.error() != QFileDevice::NoError) {
        QMessageBox(QMessageBox::Critical,
                    tr("IO error"),
                    tr("Failed to save table. ") + f.errorString()
        ).exec();
    }
}

bool AbstractIpWidget::copy() {
    if(ui->addressInput->hasFocus()) {
        ui->addressInput->copy();
    }
    else if(ui->cidrInput->hasFocus()) {
        QWidget* w = ui->cidrInput->property("_q_spinbox_lineedit").value<QWidget*>();
        if(!w) {
            qWarning() << __FUNCTION__ << "Could not get QLineEdit property of QSpinBox";
            return false;
        }
        QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(w);
        assert(lineEdit);
        lineEdit->copy();
    }
    else if(ui->subnetsTable->hasFocus() 
            && ui->subnetsTable->selectedItems().count() > 0)
    {
        int selectedColumn = ui->subnetsTable->currentColumn();
        QTreeWidgetItem* selectedItem = ui->subnetsTable->selectedItems()[0];
        QApplication::clipboard()->setText(selectedItem->text(selectedColumn));
    }
    else {
        return false;
    }
    return true;
}

void AbstractIpWidget::cut() {
    if(ui->addressInput->hasFocus()) {
        ui->addressInput->cut();
    }
    else if(ui->cidrInput->hasFocus()) {
        QWidget* w = ui->cidrInput->property("_q_spinbox_lineedit").value<QWidget*>();
        if(!w) {
            qWarning() << __FUNCTION__ << "Could not get QLineEdit property of QSpinBox";
        }
        QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(w);
        assert(lineEdit);
        lineEdit->cut();
    }
}

void AbstractIpWidget::paste() {
    if(ui->addressInput->hasFocus()) {
        ui->addressInput->paste();
    }
    else if(ui->cidrInput->hasFocus()) {
        QWidget* w = ui->cidrInput->property("_q_spinbox_lineedit").value<QWidget*>();
        if(!w) {
            qWarning() << __FUNCTION__ << "Could not get QLineEdit property of QSpinBox";
        }
        QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(w);
        assert(lineEdit);
        lineEdit->paste();
    }
}
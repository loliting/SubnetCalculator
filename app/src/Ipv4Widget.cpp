// This file is part of Subnet Calculator.
// Copyright (C) 2024 Karol Maksymowicz
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

#include "Ipv4Widget.hpp"
#include "ui_Ipv4Widget.h"

#include <QtCore/QFile>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QFileDialog>

#include "SaveAsDialog.hpp"

#define IPV4_REGEX "^((25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)(\\.(?!$)|$)){3}(25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)$"

using namespace libSubnetCalculator;

Ipv4InputEventFilter::Ipv4InputEventFilter(
        QSpinBox* cidrInput,
        QLineEdit* ipv4AddressInput) :
    cidrInput(cidrInput),
    ipv4AddressInput(ipv4AddressInput),
    QObject(nullptr)
{
    
}

bool Ipv4InputEventFilter::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if(keyEvent->key() == Qt::Key_Slash 
        || keyEvent->key() == Qt::Key_Return
        || keyEvent->key() == Qt::Key_Enter)
        {
            cidrInput->setFocus();
            cidrInput->selectAll();
            return true;
        }
        else if(keyEvent->key() == ' ') { // Try "jumping" to the next IPv4 octet
            ipv4AddressInput->insert(".");

            // Check if current contents of ipv6AddressInput is valid, if so 
            // jump to the cidr SpinBox instead
            const QValidator *validator = ipv4AddressInput->validator();
            QString str = ipv4AddressInput->text();
            int pos = 0;
            if(validator->validate(str, pos) == QValidator::Acceptable) {
                cidrInput->setFocus();
                cidrInput->selectAll();
            }

            return true;
        }

        return false;
    }
    
    return QObject::eventFilter(obj, event);
}

Ipv4Widget::Ipv4Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Ipv4Widget),
    // Initializate `lastIpv4Class` with non-existing class
    lastIpv4Class((IPv4Address::Class)(-1))
{
    ui->setupUi(this);

    ui->ipv4Address->setValidator(
        new QRegularExpressionValidator(QRegularExpression(IPV4_REGEX), this)
    );
    inputFilter = new Ipv4InputEventFilter(ui->cidr, ui->ipv4Address);
    ui->ipv4Address->installEventFilter(inputFilter);

    ui->subnetsTable->header()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->subnetsTable->setColumnWidth(0, 0);
    
    ui->calculate->hide();
}

Ipv4Widget::~Ipv4Widget() {
    inputFilter->deleteLater();

    delete ui;
}


/* UI signal handlers slots */
void Ipv4Widget::on_ipv4Address_textEdited(const QString) {
    updateIpv4AddressClassTooltip();
    update();
}

void Ipv4Widget::on_cidr_valueChanged(int) {
    update();
}

void Ipv4Widget::on_subnetCount_currentIndexChanged(int index) {
    update();
}

void Ipv4Widget::on_calculate_clicked() {
    updateTableContent();
    emit saveTableAvaliable(isSaveTableAvaliable);
}


void Ipv4Widget::updateIpv4AddressClassTooltip() {
    IPv4Address::Class ipv4Class;
    try {
        ipv4Class = IPv4Address(ui->ipv4Address->text().toStdString()).getClass();
    }
    catch(InvalidAddressException& e) {
        return;
    }
    catch(libSubnetCalculatorException& e) {
        qWarning() << "Unknown libSubnetCalculatorException";
        return;
    }

    if(lastIpv4Class == ipv4Class)
        return;
    lastIpv4Class = ipv4Class;

    ui->cidr->blockSignals(true);
    switch(ipv4Class) {
    case IPv4Address::Class::A:
        ui->ipv4Address->setToolTip(tr("Class A"));
        ui->cidr->setValue(8);
        break;
    case IPv4Address::Class::B:
        ui->ipv4Address->setToolTip(tr("Class B"));
        ui->cidr->setValue(16);
        break;
    case IPv4Address::Class::C:
        ui->ipv4Address->setToolTip(tr("Class C"));
        ui->cidr->setValue(24);
        break;
    case IPv4Address::Class::D:
        ui->ipv4Address->setToolTip(tr("Class D"));
        ui->cidr->setValue(4);
        break;
    case IPv4Address::Class::E:
        ui->ipv4Address->setToolTip(tr("Class E"));
        ui->cidr->setValue(4);
        break;
    }
    ui->cidr->blockSignals(false);
}

void Ipv4Widget::updateSubnetCountRange() {
    ui->subnetCount->blockSignals(true);

    int currentSubnetCountIndex = ui->subnetCount->currentIndex();
    ui->subnetCount->clear();
    ui->subnetCount->addItem(QString::number(1));

    if(ui->ipv4Address->hasAcceptableInput()) {
        uint32_t ipCount = pow(2, 32 - ui->cidr->value() - 1);
        for(uint32_t i = 2; i < ipCount; i *= 2)
            ui->subnetCount->addItem(QString::number(i));
    }

    ui->subnetCount->setCurrentIndex(
        qMin(currentSubnetCountIndex, ui->subnetCount->count() - 1)
    );

    ui->subnetCount->blockSignals(false);
}

void Ipv4Widget::updateTableContent() {
    auto getNetworkAddressString = [=](IPv4Network &net) {
        switch (net.CIDR()){
        case 31:
            return tr("N/A\nPoint-to-Point Link\n(RFC 3021)");
        case 32:
            return tr("N/A\n(Host Route)");
        default:
            return QString::fromStdString(net.networkAddress().toString());
        }
    };
    auto getIpv4RangeString = [=](IPv4Network &net) {
        if(net.CIDR() == 32)
            return QString::fromStdString(net.host(0).toString());
        return QString::fromStdString(net.host(0).toString())
             + "\n-\n"
             + QString::fromStdString(net.host(net.hostCount() - 1).toString());
    };
    auto getBroadcastAddressString = [=](IPv4Network &net) {
        switch (net.CIDR()){
        case 32:
            return tr("N/A\n(Host Route)");
        case 31:
            return tr("N/A\nPoint-to-Point Link\n(RFC 3021)");
        default:
            return QString::fromStdString(net.broadcastAddress().toString());
        }
    };
    auto getMaskString = [=](IPv4Network &net) {
        return QString::fromStdString(net.subnetMask().toString())
            + "\n(/" + QString::number(net.CIDR()) + ")";
    };


    isSaveTableAvaliable = false;
    ui->subnetsTable->clear();
    uint32_t subnetCount = qMax<uint32_t>(pow(2, ui->subnetCount->currentIndex()), 1);

    try {
        net = IPv4Network(
            IPv4Address(ui->ipv4Address->text().toStdString()),
            ui->cidr->value()
        );
        subnets = net.getSubnets(subnetCount);
    }
    catch(InvalidAddressException& e) {
        return;
    }
    catch(libSubnetCalculatorException& e) {
        QMessageBox msgBox(QMessageBox::Critical,       /* icon */
            tr("Computation error"),                    /* title */
            tr("An error occurred during calculating table's content:\n")
                + e.what(),                             /* message */
            QMessageBox::Ok,                            /* buttons */
            this                                        /* messagebox parent */
        );
        msgBox.exec();
        return;
    }


    isSaveTableAvaliable = true;
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(ui->subnetsTable);
    rootItem->setText(1, getNetworkAddressString(net));
    rootItem->setText(2, getIpv4RangeString(net));
    rootItem->setText(3, getBroadcastAddressString(net));
    rootItem->setText(4, getMaskString(net));
    rootItem->setText(5, QString::number(net.hostCount()));
    for(int i = 0; i < 6; ++i)
        rootItem->setTextAlignment(i, Qt::AlignCenter);

    if(subnetCount > 1) {
        rootItem->setExpanded(true);

        QFontMetrics subnetIdFontMetrics = ui->subnetsTable->fontMetrics();
        int subnetIdFontWidth = subnetIdFontMetrics.horizontalAdvance(
            QString::number(subnetCount)
        );
        ui->subnetsTable->setColumnWidth(0, 60 + subnetIdFontWidth);

        for(int i = 0; i < subnets.size(); ++i) {
            QTreeWidgetItem *item = new QTreeWidgetItem(rootItem);
            item->setText(0, QString::number(i + 1));
            item->setTextAlignment(0, Qt::AlignRight);

            item->setText(1, getNetworkAddressString(subnets.at(i)));
            item->setText(2, getIpv4RangeString(subnets.at(i)));
            item->setText(3, getBroadcastAddressString(subnets.at(i)));
            item->setText(4, getMaskString(subnets.at(i)));
            item->setText(5, QString::number(subnets.at(i).hostCount()));
            
            for(int j = 1; j < 6; ++j)
                item->setTextAlignment(j, Qt::AlignCenter);
        }
    }
}

void Ipv4Widget::update() {
    updateSubnetCountRange();

    ui->subnetsTable->setColumnWidth(0, 0);
    ui->subnetsTable->clear();
    isSaveTableAvaliable = false;
    if(ui->subnetCount->currentIndex() > 8)
        ui->calculate->show();
    else {
        ui->calculate->hide();
        updateTableContent();
    }

    emit saveTableAvaliable(isSaveTableAvaliable);
}

bool Ipv4Widget::canSaveTable() {
    return isSaveTableAvaliable;
}

void Ipv4Widget::saveTable() {
    SaveAsDialog dialog(subnets.size() > 1, this);
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
    f.open(QIODevice::WriteOnly);
    
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

    if(f.error() != QFileDevice::NoError)
        QMessageBox(QMessageBox::Critical,
                    tr("IO error"),
                    tr("Failed to save table. ") + f.errorString()
        ).exec();
    
}
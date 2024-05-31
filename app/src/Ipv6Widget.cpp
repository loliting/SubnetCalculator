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

#include "Ipv6Widget.hpp"
#include "ui_Ipv6Widget.h"

#include <QtGui/QKeyEvent>
#include <QtWidgets/QMessageBox>

#define IPV6_REGEX                                                             \
  "^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:)"             \
  "{1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:)"       \
  "{1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4})"  \
  "{1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:)" \
  "{1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})"  \
  "|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4})"                    \
  "{0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:)"                              \
  "{0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.)"                         \
  "{3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:)"          \
  "{1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.)"                        \
  "{3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))$"

using namespace libSubnetCalculator;

Ipv6InputEventFilter::Ipv6InputEventFilter(
        QSpinBox* cidrInput,
        QLineEdit* ipv6AddressInput) :
    cidrInput(cidrInput),
    ipv6AddressInput(ipv6AddressInput),
    QObject(nullptr)
{
    
}

bool Ipv6InputEventFilter::eventFilter(QObject *obj, QEvent *event) {
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
        else if(keyEvent->key() == ' ') { // Try "jumping" to the next IPv6 octet
            ssize_t len = ipv6AddressInput->text().length();
            ipv6AddressInput->insert(":");
            len -= ipv6AddressInput->text().length();
            
            // IPv4 mapped IPv6 address
            ipv6AddressInput->insert(".");

            // Check if current contents of ipv6AddressInput is valid, if so 
            // jump to the cidr SpinBox instead
            const QValidator *validator = ipv6AddressInput->validator();
            QString str = ipv6AddressInput->text();
            int pos = 0;
            // If colon was now inserted we don't want to jump to SpinBox yet,
            // because user may still append some octets at the end
            bool wasColonInserted = len == -1;
            if(validator->validate(str, pos) == QValidator::Acceptable
            && !wasColonInserted)
            {
                cidrInput->setFocus();
                cidrInput->selectAll();
            }

            return true;
        }

        return false;
    }

    return QObject::eventFilter(obj, event);
}

Ipv6Widget::Ipv6Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Ipv6Widget)
{
    ui->setupUi(this);

    ui->ipv6Address->setValidator(
        new QRegularExpressionValidator(QRegularExpression(IPV6_REGEX), this)
    );
    inputFilter = new Ipv6InputEventFilter(ui->cidr, ui->ipv6Address);
    ui->ipv6Address->installEventFilter(inputFilter);

    ui->subnetsTable->header()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->subnetsTable->header()->setMinimumWidth(3);
    ui->subnetsTable->setColumnWidth(0, 0);
    
    ui->calculate->hide();
}

Ipv6Widget::~Ipv6Widget() {
    inputFilter->deleteLater();

    delete ui;
}


/* UI signal handlers slots */
void Ipv6Widget::on_ipv6Address_textEdited(const QString&) {
    update();
}

void Ipv6Widget::on_subnetCount_currentIndexChanged(int) {
    update();
}

void Ipv6Widget::on_cidr_valueChanged(int) {
    update();
}

void Ipv6Widget::on_calculate_clicked() {
    updateTableContent();
}


void Ipv6Widget::update() {
    updateSubnetCountRange();

    ui->subnetsTable->setColumnWidth(0, 0);
    ui->subnetsTable->clear();
    if(ui->subnetCount->currentIndex() > 8)
        ui->calculate->show();
    else {
        ui->calculate->hide();
        updateTableContent();
    }
}

void Ipv6Widget::updateSubnetCountRange() {
    ui->subnetCount->blockSignals(true);

    int currentSubnetCountIndex = ui->subnetCount->currentIndex();
    ui->subnetCount->clear();

    if(ui->ipv6Address->hasAcceptableInput()) {
        uint_fast8_t cidr = ui->cidr->value();
        uint128 ipCount = 1;
        for(uint_fast8_t i = 0; i < 128 - cidr - 1; ++i, ipCount *= 2) {
            ui->subnetCount->addItem(
                QString::fromStdString(uint128toStdString(ipCount))
                + " (2^" + QString::number(i) + ")"
            );
        }
    }

    if(ui->subnetCount->count() == 0)
        ui->subnetCount->addItem("1 (2^0)");

    ui->subnetCount->setCurrentIndex(
        qMin(currentSubnetCountIndex, ui->subnetCount->count() - 1)
    );

    ui->subnetCount->blockSignals(false);
}

void Ipv6Widget::updateTableContent() {
    auto getIpv6RoutingPrefix = [=](IPv6Network &net) {
        if(net.CIDR() == 128)
            return tr("N/A\n(Host Route)");
        return QString::fromStdString(net.routingPrefix().toString());
    };
    auto getIpv6RangeString = [=](IPv6Network &net) {
        if(net.CIDR() >= 127)
            return QString::fromStdString(net.host(0).toString());
        return QString::fromStdString(net.host(0).toString())
             + "\n-\n"
             + QString::fromStdString(net.host(net.hostCount() - 1).toString());
    };


    ui->subnetsTable->clear();

    uint64_t subnetCount = 1;
    for(uint_fast8_t i = 0; i < ui->subnetCount->currentIndex(); ++i)
        subnetCount *= 2;

    try {
        net = IPv6Network(
            IPv6Address(ui->ipv6Address->text().toStdString()),
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


    QTreeWidgetItem *rootItem = new QTreeWidgetItem(ui->subnetsTable);
    rootItem->setText(1, getIpv6RoutingPrefix(net));
    rootItem->setText(2, getIpv6RangeString(net));
    rootItem->setText(3, "/" + QString::number(net.CIDR()));
    rootItem->setText(4, QString::fromStdString(
        uint128toStdString(net.hostCount())
    ));
    for(int i = 0; i < 5; ++i)
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
            item->setText(1, getIpv6RoutingPrefix(subnets[i]));
            item->setText(2, getIpv6RangeString(subnets[i]));
            item->setText(3, "/" + QString::number(subnets[i].CIDR()));
            item->setText(4, QString::fromStdString(
                uint128toStdString(subnets.at(i).hostCount())
            ));

            for(int j = 1; j < 6; ++j)
                item->setTextAlignment(j, Qt::AlignCenter);
        }
    }
}
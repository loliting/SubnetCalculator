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

#include "Ipv6Widget.hpp"
#include "ui_AbstractIpWidget.h"

#include <QMessageBox>

#include "SaveAsDialog.hpp"
#include "IpInputEventFilter.hpp"

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

Ipv6Widget::Ipv6Widget(QWidget* parent) :
    AbstractIpWidget(parent)
{
    ui->subnetsTable->setHeaderLabels(QStringList()
        << tr("No.")
        << tr("Routing Prefix")
        << tr("Range")
        << tr("Prefix Length (CIDR)")
        << tr("Host Count")
    );

    ui->addressInput->setValidator(
        new QRegularExpressionValidator(QRegularExpression(IPV6_REGEX), this)
    );

    inputFilter = new IpInputEventFilter(ui->cidrInput, [this]() -> void {
        ssize_t len = ui->addressInput->text().length();
        ui->addressInput->insert(":");
        len -= ui->addressInput->text().length();
        
        // IPv4 mapped IPv6 address
        ui->addressInput->insert(".");

        // Check if current contents of ipv6Address is valid, if so 
        // jump to the cidr SpinBox instead
        const QValidator *validator = ui->addressInput->validator();
        QString str = ui->addressInput->text();
        int pos = 0;
        // If colon was just inserted we don't want to jump to SpinBox yet,
        // because user may still append some octets at the end
        bool wasColonInserted = len == -1;
        if(validator->validate(str, pos) == QValidator::Acceptable
           && !wasColonInserted)
        {
            ui->cidrInput->setFocus();
            ui->cidrInput->selectAll();
        }
    });
    ui->addressInput->installEventFilter(inputFilter);
    ui->addressInput->setPlaceholderText("2001:0db8:85a3::8a2e:0370:7334");
    ui->addressInput->setMinimumWidth(300);

    ui->cidrInput->setMaximum(128);
}

Ipv6Widget::~Ipv6Widget() {
    inputFilter->deleteLater();
    inputFilter = nullptr;
}


QString Ipv6Widget::getIpv6RoutingPrefix(IPv6Network &net) {
    if(net.CIDR() == 128)
        return tr("N/A\n(Host Route)");
    return QString::fromStdString(net.routingPrefix().toString());
}

QString Ipv6Widget::getIpv6RangeString(IPv6Network &net) {
    if(net.CIDR() >= 127)
        return QString::fromStdString(net.host(0).toString());
    return QString::fromStdString(net.host(0).toString()
              + "\n-\n"
              + net.host(net.hostCount() - 1).toString());
}


void Ipv6Widget::update(bool updateTableContents) {
    updateSubnetCountRange();
    if(!updateTableContents) return;

    uint64_t subnetCount = 1;
    for(uint_fast8_t i = 0; i < ui->subnetCountInput->currentIndex(); ++i)
        subnetCount *= 2;

    try {
        net = IPv6Network(
            IPv6Address(ui->addressInput->text().toStdString()),
            ui->cidrInput->value()
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

    QTreeWidgetItem *rootItem = new QTreeWidgetItem(*getItemTemplate());
    ui->subnetsTable->addTopLevelItem(rootItem);
    rootItem->setText(1, getIpv6RoutingPrefix(net));
    rootItem->setText(2, getIpv6RangeString(net));
    rootItem->setText(3, "/" + QString::number(net.CIDR()));
    rootItem->setText(4, QString::fromStdString(
        uint128toStdString(net.hostCount())
    ));

    if(subnetCount > 1) {
        for(int i = 0; i < subnets.size(); ++i) {
            QTreeWidgetItem *item = new QTreeWidgetItem(*getItemTemplate());
            rootItem->addChild(item);

            item->setText(0, QString::number(i + 1));
            item->setText(1, getIpv6RoutingPrefix(subnets[i]));
            item->setText(2, getIpv6RangeString(subnets[i]));
            item->setText(3, "/" + QString::number(subnets[i].CIDR()));
            item->setText(4, QString::fromStdString(
                uint128toStdString(subnets.at(i).hostCount())
            ));
        }
    }
    setSaveTableAvaliable(true);
}


void Ipv6Widget::updateSubnetCountRange() {
    ui->subnetCountInput->blockSignals(true);

    int currentSubnetCountIndex = ui->subnetCountInput->currentIndex();
    ui->subnetCountInput->clear();

    if(ui->addressInput->hasAcceptableInput()) {
        uint_fast8_t cidr = ui->cidrInput->value();
        uint128 ipCount = 1;
        for(uint_fast8_t i = 0; i < 128 - cidr - 1; ++i, ipCount *= 2) {
            ui->subnetCountInput->addItem(
                QString::fromStdString(uint128toStdString(ipCount))
                + " (2^" + QString::number(i) + ")"
            );
        }
    }

    if(ui->subnetCountInput->count() == 0)
        ui->subnetCountInput->addItem("1 (2^0)");

    ui->subnetCountInput->setCurrentIndex(
        qMin(currentSubnetCountIndex, ui->subnetCountInput->count() - 1)
    );

    ui->subnetCountInput->blockSignals(false);
}
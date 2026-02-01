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

#include "Ipv4Widget.hpp"
#include "ui_AbstractIpWidget.h"

#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMessageBox>

#include "IpInputEventFilter.hpp"

#define IPV4_REGEX "^((25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)(\\.(?!$)|$)){3}(25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)$"

using namespace libSubnetCalculator;

Ipv4Widget::Ipv4Widget(QWidget* parent) :
    lastIpv4Class((IPv4Address::Class)-1),
    AbstractIpWidget(parent)
{
    ui->subnetsTable->setHeaderLabels(QStringList()
        << tr("No.")
        << tr("Network Address")
        << tr("Range")
        << tr("Broadcast Address")
        << tr("Subnet Mask")
        << tr("Host Count")
    );

    ui->addressInput->setValidator(
        new QRegularExpressionValidator(QRegularExpression(IPV4_REGEX), this)
    );
    
    inputFilter = new IpInputEventFilter(ui->cidrInput, [this]() -> void {
        ui->addressInput->insert(".");

        const QValidator *validator = ui->addressInput->validator();
        QString str = ui->addressInput->text();
        int pos = 0;
        if(validator->validate(str, pos) == QValidator::Acceptable) {
            ui->cidrInput->setFocus();
            ui->cidrInput->selectAll();
        }
    });
    ui->addressInput->installEventFilter(inputFilter);
    ui->addressInput->setPlaceholderText("10.20.30.40");
}

Ipv4Widget::~Ipv4Widget(){
    inputFilter->deleteLater();
    inputFilter = nullptr;
}


QString Ipv4Widget::getNetworkAddressString(IPv4Network &net) {
    switch (net.CIDR()){
    case 31:
        return tr("N/A\nPoint-to-Point Link\n(RFC 3021)");
    case 32:
        return tr("N/A\n(Host Route)");
    default:
        return QString::fromStdString(net.networkAddress().toString());
    }
}

QString Ipv4Widget::getIpv4RangeString(IPv4Network &net) {
    if(net.CIDR() == 32)
        return QString::fromStdString(net.host(0).toString());
    return QString::fromStdString(net.host(0).toString()
              + "\n-\n"
              + net.host(net.hostCount() - 1).toString());
}

QString Ipv4Widget::getBroadcastAddressString(IPv4Network &net) {
    switch (net.CIDR()){
    case 32:
        return tr("N/A\n(Host Route)");
    case 31:
        return tr("N/A\nPoint-to-Point Link\n(RFC 3021)");
    default:
        return QString::fromStdString(net.broadcastAddress().toString());
    }
}

QString Ipv4Widget::getMaskString(IPv4Network &net) {
    return QString::fromStdString(net.subnetMask().toString())
        + "\n(/" + QString::number(net.CIDR()) + ")";
}


void Ipv4Widget::update(bool updateTableContents) {
    updateIpv4AddressClassToolTip();
    updateSubnetCountRange();
    if(!updateTableContents) return;

    uint32_t subnetCount = qMax<uint32_t>(pow(2, ui->subnetCountInput->currentIndex()), 1);
    try {
        net = IPv4Network(
            IPv4Address(ui->addressInput->text().toStdString()),
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

    rootItem->setText(1, getNetworkAddressString(net));
    rootItem->setText(2, getIpv4RangeString(net));
    rootItem->setText(3, getBroadcastAddressString(net));
    rootItem->setText(4, getMaskString(net));
    rootItem->setText(5, QString::number(net.hostCount()));

    if(subnetCount > 1) {
        for(int i = 0; i < subnets.size(); ++i) {
            QTreeWidgetItem *item = new QTreeWidgetItem(*getItemTemplate());
            rootItem->addChild(item);

            item->setText(0, QString::number(i + 1));
            item->setText(1, getNetworkAddressString(subnets.at(i)));
            item->setText(2, getIpv4RangeString(subnets.at(i)));
            item->setText(3, getBroadcastAddressString(subnets.at(i)));
            item->setText(4, getMaskString(subnets.at(i)));
            item->setText(5, QString::number(subnets.at(i).hostCount()));
        }
    }
    setSaveTableAvaliable(true);
}

void Ipv4Widget::updateSubnetCountRange() {
    ui->subnetCountInput->blockSignals(true);

    int currentSubnetCountIndex = ui->subnetCountInput->currentIndex();
    ui->subnetCountInput->clear();
    ui->subnetCountInput->addItem(QString::number(1));

    if(ui->addressInput->hasAcceptableInput()) {
        uint32_t ipCount = pow(2, 32 - ui->cidrInput->value() - 1);
        for(uint32_t i = 2; i < ipCount; i *= 2)
            ui->subnetCountInput->addItem(QString::number(i));
    }

    ui->subnetCountInput->setCurrentIndex(
        qMin(currentSubnetCountIndex, ui->subnetCountInput->count() - 1)
    );

    ui->subnetCountInput->blockSignals(false);
}

void Ipv4Widget::updateIpv4AddressClassToolTip() {
    IPv4Address::Class ipv4Class;
    try {
        ipv4Class = IPv4Address(ui->addressInput->text().toStdString()).getClass();
    }
    catch(InvalidAddressException& e) {
        ui->addressInput->setToolTip("");
        lastIpv4Class = (IPv4Address::Class)-1;
        return;
    }
    catch(libSubnetCalculatorException& e) {
        return;
    }

    if(lastIpv4Class == ipv4Class)
        return;
    lastIpv4Class = ipv4Class;

    ui->cidrInput->blockSignals(true);
    switch(ipv4Class) {
    case IPv4Address::Class::A:
        ui->addressInput->setToolTip(tr("Class A"));
        ui->cidrInput->setValue(8);
        break;
    case IPv4Address::Class::B:
        ui->addressInput->setToolTip(tr("Class B"));
        ui->cidrInput->setValue(16);
        break;
    case IPv4Address::Class::C:
        ui->addressInput->setToolTip(tr("Class C"));
        ui->cidrInput->setValue(24);
        break;
    case IPv4Address::Class::D:
        ui->addressInput->setToolTip(tr("Class D"));
        ui->cidrInput->setValue(4);
        break;
    case IPv4Address::Class::E:
        ui->addressInput->setToolTip(tr("Class E"));
        ui->cidrInput->setValue(4);
        break;
    }
    ui->cidrInput->blockSignals(false);
}
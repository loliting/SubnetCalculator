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

#ifndef IPV4WIDGET_HPP
#define IPV4WIDGET_HPP

#include <QtWidgets/QWidget>

#include <libSubnetCalculator.hpp>

#include "AbstractIpWidget.hpp"

class IpInputEventFilter;

class Ipv4Widget : public AbstractIpWidget
{
    Q_OBJECT
public:
    explicit Ipv4Widget(QWidget *parent = nullptr);
    ~Ipv4Widget();
public slots:
    void update(bool updateTableContents) override;
private slots:
    void updateSubnetCountRange();
    void updateIpv4AddressClassToolTip();
private:
    QString getNetworkAddressString(libSubnetCalculator::IPv4Network&);
    QString getIpv4RangeString(libSubnetCalculator::IPv4Network&);
    QString getBroadcastAddressString(libSubnetCalculator::IPv4Network&);
    QString getMaskString(libSubnetCalculator::IPv4Network&);
private:
    IpInputEventFilter *inputFilter;

    libSubnetCalculator::IPv4Address::Class lastIpv4Class;
    libSubnetCalculator::IPv4Network net;
    std::vector<libSubnetCalculator::IPv4Network> subnets;
};

#endif // IPV4WIDGET_HPP
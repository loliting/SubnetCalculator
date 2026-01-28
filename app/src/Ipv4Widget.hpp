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

#ifndef IPV4WIDGET_HPP
#define IPV4WIDGET_HPP

#include <QtCore/QObject>
#include <QtWidgets/QWidget>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLineEdit>

#include <libSubnetCalculator.hpp>

namespace Ui {
class Ipv4Widget;
}
class IpInputEventFilter;

class Ipv4Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Ipv4Widget(QWidget *parent = nullptr);
    ~Ipv4Widget();
signals:
    void saveTableAvaliable(bool);
public slots:
    void update();
    bool canSaveTable();
    void saveTable();
private slots:
    void updateSubnetCountRange();
    void updateIpv4AddressClassTooltip();
    void updateTableContent();

    void on_ipv4Address_textEdited(const QString);
    void on_cidr_valueChanged(int);
    void on_calculate_clicked();
    void on_subnetCount_currentIndexChanged(int index);

private:
    Ui::Ipv4Widget *ui;
    IpInputEventFilter *inputFilter = nullptr;

    libSubnetCalculator::IPv4Address::Class lastIpv4Class;
    libSubnetCalculator::IPv4Network net;
    std::vector<libSubnetCalculator::IPv4Network> subnets;

    bool isSaveTableAvaliable = false;
};

#endif // IPV4WIDGET_HPP
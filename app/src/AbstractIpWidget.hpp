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

#ifndef ABSTRACTIPWIDGET_HPP
#define ABSTRACTIPWIDGET_HPP

#include <QtWidgets/QWidget>

namespace Ui {
    class AbstractIpWidget;
}

class QTreeWidgetItem;

class AbstractIpWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AbstractIpWidget(QWidget *parent = nullptr);
    ~AbstractIpWidget();

    bool canSaveTable();
signals:
    void saveTableAvaliable(bool);
public slots:
    virtual void update(bool updateTableContents) = 0;
    void saveAsCsvTable();
    bool copy();
    void cut();
    void paste();
protected slots:
    void setSaveTableAvaliable(bool);
private slots:
    void _update(bool buttonSender);
protected:
    QTreeWidgetItem* const getItemTemplate() const { return itemTemplate; }
protected:
    Ui::AbstractIpWidget *ui;
private:
    bool isSaveTableAvaliable = false;
    QTreeWidgetItem* itemTemplate = nullptr;
};

#endif // ABSTRACTIPWIDGET_HPP
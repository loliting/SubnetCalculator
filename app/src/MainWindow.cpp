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

#include "MainWindow.hpp"
#include "ui_MainWindow.h"

#include "Ipv4Widget.hpp"
#include "Ipv6Widget.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowIcon(QIcon(":/icon.png"));

    ui->actionSave_Table_As->setDisabled(true);
    connect(ui->ipv4, &Ipv4Widget::saveTableAvaliable,
        this, &MainWindow::updateActionSaveTableState
    );
    connect(ui->ipv6, &Ipv6Widget::saveTableAvaliable,
        this, &MainWindow::updateActionSaveTableState
    );

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateActionSaveTableState() {
    if(ui->ipv4->isVisible())
        ui->actionSave_Table_As->setEnabled(ui->ipv4->canSaveTable());
    else
        ui->actionSave_Table_As->setEnabled(ui->ipv6->canSaveTable());
}

void MainWindow::on_tabWidget_currentChanged(int index) {
    updateActionSaveTableState();
}

void MainWindow::on_actionSave_Table_As_triggered() {
    if(ui->ipv4->isVisible())
        ui->ipv4->saveTable();
    else
        ui->ipv6->saveTable();
}
#include "SaveAsDialog.hpp"
#include "ui_SaveAsDialog.h"

SaveAsDialog::SaveAsDialog(bool hasSubnets, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveAsDialog)
{
    ui->setupUi(this);

    QStringList encodingStrings(ENCODING_MAX);
    encodingStrings[UTF8_BOM] = "UTF-8 BOM";
    encodingStrings[UTF8] = "UTF-8";
    encodingStrings[UTF16BE_BOM] = "UTF-16BE BOM";
    encodingStrings[UTF16BE] = "UTF-16BE";
    encodingStrings[UTF16LE_BOM] = "UTF-16LE BOM";
    encodingStrings[UTF16LE] = "UTF-16LE";
    encodingStrings[UTF32BE_BOM] = "UTF-32BE BOM";
    encodingStrings[UTF32BE] = "UTF-32E";
    encodingStrings[UTF32LE_BOM] = "UTF-32LE BOM";
    encodingStrings[UTF32LE] = "UTF-32LE";
    ui->encoding->addItems(encodingStrings);

    if(hasSubnets) {
        ui->saveRootNet->setEnabled(true);
        ui->saveSubnetsSeperator->setEnabled(true);
        ui->saveSubnetsSeperator->setChecked(true);
    }
}

SaveAsDialog::~SaveAsDialog() {
    delete ui;
}

int SaveAsDialog::exec() {
    int rc = QDialog::exec();
    if(QDialog::result() == QDialog::Rejected) {
        m_result = { };
        return rc;
    }

    QString delimiter, separator;
    if(ui->delimiterSingleQuote->isChecked())
        delimiter = "\'";
    if(ui->delimiterOtherRbtn->isChecked() && ui->delimiterOther->text().length() > 0)
        delimiter = ui->delimiterOther->text();
    else // default
        delimiter = "\"";

    if(ui->seperatorSemicolon->isChecked())
        separator = ";";
    else if(ui->seperatorSpace->isChecked())
        separator = " ";
    else if(ui->seperatorRbtn->isChecked() && ui->seperatorOther->text().length() > 0)
        separator = ui->seperatorOther->text();
    else //default
        separator = ",";


    SaveAsOptions options {
        .encoding = CsvEncoding(ui->encoding->currentIndex()),
        .delimiter = delimiter,
        .separator = separator,

        .saveHeaderNames = ui->saveHeaderNames->isChecked(),
        .saveRootNet = ui->saveRootNet->isChecked(),
        .saveSubnetsSeparator = ui->saveSubnetsSeperator->isChecked(),
    };

    m_result = options;
    return rc;
}
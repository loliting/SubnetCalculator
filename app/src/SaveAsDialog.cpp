#include "SaveAsDialog.hpp"
#include "ui_SaveAsDialog.h"

SaveAsDialog::SaveAsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveAsDialog)
{
    ui->setupUi(this);

    QStringList encodingStrings(6);
    encodingStrings[UTF8_BOM] = "UTF-8 BOM";
    encodingStrings[UTF8] = "UTF-8";
    encodingStrings[UTF_16BE_BOM] = "UTF-16BE BOM";
    encodingStrings[UTF_16BE] = "UTF-16BE";
    encodingStrings[UTF_16LE_BOM] = "UTF-16LE BOM";
    encodingStrings[UTF_16LE] = "UTF-16LE";
    ui->encoding->addItems(encodingStrings);
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
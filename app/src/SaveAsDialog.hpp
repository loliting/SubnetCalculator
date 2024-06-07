#ifndef SAVEASDIALOG_HPP
#define SAVEASDIALOG_HPP

#include <QDialog>

#include <optional>

namespace Ui {
class SaveAsDialog;
}

enum CsvEncoding {
    /* BOM encodings should have uneven indices */
    BOM_FLAG = 1,

    UTF8 = 0,
    UTF8_BOM = UTF8 | BOM_FLAG,
    UTF16BE = 2,
    UTF16BE_BOM = UTF16BE | BOM_FLAG,
    UTF16LE = 4,
    UTF16LE_BOM = UTF16LE | BOM_FLAG,
    UTF32BE = 6,
    UTF32BE_BOM = UTF32BE | BOM_FLAG,
    UTF32LE = 8,
    UTF32LE_BOM = UTF32LE | BOM_FLAG,


    ENCODING_MAX
};

struct SaveAsOptions
{
public:
    CsvEncoding encoding;
    QString separator;
    QString delimiter;

    bool saveHeaderNames;
    bool saveRootNet;
    bool saveSubnetsSeparator;
};

class SaveAsDialog : public QDialog
{
    Q_OBJECT
public:

    explicit SaveAsDialog(bool hasSubnets, QWidget *parent = nullptr);
    ~SaveAsDialog();

    int exec();
    std::optional<SaveAsOptions> result() const { return m_result; }
private:
    std::optional<SaveAsOptions> m_result;

    Ui::SaveAsDialog *ui;
};

#endif // SAVEASDIALOG_HPP

#ifndef SAVEASDIALOG_HPP
#define SAVEASDIALOG_HPP

#include <QDialog>

#include <optional>

namespace Ui {
class SaveAsDialog;
}

enum CsvEncoding {
    UTF8_BOM,
    UTF8 ,
    UTF_16BE_BOM,
    UTF_16BE,
    UTF_16LE_BOM,
    UTF_16LE,


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

    explicit SaveAsDialog(QWidget *parent = nullptr);
    ~SaveAsDialog();

    int exec();
    std::optional<SaveAsOptions> result() const { return m_result; }
private:
    std::optional<SaveAsOptions> m_result;

    Ui::SaveAsDialog *ui;
};

#endif // SAVEASDIALOG_HPP

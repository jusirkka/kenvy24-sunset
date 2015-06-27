#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>

namespace Ui {
class RenameDialog;
}

class RenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenameDialog(QWidget *parent = 0, const QString& text = QString());
    ~RenameDialog();

    const QString& name() const {return mName;}

public slots:

    void on_lineEdit_textEdited(const QString& text);

private:
    Ui::RenameDialog *mUI;
    QString mName;
};

#endif // NEWDIALOG_H

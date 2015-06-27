#include "renamedialog.h"
#include "ui_renamedialog.h"

RenameDialog::RenameDialog(QWidget *parent, const QString& text):
    QDialog(parent),
    mUI(new Ui::RenameDialog),
    mName(text)
{
    mUI->setupUi(this);
    mUI->lineEdit->setText(text);
    mUI->lineEdit->selectAll();
    setResult(QDialog::Rejected);
}

RenameDialog::~RenameDialog()
{
    delete mUI;
}

void RenameDialog::on_lineEdit_textEdited(const QString &text) {
    mName = text;
}

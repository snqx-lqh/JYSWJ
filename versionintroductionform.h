#ifndef VERSIONINTRODUCTIONFORM_H
#define VERSIONINTRODUCTIONFORM_H

#include <QWidget>

namespace Ui {
class VersionIntroductionForm;
}

class VersionIntroductionForm : public QWidget
{
    Q_OBJECT

public:
    explicit VersionIntroductionForm(QWidget *parent = nullptr);
    ~VersionIntroductionForm();

private:
    Ui::VersionIntroductionForm *ui;
};

#endif // VERSIONINTRODUCTIONFORM_H

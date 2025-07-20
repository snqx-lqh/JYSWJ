#ifndef INTRODUCEFORM_H
#define INTRODUCEFORM_H

#include <QWidget>

namespace Ui {
class IntroduceForm;
}

class IntroduceForm : public QWidget
{
    Q_OBJECT

public:
    explicit IntroduceForm(QWidget *parent = nullptr);
    ~IntroduceForm();

private:
    Ui::IntroduceForm *ui;
};

#endif // INTRODUCEFORM_H

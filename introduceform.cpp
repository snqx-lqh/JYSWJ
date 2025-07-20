#include "introduceform.h"
#include "ui_introduceform.h"

IntroduceForm::IntroduceForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IntroduceForm)
{
    ui->setupUi(this);
}

IntroduceForm::~IntroduceForm()
{
    delete ui;
}

#include "versionintroductionform.h"
#include "ui_versionintroductionform.h"

VersionIntroductionForm::VersionIntroductionForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VersionIntroductionForm)
{
    ui->setupUi(this);
    setWindowTitle("软件简介");
}

VersionIntroductionForm::~VersionIntroductionForm()
{
    delete ui;
}

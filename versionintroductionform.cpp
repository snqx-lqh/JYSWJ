#include "versionintroductionform.h"
#include "ui_versionintroductionform.h"

VersionIntroductionForm::VersionIntroductionForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VersionIntroductionForm)
{
    ui->setupUi(this);
}

VersionIntroductionForm::~VersionIntroductionForm()
{
    delete ui;
}

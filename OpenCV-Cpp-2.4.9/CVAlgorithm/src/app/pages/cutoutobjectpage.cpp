#include "cutoutobjectpage.h"
#include <QLabel>

CutoutObjectPage::CutoutObjectPage(QWidget *parent)
    : QWidget{parent}
{
    QLabel* label = new QLabel("CutoutObjectPage", this);
}

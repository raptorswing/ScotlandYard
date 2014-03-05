#include "DetectiveInput.h"
#include "ui_DetectiveInput.h"

#include <QRegExpValidator>

DetectiveInput::DetectiveInput(int moveNumber, const QString &ticketType, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DetectiveInput)
{
    ui->setupUi(this);

    ui->numberLabel->setText(QString::number(moveNumber));
    ui->ticketLabel->setText(ticketType);

    {
        QRegExpValidator * validator = new QRegExpValidator(QRegExp("[0-9]+(?:, ?[0-9]+)*"),
                                                            ui->detectivePosEntry);
        ui->detectivePosEntry->setValidator(validator);
    }
}

DetectiveInput::~DetectiveInput()
{
    delete ui;
}

QSet<int> DetectiveInput::detectivePositions() const
{
    QSet<int> toRet;
    QStringList list = ui->detectivePosEntry->text().split(",");

    foreach(QString item, list)
    {
        item = item.trimmed();
        item.remove(QRegExp("^0*"));
        toRet.insert(item.toInt());
    }

    return toRet;
}

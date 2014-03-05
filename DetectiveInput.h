#ifndef DETECTIVEINPUT_H
#define DETECTIVEINPUT_H

#include <QWidget>
#include <QSet>

namespace Ui {
class DetectiveInput;
}

class DetectiveInput : public QWidget
{
    Q_OBJECT

public:
    explicit DetectiveInput(int moveNumber, const QString& ticketType, QWidget *parent = 0);
    ~DetectiveInput();

    QSet<int> detectivePositions() const;

private:
    Ui::DetectiveInput *ui;
};

#endif // DETECTIVEINPUT_H

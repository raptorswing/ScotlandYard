#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QSet>

#include "DetectiveInput.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_taxiButton_clicked();

    void on_busButton_clicked();

    void on_trainButton_clicked();

    void on_blackButton_clicked();

    void on_resetButton_clicked();

    void on_calculateButton_clicked();

    void on_ticketEntry_textChanged(const QString &arg1);

    void on_lastLocationEntry_textChanged(const QString &arg1);

    void updateGUIState();

private:
    void appendTicket(const QString& type);
    void initializeGraph();
    QSet<QString> getConnections(const QString& node, const QString& ticketType);

    QList<DetectiveInput *> _detectiveInputs;

    Ui::MainWindow *ui;

    QJsonObject _graph;

};

#endif // MAINWINDOW_H

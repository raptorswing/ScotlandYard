#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QStringBuilder>
#include <QtDebug>
#include <QIntValidator>
#include <QRegExpValidator>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QMap>
#include <QQueue>
#include <QMapIterator>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Validator for "last location" entry
    {
        QIntValidator * validator = new QIntValidator(1,199, ui->lastLocationEntry);
        ui->lastLocationEntry->setValidator(validator);
    }

    //Validator for "tickets" entry
    {
        QRegExp exp("(?:taxi|bus|train|black)(?:\\, ?taxi|\\, ?bus|\\, ?train|\\, ?black)*");
        QRegExpValidator * validator = new QRegExpValidator(exp, ui->ticketEntry);
        ui->ticketEntry->setValidator(validator);
    }

    this->updateGUIState();

    this->initializeGraph();
}

MainWindow::~MainWindow()
{
    delete ui;
}

//private slot
void MainWindow::on_taxiButton_clicked()
{
    this->appendTicket("taxi");
}

//private slot
void MainWindow::on_busButton_clicked()
{
    this->appendTicket("bus");
}

//private slot
void MainWindow::on_trainButton_clicked()
{
    this->appendTicket("train");
}

//private slot
void MainWindow::on_blackButton_clicked()
{
    this->appendTicket("black");
}

//private slot
void MainWindow::on_resetButton_clicked()
{
    ui->ticketEntry->clear();
}

//private slot
void MainWindow::on_calculateButton_clicked()
{
    if (ui->lastLocationEntry->text().isEmpty() || ui->ticketEntry->text().isEmpty())
    {
        QMessageBox::information(this, "Whoops", "We need more information than that to do any analysis! Try again!");
        return;
    }

    //The user COULD put some zeroes in front of the numbers and it would sneak past the validator.
    //Let's nip that in the bud, shall we?
    QString lastLocation = ui->lastLocationEntry->text();
    lastLocation.remove(QRegExp("^0*"));

    //Used to store the current places Mr. X could be at each step of the calculation
    QMap<QString, int> frontier;
    frontier.insert(lastLocation, 1);

    //Queue of tickets to process
    QQueue<QString> tickets;

    {
        QStringList stubs = ui->ticketEntry->text().split(",");
        foreach(const QString& stub, stubs)
            tickets.enqueue(stub.trimmed());
    }

    while (!tickets.isEmpty())
    {
        const QString ticket = tickets.dequeue();

        //We'll calculate a new frontier
        QMap<QString, int> newFrontier;

        foreach(const QString& frontierNode, frontier.keys())
        {
            QSet<QString> connections = this->getConnections(frontierNode, ticket);
            if (ticket == "black")
            {
                connections.unite(this->getConnections(frontierNode, "taxi"));
                connections.unite(this->getConnections(frontierNode, "bus"));
                connections.unite(this->getConnections(frontierNode, "train"));
            }

            foreach(const QString& conn, connections)
            {
                if (newFrontier.contains(conn))
                    newFrontier[conn] = newFrontier[conn] + 1;
                else
                    newFrontier[conn] = 1;
            }
        }
        frontier = newFrontier;
    }

    int total = 0;
    foreach (int count, frontier.values())
        total += count;

    QMultiMap<qreal, QString> probSorted;

    QMapIterator<QString, int> resultIter(frontier);
    while (resultIter.hasNext())
    {
        resultIter.next();

        const qreal prob = (qreal)resultIter.value() / (qreal)total * 100;
        probSorted.insert(prob, resultIter.key());
    }

    QString resultString = QString("Position").leftJustified(20) % "Probability";

    QMapIterator<qreal, QString> sortedIterator(probSorted);
    sortedIterator.toBack();
    while (sortedIterator.hasPrevious())
    {
        sortedIterator.previous();
        const qreal prob = sortedIterator.key();
        const QString& val = sortedIterator.value();

        resultString += "\n" % val.leftJustified(20) % QString::number(prob);
    }

    ui->resultTextEdit->clear();
    ui->resultTextEdit->setPlainText(resultString);
}

//private slot
void MainWindow::on_ticketEntry_textChanged(const QString &)
{
    this->updateGUIState();
}

//private slot
void MainWindow::on_lastLocationEntry_textChanged(const QString &)
{
    this->updateGUIState();
}

//private slot
void MainWindow::updateGUIState()
{
    bool validTicket = false;
    {
        QString tempString = ui->ticketEntry->text();
        int tempInt = 0;
        validTicket = (ui->ticketEntry->validator()->validate(tempString, tempInt) == QValidator::Acceptable);
    }
    ui->calculateButton->setEnabled(validTicket && !ui->lastLocationEntry->text().isEmpty());
}

//private
void MainWindow::appendTicket(const QString &type)
{
    QString current = ui->ticketEntry->text();
    if (!current.isEmpty())
        current.append(", ");
    current.append(type);

    int temp = 0;
    if (ui->ticketEntry->validator()->validate(current, temp) == QValidator::Acceptable)
        ui->ticketEntry->setText(current);
}

//private
void MainWindow::initializeGraph()
{
    //Try to read the file
    QFile fp(":/graphs/graph.json");
    if (!fp.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, "Error!", "Failed to open graph. Analysis will not work.");
        ui->calculateButton->setEnabled(false);
        return;
    }

    //Try to parse the data
    QByteArray data = fp.readAll();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);

    if (err.error != QJsonParseError::NoError || !doc.isObject())
    {
        QMessageBox::warning(this, "Error!", "Failed to parse graph file. Analysis will not work");
        qWarning() << err.errorString() << err.offset;
        ui->calculateButton->setEnabled(false);
        return;
    }

    _graph = doc.object();


    QSet<QString> modes;
    modes.insert("taxi");
    modes.insert("bus");
    modes.insert("train");
    modes.insert("black");

    //Do some basic tests to make sure the graph is reasonable
    for (int i = 1; i < 199; i++)
    {
        const QString me = QString::number(i);

        foreach(const QString& mode, modes)
        {
            QSet<QString> connections = this->getConnections(me, mode);
            if (connections.contains(me))
                qWarning() << me << "has reflexive relation on" << mode;
            foreach(const QString& them, connections)
            {
                QSet<QString> backConnections = this->getConnections(them, mode);
                if (!backConnections.contains(me))
                    qWarning() << mode << "connection missing?" << them << "to" << me;
            }
        }
    }
}

//private
QSet<QString> MainWindow::getConnections(const QString &node, const QString &ticketType)
{
    QSet<QString> toRet;

    QJsonValue nodeInfo = _graph.value(node);
    if (!nodeInfo.isObject())
    {
        qWarning() << "Undefined node" << node;
        return toRet;
    }

    QJsonValue nodeLinks = nodeInfo.toObject().value(ticketType);
    if (!nodeLinks.isArray())
    {
        //qWarning() << "Bad transport mode" << ticketType << "for node" << node;
        return toRet;
    }

    foreach(const QVariant& variant, nodeLinks.toArray().toVariantList())
        toRet.insert(variant.toString());

    return toRet;
}

#include <QApplication>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProcess>
#include <QThread>
#include <QMessageBox>
#include <QLabel>
#include <QTimeEdit>
#include <QPushButton>

int main(int argc, char **argv)
{
    constexpr quint64 SecondsInMinute = 60;
    constexpr quint64 SecondsInHour = SecondsInMinute * 60;
    constexpr quint64 SecondsInDay = SecondsInHour * 24;

    QApplication app(argc, argv);
    QMainWindow window;
    QWidget centralWidget;
    QVBoxLayout mainLayout;
    QHBoxLayout methodLayout;
    QProcess process;
    const QString command = "shutdown";
    QPushButton abortButton("Abort shutdown");
    auto abortShutdown = [&]()
    {
        QStringList arguments;
        QMessageBox waitBox;
        waitBox.setText("Aborting shutdown\nplease wait...");
        waitBox.setWindowTitle("Information");
        waitBox.setIcon(QMessageBox::Information);
        waitBox.setStandardButtons(QMessageBox::NoButton);
        waitBox.show();
        arguments << "/a";
        QApplication::processEvents();
        qDebug() << "executing:" << command << arguments;

        process.start(command, arguments);

        if (!process.waitForStarted())
        {
            waitBox.hide();
            QMessageBox::critical(nullptr, "Error!", "Failed to start abort shutdown!");
        }
        else
        {
            waitBox.hide();
            if (!process.waitForFinished())
            {
                QMessageBox::critical(nullptr, "Error!", "Shutdown abort command did not finish properly!");
            }
            else
            {
                auto exitCode = process.exitCode();
                if (!exitCode)
                {
                    QString text = "Succesfully aborted shutdown!";
                    QMessageBox::information(nullptr, "Success!", text);
                }
                else if (exitCode == 1116)
                {
                    QString text = "No shutdown was in progress!";
                    QMessageBox::information(nullptr, "Success!", text);
                }
                else
                {
                    QString text = "Abort shutdown command failed with exit code: " + QString::number(exitCode) + "!";
                    QMessageBox::critical(nullptr, "Error!", text);
                }
            }
        }
    };
    QObject::connect(&abortButton, &QPushButton::released, abortShutdown);

    auto executeShutdown = [&](quint64 secs)
    {
        QStringList arguments;
        QTime shutdownTime = QTime::currentTime().addSecs(secs);
        QString shutdownClock = QString("%1:%2").arg(shutdownTime.hour(), 2, 10, QChar('0')).arg(shutdownTime.minute(), 2, 10, QChar('0'));
        QMessageBox waitBox;
        waitBox.setText("Scheduling shutdown at " + shutdownClock + "\nplease wait...");
        waitBox.setWindowTitle("Information");
        waitBox.setIcon(QMessageBox::Information);
        waitBox.setStandardButtons(QMessageBox::NoButton);
        waitBox.show();
        QApplication::processEvents();
        arguments << "/s" << "/f" << "/t" << QString::number(secs);

        qDebug() << "executing:" << command << arguments;

        process.start(command, arguments);

        if (!process.waitForStarted())
        {
            waitBox.hide();
            QMessageBox::critical(nullptr, "Error!", "Failed to start shutdown command!");
        }
        else
        {
            waitBox.hide();
            if (!process.waitForFinished())
            {
                QMessageBox::critical(nullptr, "Error!", "Shutdown command did not finish properly!");
            }
            else
            {
                auto exitCode = process.exitCode();
                if (!exitCode)
                {
                    QString text = "PC will shutdown at " + shutdownClock + "!";
                    QMessageBox::information(nullptr, "Success!", text);
                }
                else if (exitCode == 1190)
                {
                    QString text = "Shutdown already scheduled!";
                    QMessageBox::critical(nullptr, "Error!", text);
                }
                else
                {
                    QString text = "Shutdown command failed with exit code: " + QString::number(exitCode) + "!";
                    QMessageBox::critical(nullptr, "Error!", text);
                }
            }
        }
    };

    QVBoxLayout afterTimeLayout;
    QLabel afterTimeLabel("Will shutdown after:");
    QTimeEdit afterTimeEdit(QTime(2, 0, 0));
    QPushButton afterTimeButton("Schedule a shutdown after a specific time");
    auto secondsAction = [&]()
    {
        quint64 secs = afterTimeEdit.time().hour() * SecondsInHour + afterTimeEdit.time().minute() * SecondsInMinute;
        executeShutdown(secs);
    };
    QObject::connect(&afterTimeButton, &QPushButton::released, secondsAction);

    QVBoxLayout forTimeLayout;
    QLabel forTimeLabel("Will shutdown at:");
    QTimeEdit forTimeEdit;
    QPushButton forTimeButton("Schedule a shutdown for a specific time");
    auto clockAction = [&]()
    {
        auto secs = QTime::currentTime().secsTo(forTimeEdit.time());
        if (secs < 0)
            secs += SecondsInDay;
        executeShutdown(secs);
    };
    QObject::connect(&forTimeButton, &QPushButton::released, clockAction);

    window.setFixedSize(600, 120);
    window.setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);
    window.setCentralWidget(&centralWidget);

    centralWidget.setLayout(&mainLayout);

    mainLayout.addLayout(&methodLayout);

    methodLayout.addLayout(&afterTimeLayout, 1);
    methodLayout.addLayout(&forTimeLayout, 1);

    afterTimeLayout.addWidget(&afterTimeLabel);
    afterTimeLabel.setAlignment(Qt::AlignCenter | Qt::AlignBottom);
    afterTimeLayout.addWidget(&afterTimeEdit);
    afterTimeLayout.addWidget(&afterTimeButton);

    forTimeLayout.addWidget(&forTimeLabel);
    forTimeLabel.setAlignment(Qt::AlignCenter | Qt::AlignBottom);
    forTimeLayout.addWidget(&forTimeEdit);
    forTimeLayout.addWidget(&forTimeButton);

    mainLayout.addWidget(&abortButton);

    window.show();

    return app.exec();
}
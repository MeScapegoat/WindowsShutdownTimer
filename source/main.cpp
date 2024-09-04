#include <QApplication>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QProcess>
#include <QThread>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QTimeEdit>
#include <QPushButton>

int main(int argc, char **argv)
{
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

    QVBoxLayout secondsLayout;
    QLabel secondsLabel("Enter seconds:");
    QLineEdit seconds("60");
    QIntValidator intValidator(0, 60 * 60 * 24);
    seconds.setValidator(&intValidator);
    QPushButton secondsButton("Schedule shutdown by seconds");
    auto secondsAction = [&]()
    {
        auto secs = seconds.text();
        secs.remove(' ');
        secs.remove(QChar(0x00A0));
        executeShutdown(secs.toULongLong());
    };
    QObject::connect(&secondsButton, &QPushButton::released, secondsAction);

    QVBoxLayout clockLayout;
    QLabel clockLabel("Choose time:");
    QTimeEdit clock;
    QPushButton clockButton("Schedule shutdown by time");
    auto clockAction = [&]()
    {
        auto secs = QTime::currentTime().secsTo(clock.time());
        if (secs < 0)
            secs += 60 * 60 * 24;
        executeShutdown(secs);
    };
    QObject::connect(&clockButton, &QPushButton::released, clockAction);

    window.setFixedSize(600, 120);
    window.setWindowFlags(Qt::Window | Qt::MSWindowsFixedSizeDialogHint);
    window.setCentralWidget(&centralWidget);

    centralWidget.setLayout(&mainLayout);

    mainLayout.addLayout(&methodLayout);

    methodLayout.addLayout(&secondsLayout, 1);
    methodLayout.addLayout(&clockLayout, 1);

    secondsLayout.addWidget(&secondsLabel);
    secondsLabel.setAlignment(Qt::AlignCenter | Qt::AlignBottom);
    secondsLayout.addWidget(&seconds);
    secondsLayout.addWidget(&secondsButton);

    clockLayout.addWidget(&clockLabel);
    clockLabel.setAlignment(Qt::AlignCenter | Qt::AlignBottom);
    clockLayout.addWidget(&clock);
    clockLayout.addWidget(&clockButton);

    mainLayout.addWidget(&abortButton);

    window.show();

    return app.exec();
}
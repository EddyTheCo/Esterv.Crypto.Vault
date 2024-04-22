#include "vault.hpp"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("VaultCli");

    QCommandLineParser parser;
    parser.setApplicationDescription("Get and set data from the Vault file. Only one mode can be set [Set|Get].");
    parser.addHelpOption();
    parser.addPositionalArgument("data",
                                 QCoreApplication::translate("main", "data to store on the vault. Only on Set Mode."));
    parser.addPositionalArgument("pass", QCoreApplication::translate("main", "password of the vault."));
    QString fileHelp = "[optional] file path to the vault.\n default:" +
                       (QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/qvault.bin");
    parser.addPositionalArgument("file", QCoreApplication::translate("main", fileHelp.toLocal8Bit().constData()));

    QCommandLineOption getO(QStringList() << "g"
                                          << "get",
                            QCoreApplication::translate("main", "get data from the vault => Get Mode."));
    parser.addOption(getO);
    QCommandLineOption setO(QStringList() << "s"
                                          << "set",
                            QCoreApplication::translate("main", "set the data of the vault => Set Mode."));
    parser.addOption(setO);

    parser.process(app);
    const QStringList args = parser.positionalArguments();
    bool getMode = parser.isSet(getO);
    bool setMode = parser.isSet(setO);

    if ((setMode && getMode) || (!setMode && !getMode))
        parser.showHelp();
    if (setMode && (args.size() < 2 || args.size() > 3))
        parser.showHelp();

    if (getMode && (args.size() < 1 || args.size() > 2))
        parser.showHelp();

    qutils::Vault *vault = nullptr;
    if (getMode && args.size() > 1)
    {
        vault = new qutils::Vault(&app, args.at(1));
    }
    if (setMode && args.size() > 2)
    {
        vault = new qutils::Vault(&app, args.at(2));
    }
    if (!vault)
        vault = new qutils::Vault(&app);
    if (getMode)
    {
        const auto data = vault->getData(args.at(0).toUtf8());
        QTextStream out(stdout);
        out << data << "\n";
    }
    else
    {
        vault->setData(args.at(0).toUtf8(), args.at(1).toUtf8());
    }

    return 0;
}

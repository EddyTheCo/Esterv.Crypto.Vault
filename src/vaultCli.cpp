#include "esterv/crypto/vault.hpp"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
    using namespace Esterv::Crypto;
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("VaultCli");

    QCommandLineParser parser;
    parser.setApplicationDescription("Get/Set data from/to the Vault file. Only one mode can be set [Set|Get].");
    parser.addHelpOption();
    parser.addPositionalArgument("pass", QCoreApplication::translate("main", "password of the vault."));
    parser.addPositionalArgument("newpass",
                                 QCoreApplication::translate("main", "new password of the vault. Only with -sc mode."));
    parser.addPositionalArgument("data",
                                 QCoreApplication::translate("main", "data to store on the vault. Only with -s mode"));

    QString fileHelp = "[optional] file path to the vault.\n default:" +
                       (QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/estervvault.bin");
    parser.addPositionalArgument("file", QCoreApplication::translate("main", fileHelp.toLocal8Bit().constData()));

    QCommandLineOption getO(QStringList() << "g" << "get",
                            QCoreApplication::translate("main", "get data from the vault => Get Mode."));
    parser.addOption(getO);
    QCommandLineOption setO(QStringList() << "s" << "set",
                            QCoreApplication::translate("main", "set the data of the vault => Set Mode."));
    parser.addOption(setO);
    QCommandLineOption setP(QStringList() << "c" << "cpass",
                            QCoreApplication::translate("main", "change vault password => Set Mode."));
    parser.addOption(setP);

    parser.process(app);
    const QStringList args = parser.positionalArguments();
    bool getMode = parser.isSet(getO);
    bool setMode = parser.isSet(setO);
    bool changePassword = parser.isSet(setP);

    if ((setMode && getMode) || (!setMode && !getMode))
        parser.showHelp();
    if (setMode && (args.size() < 2 || args.size() > 3))
        parser.showHelp();

    if (getMode && (args.size() < 1 || args.size() > 2))
        parser.showHelp();

    Vault *vault = nullptr;
    if (getMode && args.size() > 1)
    {
        vault = new Vault(&app, args.at(1));
    }
    if (setMode && args.size() > 2)
    {
        vault = new Vault(&app, args.at(2));
    }
    if (!vault)
        vault = new Vault(&app);
    if (getMode)
    {
        const auto data = vault->getData(args.at(0).toUtf8());
        QTextStream out(stdout);
        out << data << "\n";
    }
    else
    {
        if (changePassword)
        {
            vault->changePassword(args.at(0).toUtf8(), args.at(1).toUtf8());
        }
        else
        {
            vault->setData(args.at(0).toUtf8(), args.at(1).toUtf8());
        }
    }

    return 0;
}

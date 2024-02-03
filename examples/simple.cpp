#include"vault.hpp"
#include<QCoreApplication>
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    qutils::Vault* vault=new qutils::Vault(&app);
    vault->setData("This is a test","password");
    auto data=vault->getData("password");
    qDebug()<<data;
    return app.exec();
}

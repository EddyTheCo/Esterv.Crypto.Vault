#pragma once

#include<QObject>
#include<QByteArray>
#include <QFileInfo>
#include <QStandardPaths>

#include <openssl/evp.h>

#if defined(WINDOWS_QVAULT)
# define QVAULT_EXPORT Q_DECL_EXPORT
#else
#define QVAULT_EXPORT Q_DECL_IMPORT
#endif


namespace qutils{

class QVAULT_EXPORT Vault: public QObject
{

    Q_OBJECT
public:
    Vault(QObject *parent=nullptr,const QString filename=(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/qvault.bin"));
    QByteArray getData(QByteArray password);
    bool setData(QByteArray plainText,QByteArray password);

private:
    bool setContent(QByteArray plainText, QByteArray key);
    QByteArray getContent(QByteArray key);
    void setRandomIV();
    void readFromFile();
    void writeToFile();


    const QFileInfo m_file;
    QByteArray m_passHash,m_iv,m_cipherText;
    EVP_CIPHER_CTX *m_ctx;
};
}

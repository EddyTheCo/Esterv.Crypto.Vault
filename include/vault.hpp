#pragma once

#include<QObject>
#include<QByteArray>



#ifndef USE_EMSCRIPTEN

#include <QStandardPaths>
#endif

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
    Vault(QObject *parent=nullptr,const QString filename=
#if defined(USE_EMSCRIPTEN)
"vault"
#else
(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/qvault.bin")
#endif
          );
    QByteArray getData(QByteArray password);
    bool setData(QByteArray plainText,QByteArray password);
    bool isEmpty()const{return m_isEmpty;}

signals:
    void isEmptyChanged();
private:
    bool fromArray(QByteArray);
    void setIsEmpty(bool isEmpty){if(isEmpty!=m_isEmpty){m_isEmpty=isEmpty;emit isEmptyChanged();}};
    bool setContent(QByteArray plainText, QByteArray key);
    QByteArray getContent(QByteArray key);
    void setRandomIV();
    void readFromFile();
    void writeToFile();

    QString m_file;

    QByteArray m_passHash,m_iv,m_cipherText;
    EVP_CIPHER_CTX *m_ctx;
    bool m_isEmpty;
};
}

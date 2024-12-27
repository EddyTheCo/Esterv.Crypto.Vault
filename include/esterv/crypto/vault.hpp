#pragma once

#include <QByteArray>
#include <QObject>

#ifndef USE_EMSCRIPTEN

#include <QStandardPaths>
#endif

#include <openssl/evp.h>

#if defined(USE_QML)
#include <QtQml>
#endif

#if defined(WINDOWS_VAULT)
#define QVAULT_EXPORT Q_DECL_EXPORT
#else
#define QVAULT_EXPORT Q_DECL_IMPORT
#endif

namespace Esterv::Crypto
{

class QVAULT_EXPORT Vault : public QObject
{

    Q_OBJECT
#ifdef USE_QML
    Q_PROPERTY(QString file MEMBER m_file NOTIFY fileChanged)
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY isEmptyChanged)
    QML_ELEMENT
#endif
  public:
    Vault(QObject *parent = nullptr, const QString filename =
#if defined(USE_EMSCRIPTEN)
                                         "vault"
#else
                                         (QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) +
                                          "/estervvault.bin")
#endif
    );
    Q_INVOKABLE QString getDataString(QString password) const;
    QByteArray getData(QByteArray password) const;
    void setFile(QString file)
    {
        if (file != m_file)
        {
            m_file = file;
            restart();
            emit fileChanged();
        }
    }
    Q_INVOKABLE bool setDataString(QString plainText, QString password);
    Q_INVOKABLE bool changePassword(QString oldPass, QString newPass);
    Q_INVOKABLE bool checkPassword(QString password) const;
    bool setData(QByteArray password, QByteArray plainText);
    bool isEmpty() const
    {
        return m_isEmpty;
    }

  signals:
    void isEmptyChanged();
    void fileChanged();

  private:
    bool fromArray(QByteArray);
    void setIsEmpty(bool isEmpty)
    {
        if (isEmpty != m_isEmpty)
        {
            m_isEmpty = isEmpty;
            emit isEmptyChanged();
        }
    };
    bool setContent(QByteArray plainText, QByteArray key);
    QByteArray getContent(QByteArray key) const;
    void setRandomIV();
    void readFromFile();
    void writeToFile();
    void restart();

    QString m_file;

    QByteArray m_passHash, m_iv, m_cipherText;
    EVP_CIPHER_CTX *m_ctx;
    bool m_isEmpty;
};
} // namespace Esterv::Crypto

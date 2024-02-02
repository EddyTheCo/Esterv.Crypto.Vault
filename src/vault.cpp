#include"vault.hpp"
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QPasswordDigestor>
#include <QDir>
#include<QDebug>

#define NITERATIONS 10000
namespace qutils{

Vault::Vault(QObject *parent,const QString filename):QObject(parent),m_ctx(EVP_CIPHER_CTX_new()),
    m_cipherText(128,Qt::Initialization::Uninitialized),m_file(filename)

{
    readFromFile();

    connect(this,&QObject::destroyed,this,[=](){EVP_CIPHER_CTX_free(m_ctx);});
}
void Vault::writeToFile()
{
    if(QDir().mkpath(m_file.absolutePath()))
    {
        auto file=QFile(m_file.absoluteFilePath(),this);
        if(file.open(QIODevice::WriteOnly))
        {
            file.write(m_passHash);
            file.write(m_iv);
            file.write(m_cipherText);
            file.close();
        }
    }
}
void Vault::readFromFile()
{
    auto file=QFile(m_file.absoluteFilePath(),this);
    if(file.exists()&&file.open(QIODevice::ReadOnly))
    {

        QByteArray var=file.readAll();
        file.close();

        if(var.size()>64+16)
        {
            m_passHash=var.first(64);
            var.remove(0,64);
            m_iv=var.first(16);
            var.remove(0,16);
            m_cipherText=var;
            return;
        }
    }
    setRandomIV();

}
void Vault::setRandomIV()
{

    auto buffer=QDataStream(&m_iv,QIODevice::WriteOnly | QIODevice::Append);
    for(auto i=0;i<4;i++)
    {
        quint32 r1 = QRandomGenerator::global()->generate();
        buffer<<r1;
    }

}
bool Vault::setContent(QByteArray plainText, QByteArray key)
{
    int len;
    int ciphertext_len;

    auto var_cipherText=QByteArray(128,Qt::Initialization::Uninitialized);

    if(key.size()!=32)
        return false;

    if(1 != EVP_EncryptInit_ex(m_ctx, EVP_aes_256_cbc(), NULL, reinterpret_cast<const unsigned char*> (key.data()), reinterpret_cast<const unsigned char*> (m_iv.data())))
        return false;

    if(1 != EVP_EncryptUpdate(m_ctx, reinterpret_cast<unsigned char*> (var_cipherText.data()),
                               &len,
                               reinterpret_cast<const unsigned char*> (plainText.data()), plainText.size()))
        return false;
    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(m_ctx, reinterpret_cast<unsigned char*> (var_cipherText.data()) + len, &len))
        return false;
    ciphertext_len += len;
    m_cipherText=var_cipherText;
    m_cipherText.resize(ciphertext_len);
    writeToFile();
    return true;
}
QByteArray Vault::getData(QByteArray password)
{
    auto passHash=QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha512,
        password,
        m_iv,
        NITERATIONS,
        64);

    if(!m_passHash.isEmpty()&&passHash==m_passHash)
    {
        auto key=QPasswordDigestor::deriveKeyPbkdf2(
            QCryptographicHash::Sha256,
            password,
            m_iv,
            NITERATIONS,
            32);

        return getContent(key);
    }
    return QByteArray();
}
bool Vault::setData(QByteArray plainText,QByteArray password)
{
    auto passHash=QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha512,
        password,
        m_iv,
        NITERATIONS,
        64);

    if(m_passHash.isEmpty()||passHash==m_passHash)
    {
        if(m_passHash.isEmpty())m_passHash=passHash;

        auto key=QPasswordDigestor::deriveKeyPbkdf2(
            QCryptographicHash::Sha256,
            password,
            m_iv,
            NITERATIONS,
            32);
        return setContent(plainText,key);
    }
    return false;
}
QByteArray Vault::getContent(QByteArray key)
{
    int len;
    int plaintext_len;

    QByteArray plainText=QByteArray(128,Qt::Initialization::Uninitialized);

    if(1 != EVP_DecryptInit_ex(m_ctx, EVP_aes_256_cbc(), NULL,
                                reinterpret_cast<const unsigned char*> (key.data()),
                                reinterpret_cast<const unsigned char*> (m_iv.data())))
        return QByteArray();

    if(1 != EVP_DecryptUpdate(m_ctx, reinterpret_cast<unsigned char*> (plainText.data()),
                               &len,
                               reinterpret_cast<const unsigned char*> (m_cipherText.data()),
                               m_cipherText.size()))
        return QByteArray();
    plaintext_len=len;
    if(1 != EVP_DecryptFinal_ex(m_ctx, reinterpret_cast<unsigned char*> (plainText.data()) + len, &len))
        return QByteArray();
    plaintext_len += len;

    plainText.resize(plaintext_len);
    return plainText;

}
}

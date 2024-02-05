#include"vault.hpp"
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QPasswordDigestor>
#include <QDir>
#include <QFileInfo>

#define NITERATIONS 10000


#ifdef USE_EMSCRIPTEN

#include <emscripten.h>


EM_JS(void, writeToLS, (const char* name,const char* str), {

    const data=UTF8ToString(str);
    const key=UTF8ToString(name);
    localStorage.setItem(key, data);
});
EM_JS(char*, readFromLS, (const char* name), {
    const key=UTF8ToString(name);
    const data=localStorage.getItem(key);
    return stringToNewUTF8((data)?data:"");
});

#endif

namespace qutils{

Vault::Vault(QObject *parent,const QString filename):QObject(parent),m_ctx(EVP_CIPHER_CTX_new()),
    m_cipherText(128,Qt::Initialization::Uninitialized),m_file(filename),
    m_isEmpty(true)
{
    readFromFile();
    connect(this,&Vault::fileChanged,this,&Vault::restart);
    connect(this,&QObject::destroyed,this,[=](){EVP_CIPHER_CTX_free(m_ctx);});
}
void Vault::restart()
{
    m_cipherText=QByteArray(128,Qt::Initialization::Uninitialized);
    setIsEmpty(true);
    readFromFile();
}
bool Vault::checkPassword(QString password)const
{
    if(m_isEmpty||password.size()<8) return false;

    auto passHash=QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha512,
        password.toUtf8(),
        m_iv,
        NITERATIONS,
        64);
    if(passHash==m_passHash)
    {
        return true;
    }
    return false;

}
bool Vault::changePassword(QString oldPass,QString newPass)
{

    if(m_isEmpty||newPass.size()<8||oldPass.size()<8) return false;

    auto oldPassHash=QPasswordDigestor::deriveKeyPbkdf2(
        QCryptographicHash::Sha512,
        oldPass.toUtf8(),
        m_iv,
        NITERATIONS,
        64);

    if(oldPassHash==m_passHash)
    {
        auto oldkey=QPasswordDigestor::deriveKeyPbkdf2(
            QCryptographicHash::Sha256,
            oldPass.toUtf8(),
            m_iv,
            NITERATIONS,
            32);

        const auto plainText=getContent(oldkey);
        setRandomIV();
        auto newPassHash=QPasswordDigestor::deriveKeyPbkdf2(
            QCryptographicHash::Sha512,
            newPass.toUtf8(),
            m_iv,
            NITERATIONS,
            64);

        auto newkey=QPasswordDigestor::deriveKeyPbkdf2(
            QCryptographicHash::Sha256,
            newPass.toUtf8(),
            m_iv,
            NITERATIONS,
            32);

        m_passHash=newPassHash;
        setContent(plainText,newkey);

        return true;
    }
    return false;
}
void Vault::writeToFile()
{
    QByteArray data;
    data.append(m_passHash);
    data.append(m_iv);
    data.append(m_cipherText);
#ifndef USE_EMSCRIPTEN
    const auto fileInfo=QFileInfo(m_file);
    if(QDir().mkpath(fileInfo.absolutePath()))
    {
        auto file=QFile(fileInfo.absoluteFilePath(),this);
        if(file.open(QIODevice::WriteOnly))
        {
            file.write(data);
            file.close();
        }
    }
#else
    writeToLS(m_file.toUtf8().data(),data.toHex().data());
#endif
}
bool Vault::fromArray(QByteArray var)
{
    if(var.size()>64+16)
    {
        m_passHash=var.first(64);
        var.remove(0,64);
        m_iv=var.first(16);
        var.remove(0,16);
        m_cipherText=var;
        setIsEmpty(false);
        return true;
    }

    return false;
}
void Vault::readFromFile()
{

#ifdef USE_EMSCRIPTEN

char* str=readFromLS(m_file.toUtf8().data());

const auto var=QByteArray(str,strlen(str)); //**********Check this

if(fromArray(QByteArray::fromHex(var)))
{
    free(str);
    return;
}
free(str);

#else
const auto fileInfo=QFileInfo(m_file);
auto file=QFile(fileInfo.absoluteFilePath(),this);
if(file.exists()&&file.open(QIODevice::ReadOnly))
{
    QByteArray var=file.readAll();
    file.close();

    if(fromArray(var))return;
}

#endif
    setRandomIV();

}
void Vault::setRandomIV()
{
    m_iv=QByteArray();
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
    setIsEmpty(false);
    writeToFile();
    return true;
}
QString Vault::getDataString(QString password)const
{
    return QString(getData(password.toUtf8()));
}
QByteArray Vault::getData(QByteArray password)const
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
bool Vault::setDataString(QString plainText,QString password)
{
    return setData(plainText.toUtf8(),password.toUtf8());
}
bool Vault::setData(QByteArray plainText,QByteArray password)
{

    if(password.size()<8)return false;

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
QByteArray Vault::getContent(QByteArray key)const
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

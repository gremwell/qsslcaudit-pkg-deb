#include "test.h"
#include "ssltests.h"
#include "ciphers.h"

#include <QCoreApplication>
#include <QUdpSocket>

#ifdef UNSAFE_QSSL
#include "sslunsafedtls.h"
#else
#include <QDtls>
#endif

// Target SslTest is SslTestCiphersDtls10Exp:
// "test for DTLS 1.0 protocol and EXPORT grade ciphers support"


// do verify peer certificate, use DTLSv1.2
// check for proper test result code
class Test01 : public Test
{
    Q_OBJECT
public:
    Test01(int id, QString testBaseName, QList<SslTest *> sslTests) : Test(id, testBaseName, sslTests) {
        socket = nullptr;
    }

    ~Test01() {
        delete socket;
    }

    void setTestsSettings()
    {
        testSettings.setUseDtls(true);
        testSettings.setUserCN("www.example.com");
    }

    void executeNextSslTest()
    {
        if (!socket)
            socket = new QUdpSocket;

        XSslConfiguration conf;
        conf.setDtlsCookieVerificationEnabled(false);
        conf.setProtocol(XSsl::DtlsV1_2);
        conf.setPeerVerifyMode(XSslSocket::VerifyPeer);

        XDtls dtls(XSslSocket::SslClientMode);
        dtls.setPeer(QHostAddress("127.0.0.1"), 8443);
        dtls.setDtlsConfiguration(conf);

        socket->connectToHost(QHostAddress("127.0.0.1"), 8443);

        bool ret = dtls.doHandshake(socket);
        if (!ret) {
            printTestFailed("handshake failed too early");
            dtls.shutdown(socket);
            return;
        }

        while (socket->waitForReadyRead(200)) {
            qint64 bytesToRead = socket->pendingDatagramSize();
            QByteArray dgram(bytesToRead, Qt::Uninitialized);
            qint64 bytesRead = socket->readDatagram(dgram.data(), dgram.size());
            dgram.resize(bytesRead);
            dtls.doHandshake(socket, dgram);
        }

        if ((dtls.handshakeState() == XDtls::HandshakeNotStarted)
                && (dtls.dtlsErrorString().contains("handshake failure"))) {
            ;
        } else {
            printTestFailed("encrypted session was established, but should not");
        }

        dtls.shutdown(socket);
    }

    void verifySslTestResult()
    {
        if (currentSslTest()->result() == SslTestResult::Success) {
            setResult(0);
            printTestSucceeded();
        } else {
            setResult(-1);
            printTestFailed(QString("unexpected test result (%1)")
                            .arg(sslTestResultToString(currentSslTest()->result())));
        }
    }

private:
    QUdpSocket *socket;
};

// do verify peer certificate, use DtlsV1_0 protocol with medium ciphers
// check for proper test result code
class Test02 : public Test
{
    Q_OBJECT
public:
    Test02(int id, QString testBaseName, QList<SslTest *> sslTests) : Test(id, testBaseName, sslTests) {
        socket = nullptr;
    }

    ~Test02() {
        delete socket;
    }

    void setTestsSettings()
    {
        testSettings.setUseDtls(true);
        testSettings.setUserCN("www.example.com");
    }

    void executeNextSslTest()
    {
        if (!socket)
            socket = new QUdpSocket;

        XSslConfiguration conf;
        conf.setDtlsCookieVerificationEnabled(false);
        conf.setProtocol(XSsl::DtlsV1_0);
        conf.setPeerVerifyMode(XSslSocket::VerifyPeer);

        QList<XSslCipher> mediumCiphers;
        QStringList opensslCiphers = ciphers_medium_str.split(":");

        for (int i = 0; i < opensslCiphers.size(); i++) {
            XSslCipher cipher = XSslCipher(opensslCiphers.at(i));

            if (!cipher.isNull())
                mediumCiphers << cipher;
        }
        if (mediumCiphers.size() == 0) {
            printTestFailed();
            QThread::currentThread()->quit();
            return;
        }
        conf.setCiphers(mediumCiphers);

        XDtls dtls(XSslSocket::SslClientMode);
        dtls.setPeer(QHostAddress("127.0.0.1"), 8443);
        dtls.setDtlsConfiguration(conf);

        socket->connectToHost(QHostAddress("127.0.0.1"), 8443);

        bool ret = dtls.doHandshake(socket);
        if (!ret) {
            printTestFailed("handshake failed too early");
            dtls.shutdown(socket);
            return;
        }

        while (socket->waitForReadyRead(200)) {
            qint64 bytesToRead = socket->pendingDatagramSize();
            QByteArray dgram(bytesToRead, Qt::Uninitialized);
            qint64 bytesRead = socket->readDatagram(dgram.data(), dgram.size());
            dgram.resize(bytesRead);
            dtls.doHandshake(socket, dgram);
        }

        if ((dtls.handshakeState() == XDtls::HandshakeNotStarted)
                && (dtls.dtlsErrorString().contains("handshake failure"))) {
            ;
        } else {
            printTestFailed("encrypted session was established, but should not");
        }

        dtls.shutdown(socket);
    }

    void verifySslTestResult()
    {
        if (currentSslTest()->result() == SslTestResult::Success) {
            setResult(0);
            printTestSucceeded();
        } else {
            setResult(-1);
            printTestFailed(QString("unexpected test result (%1)")
                            .arg(sslTestResultToString(currentSslTest()->result())));
        }
    }

private:
    QUdpSocket *socket;

};

// do verify peer certificate, use DtlsV1_0 protocol with high ciphers
// check for proper test result code
class Test03 : public Test
{
    Q_OBJECT
public:
    Test03(int id, QString testBaseName, QList<SslTest *> sslTests) : Test(id, testBaseName, sslTests) {
        socket = nullptr;
    }

    ~Test03() {
        delete socket;
    }

    void setTestsSettings()
    {
        testSettings.setUseDtls(true);
        testSettings.setUserCN("www.example.com");
    }

    void executeNextSslTest()
    {
        if (!socket)
            socket = new QUdpSocket;

        XSslConfiguration conf;
        conf.setDtlsCookieVerificationEnabled(false);
        conf.setProtocol(XSsl::DtlsV1_0);
        conf.setPeerVerifyMode(XSslSocket::VerifyPeer);

        QList<XSslCipher> highCiphers;
        QStringList opensslCiphers = ciphers_high_str.split(":");

        for (int i = 0; i < opensslCiphers.size(); i++) {
            XSslCipher cipher = XSslCipher(opensslCiphers.at(i));

            if (!cipher.isNull())
                highCiphers << cipher;
        }
        if (highCiphers.size() == 0) {
            printTestFailed();
            QThread::currentThread()->quit();
            return;
        }
        conf.setCiphers(highCiphers);

        XDtls dtls(XSslSocket::SslClientMode);
        dtls.setPeer(QHostAddress("127.0.0.1"), 8443);
        dtls.setDtlsConfiguration(conf);

        socket->connectToHost(QHostAddress("127.0.0.1"), 8443);

        bool ret = dtls.doHandshake(socket);
        if (!ret) {
            printTestFailed("handshake failed too early");
            dtls.shutdown(socket);
            return;
        }

        while (socket->waitForReadyRead(200)) {
            qint64 bytesToRead = socket->pendingDatagramSize();
            QByteArray dgram(bytesToRead, Qt::Uninitialized);
            qint64 bytesRead = socket->readDatagram(dgram.data(), dgram.size());
            dgram.resize(bytesRead);
            dtls.doHandshake(socket, dgram);
        }

        if ((dtls.handshakeState() == XDtls::HandshakeNotStarted)
                && (dtls.dtlsErrorString().contains("handshake failure"))) {
            ;
        } else {
            printTestFailed("encrypted session was established, but should not");
        }

        dtls.shutdown(socket);
    }

    void verifySslTestResult()
    {
        if (currentSslTest()->result() == SslTestResult::Success) {
            setResult(0);
            printTestSucceeded();
        } else {
            setResult(-1);
            printTestFailed(QString("unexpected test result (%1)")
                            .arg(sslTestResultToString(currentSslTest()->result())));
        }
    }

private:
    QUdpSocket *socket;

};

// do verify peer certificate, use DtlsV1_0 protocol with export ciphers
// check for proper test result code
class Test04 : public Test
{
    Q_OBJECT
public:
    Test04(int id, QString testBaseName, QList<SslTest *> sslTests) : Test(id, testBaseName, sslTests) {
        socket = nullptr;
    }

    ~Test04() {
        delete socket;
    }

    void setTestsSettings()
    {
        testSettings.setUseDtls(true);
        testSettings.setUserCN("www.example.com");
    }

    void executeNextSslTest()
    {
        if (!socket)
            socket = new QUdpSocket;

        XSslConfiguration conf;
        conf.setDtlsCookieVerificationEnabled(false);
        conf.setProtocol(XSsl::DtlsV1_0);
        conf.setPeerVerifyMode(XSslSocket::VerifyPeer);

        QList<XSslCipher> exportCiphers;
        QStringList opensslCiphers = ciphers_export_str.split(":");

        for (int i = 0; i < opensslCiphers.size(); i++) {
            XSslCipher cipher = XSslCipher(opensslCiphers.at(i));

            if (!cipher.isNull())
                exportCiphers << cipher;
        }
        if (exportCiphers.size() == 0) {
            printTestFailed();
            QThread::currentThread()->quit();
            return;
        }
        conf.setCiphers(exportCiphers);

        XDtls dtls(XSslSocket::SslClientMode);
        dtls.setPeer(QHostAddress("127.0.0.1"), 8443);
        dtls.setDtlsConfiguration(conf);

        socket->connectToHost(QHostAddress("127.0.0.1"), 8443);

        bool ret = dtls.doHandshake(socket);
        if (!ret) {
            printTestFailed("handshake failed too early");
            dtls.shutdown(socket);
            return;
        }

        while (socket->waitForReadyRead(200)) {
            qint64 bytesToRead = socket->pendingDatagramSize();
            QByteArray dgram(bytesToRead, Qt::Uninitialized);
            qint64 bytesRead = socket->readDatagram(dgram.data(), dgram.size());
            dgram.resize(bytesRead);
            dtls.doHandshake(socket, dgram);
        }

        bool verifyError = false;
        if (dtls.peerVerificationErrors().size() > 0) {
            verifyError = true;
            dtls.abortHandshake(socket); // this clears DTLS errors too
        }

        if (verifyError) {
            ;
        } else {
            printTestFailed("encrypted session was established, but should not");
        }
        dtls.shutdown(socket);
    }

    void verifySslTestResult()
    {
        if (currentSslTest()->result() == SslTestResult::ProtoAccepted) {
            setResult(0);
            printTestSucceeded();
        } else {
            setResult(-1);
            printTestFailed(QString("unexpected test result (%1)")
                            .arg(sslTestResultToString(currentSslTest()->result())));
        }
    }

private:
    QUdpSocket *socket;

};

// do not verify peer certificate, use DtlsV1_0 protocol with export ciphers
// check for proper test result code
class Test05 : public Test
{
    Q_OBJECT
public:
    Test05(int id, QString testBaseName, QList<SslTest *> sslTests) : Test(id, testBaseName, sslTests) {
        socket = nullptr;
    }

    ~Test05() {
        delete socket;
    }

    void setTestsSettings()
    {
        testSettings.setUseDtls(true);
        testSettings.setUserCN("www.example.com");
    }

    void executeNextSslTest()
    {
        if (!socket)
            socket = new QUdpSocket;

        XSslConfiguration conf;
        conf.setDtlsCookieVerificationEnabled(false);
        conf.setProtocol(XSsl::DtlsV1_0);
        conf.setPeerVerifyMode(XSslSocket::VerifyNone);

        QList<XSslCipher> exportCiphers;
        QStringList opensslCiphers = ciphers_export_str.split(":");

        for (int i = 0; i < opensslCiphers.size(); i++) {
            XSslCipher cipher = XSslCipher(opensslCiphers.at(i));

            if (!cipher.isNull())
                exportCiphers << cipher;
        }
        if (exportCiphers.size() == 0) {
            printTestFailed();
            QThread::currentThread()->quit();
            return;
        }
        conf.setCiphers(exportCiphers);

        XDtls dtls(XSslSocket::SslClientMode);
        dtls.setPeer(QHostAddress("127.0.0.1"), 8443);
        dtls.setDtlsConfiguration(conf);

        socket->connectToHost(QHostAddress("127.0.0.1"), 8443);

        bool ret = dtls.doHandshake(socket);
        if (!ret) {
            printTestFailed("handshake failed too early");
            dtls.shutdown(socket);
            return;
        }

        while (socket->waitForReadyRead(200)) {
            qint64 bytesToRead = socket->pendingDatagramSize();
            QByteArray dgram(bytesToRead, Qt::Uninitialized);
            qint64 bytesRead = socket->readDatagram(dgram.data(), dgram.size());
            dgram.resize(bytesRead);
            dtls.doHandshake(socket, dgram);
        }

        bool verifyError = false;
        if (dtls.peerVerificationErrors().size() > 0) {
            verifyError = true;
            dtls.abortHandshake(socket); // this clears DTLS errors too
        }

        if (verifyError) {
            printTestFailed("encrypted session was not established, but should");
        } else {
            ;
        }
        dtls.shutdown(socket);
    }

    void verifySslTestResult()
    {
        if (currentSslTest()->result() == SslTestResult::ProtoAccepted) {
            setResult(0);
            printTestSucceeded();
        } else {
            setResult(-1);
            printTestFailed(QString("unexpected test result (%1)")
                            .arg(sslTestResultToString(currentSslTest()->result())));
        }
    }

private:
    QUdpSocket *socket;

};

QList<Test *> createAutotests()
{
    return QList<Test *>()
            << new Test01(1, "SslTestCiphersDtls10Exp", QList<SslTest *>() << new SslTestCiphersDtls10Exp)
            << new Test02(2, "SslTestCiphersDtls10Exp", QList<SslTest *>() << new SslTestCiphersDtls10Exp)
            << new Test03(3, "SslTestCiphersDtls10Exp", QList<SslTest *>() << new SslTestCiphersDtls10Exp)
            << new Test04(4, "SslTestCiphersDtls10Exp", QList<SslTest *>() << new SslTestCiphersDtls10Exp)
            << new Test05(5, "SslTestCiphersDtls10Exp", QList<SslTest *>() << new SslTestCiphersDtls10Exp)
               ;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TestsLauncher *testsLauncher;

    testsLauncher = new TestsLauncher(createAutotests());

    QObject::connect(testsLauncher, &TestsLauncher::autotestsFinished, [=](){
        qApp->exit(testsLauncher->testsResult());
    });

    testsLauncher->launchNextTest();

    return a.exec();
}

#include "tests_SslTestCiphersDtls10Exp.moc"

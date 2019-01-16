#include "test.h"
#include "ssltests.h"

#include <QCoreApplication>

#ifdef UNSAFE_QSSL
#include "sslunsafesocket.h"
#else
#include <QSslSocket>
#endif

// Target SslTest is SslTest02:
// "certificate trust test with self-signed certificate for user-supplied common name"

// do not verify peer certificate, send data to socket
// check for proper test result code and intercepted data
class Test01 : public Test
{
    Q_OBJECT
public:
    Test01(int id, QString testBaseName, QList<SslTest *> sslTests) : Test(id, testBaseName, sslTests){}

    void setTestSettings()
    {
        testSettings.setUserCN("www.example.com");
    }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;
        QByteArray data = QByteArray("GET / HTTP/1.0\r\n\r\n");

        socket->setPeerVerifyMode(XSslSocket::VerifyNone);

        socket->connectToHostEncrypted("localhost", 8443);

        if (!socket->waitForEncrypted()) {
            qDebug() << socket->error();
            qDebug() << socket->errorString();

            setResult(-1);
            printTestFailed();
        } else {
            socket->write(data);
            socket->waitForReadyRead();

            if (!waitForSslTestFinished()) {
                setResult(-1);
                printTestFailed();
            } else if ((sslTests.first()->result() == SslTest::SSLTEST_RESULT_DATA_INTERCEPTED)
                       && (sslTests.first()->interceptedData() == data)) {
                setResult(0);
                printTestSucceeded();
            } else {
                setResult(-1);
                printTestFailed();
            }
        }
        socket->disconnectFromHost();

        QThread::currentThread()->quit();
    }

};

// do not verify peer certificate, disconnect after timeout
// check for proper test result code
class Test02 : public Test
{
    Q_OBJECT
public:
    Test02(int id, QString testBaseName, QList<SslTest *> sslTests) : Test(id, testBaseName, sslTests){}

    void setTestSettings()
    {
        testSettings.setUserCN("www.example.com");
    }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;

        socket->setPeerVerifyMode(XSslSocket::VerifyNone);

        socket->connectToHostEncrypted("localhost", 8443);

        if (!socket->waitForEncrypted()) {
            qDebug() << socket->error();
            qDebug() << socket->errorString();

            setResult(-1);
            printTestFailed();
        } else {
            QThread::msleep(5500);

            if (sslTests.first()->result() == SslTest::SSLTEST_RESULT_CERT_ACCEPTED) {
                setResult(0);
                printTestSucceeded();
            } else {
                setResult(-1);
                printTestFailed();
            }
        }
        socket->disconnectFromHost();

        QThread::currentThread()->quit();
    }

};

// do verify peer certificate
// check for proper test result code
class Test03 : public Test
{
    Q_OBJECT
public:
    Test03(int id, QString testBaseName, QList<SslTest *> sslTests) : Test(id, testBaseName, sslTests){}

    void setTestSettings()
    {
        testSettings.setUserCN("www.example.com");
    }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;

        socket->setPeerVerifyMode(XSslSocket::VerifyPeer);

        socket->connectToHostEncrypted("localhost", 8443);

        if (!socket->waitForEncrypted()) {
            if (!waitForSslTestFinished()) {
                setResult(-1);
                printTestFailed();
            } else if ((QString::compare(socket->errorString(),
                                         "The host name did not match any of the valid hosts for this certificate") == 0)
                       && (sslTests.first()->result() == SslTest::SSLTEST_RESULT_SUCCESS)) {
                setResult(0);
                printTestSucceeded();
            } else {
                setResult(-1);
                printTestFailed();
            }

        } else {
            setResult(-1);
            printTestFailed();
        }
        socket->disconnectFromHost();

        QThread::currentThread()->quit();
    }

};

// connect to localhost, but set server name to the same as for ssl server
// do verify peer certificate
// check for proper test result code
class Test04 : public Test
{
    Q_OBJECT
public:
    Test04(int id, QString testBaseName, QList<SslTest *> sslTests) : Test(id, testBaseName, sslTests){}

    void setTestSettings()
    {
        testSettings.setUserCN("www.example.com");
    }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;

        socket->setPeerVerifyMode(XSslSocket::VerifyPeer);

        socket->connectToHostEncrypted("localhost", 8443, "www.example.com");

        if (!socket->waitForEncrypted()) {
            if (!waitForSslTestFinished()) {
                setResult(-1);
                printTestFailed();
            } else if ((QString::compare(socket->errorString(),
                                         "The issuer certificate of a locally looked up certificate could not be found") == 0)
                       && (sslTests.first()->result() == SslTest::SSLTEST_RESULT_SUCCESS)) {
                setResult(0);
                printTestSucceeded();
            } else {
                setResult(-1);
                printTestFailed();
            }

        } else {
            setResult(-1);
            printTestFailed();
        }
        socket->disconnectFromHost();

        QThread::currentThread()->quit();
    }

};


void launchTest(Test *autotest)
{
    WHITE(QString("launching autotest #%1").arg(autotest->getId()));

    // we should call it outside of its own thread
    autotest->prepare();

    QThread *autotestThread = new QThread;
    autotest->moveToThread(autotestThread);
    QObject::connect(autotestThread, SIGNAL(started()), autotest, SLOT(run()));
    QObject::connect(autotestThread, SIGNAL(finished()), autotestThread, SLOT(deleteLater()));

    autotestThread->start();

    autotestThread->wait();
}

int main(int argc, char *argv[])
{
    // we need QCoreApplication instance to initialize Qt internals
    QCoreApplication a(argc, argv);
    int ret = 0;

    QList<Test *> autotests = QList<Test *>()
            << new Test01(1, "SslTest02", QList<SslTest *>() << new SslTest02)
            << new Test02(2, "SslTest02", QList<SslTest *>() << new SslTest02)
            << new Test03(3, "SslTest02", QList<SslTest *>() << new SslTest02)
            << new Test04(4, "SslTest02", QList<SslTest *>() << new SslTest02)
               ;

    while (autotests.size() > 0) {
        Test *test = autotests.takeFirst();
        launchTest(test);
        if (test->getResult() != 0) {
            ret = -1;
        }
        test->deleteLater();
    }

    return ret;
}

#include "tests_SslTest02.moc"

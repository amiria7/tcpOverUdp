#include <QCoreApplication>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QTime>
struct tcpheader {
    unsigned short int tcph_srcport;
    unsigned short int tcph_destport;
    unsigned int tcph_seqnum;
    unsigned int tcph_acknum;
    unsigned char tcph_reserved:4, tcph_offset:4;
    unsigned char tcph_flags;
    unsigned short int tcph_win;
    unsigned short int tcph_chksum;
    unsigned short int tcph_urgptr;
};



struct tcpFlag{
    unsigned char ack = 1;
    unsigned char syn = 1 << 2 ;
    unsigned char fin = 1 << 3 ;


} flags;

enum state {
    connected ,
    listeing ,
    connecting

};

unsigned short int calcChkSum(char* data)
{
    return 0x0a0a;
}


const uint tcp_length = 20;
const qint16 dstport = 8889;
const qint16 srcport = 8888;

const QString host = "127.0.0.1";
int datasize = 400;
int trafficSize = 80000;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    tcpheader localInfo;
    enum state socket_state = state::connecting;
    QUdpSocket* client = new QUdpSocket;
    client->bind(QHostAddress::Any, srcport);

    client->connectToHost(host,dstport);
    client->waitForConnected(-1);
    int sentSize = 0;

    tcpheader data;
    data.tcph_flags = flags.syn;
    data.tcph_seqnum = 50;

    qInfo() << QString::number(data.tcph_flags);

    qInfo() << QString::number(client->write((char*)&data,20));
    client->waitForBytesWritten(-1);



    QByteArray buf;
    while (client->waitForReadyRead(-1) || buf.length() > 0 || client->hasPendingDatagrams()) {
        buf = client->receiveDatagram().data();

        if(socket_state == state::connecting)
        {

            tcpheader* _tcpheader = (tcpheader*)buf.left(tcp_length).data();
            buf.remove(0,tcp_length);
            if(_tcpheader->tcph_flags == (flags.syn & flags.ack))
            {
                socket_state = state::connected;

                localInfo.tcph_seqnum = _tcpheader->tcph_seqnum;

                tcpheader data;
                data.tcph_flags = flags.ack;
                data.tcph_seqnum = localInfo.tcph_seqnum;

                client->write((char*)&data,tcp_length);
                client->waitForBytesWritten(-1);


                data.tcph_flags = 0;
                localInfo.tcph_seqnum++;
                data.tcph_seqnum = localInfo.tcph_seqnum;
                QByteArray rawdata((char*)&data,tcp_length);
                rawdata.append(QByteArray('a',datasize));
                client->write(rawdata);
                client->waitForBytesWritten(-1);

            }

        }
        else if(socket_state == state::connected)
        {
            tcpheader* _tcpheader = (tcpheader*)buf.left(tcp_length).data();
            buf.remove(0,tcp_length);
            if(_tcpheader->tcph_flags ==  flags.ack)
            {
                tcpheader data;
                data.tcph_flags = 0;
                localInfo.tcph_seqnum++;
                data.tcph_seqnum = localInfo.tcph_seqnum;
                QByteArray rawdata((char*)&data,tcp_length);
                if(sentSize < trafficSize)
                    rawdata.append(QByteArray('a',datasize));
                else
                    rawdata.append(QByteArray("finished"));

                sentSize += client->write(rawdata);
                client->waitForBytesWritten(-1);


            }
            else if(_tcpheader->tcph_flags ==  flags.fin)
            {

                qInfo() << "the connection was closed";

                break;

            }

        }



    }







    return 0;
}

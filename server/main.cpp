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

bool validateData(short int chk , char* data)
{
    return true;
}

const uint tcp_length = 20;
const qint16 dstport = 8889;
const qint16 srcport = 8888;
QString host = "127.0.0.1";

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    tcpheader localInfo;
    enum state socket_state = state::listeing;
    QUdpSocket* server = new QUdpSocket;

    server->bind(QHostAddress::Any, dstport);

    int recSize = 0;

    QTime start,stop;

    QByteArray buf;
    while (server->waitForReadyRead(-1) || buf.length() > 0 || server->hasPendingDatagrams()) {
        buf = server->receiveDatagram().data();

        if(socket_state == state::listeing)
        {
            tcpheader* _tcpheader = (struct tcpheader*)buf.left(tcp_length).data();
            buf.remove(0,tcp_length);
            //qInfo() << QString::number(_tcpheader->tcph_flags);
            if(_tcpheader->tcph_flags == flags.syn)
            {
                start = QTime::currentTime();

                qInfo() << "syn flag was recieved";
                socket_state = state::connecting;
                localInfo.tcph_seqnum = ++(_tcpheader->tcph_seqnum);

                tcpheader data;
                data.tcph_flags = flags.syn & flags.ack;
                data.tcph_seqnum = localInfo.tcph_seqnum;

                server->connectToHost(host,srcport);
                server->waitForConnected(-1);

                server->write((char*)&data,tcp_length);
                server->waitForBytesWritten(-1);

            }
        }
        else if(socket_state == state::connecting)
        {
            tcpheader* _tcpheader = (struct tcpheader*)buf.left(tcp_length).data();
            buf.remove(0,tcp_length);
            if(_tcpheader->tcph_flags == ( flags.ack))
            {
                qInfo() << "ack flag was recieved";

                socket_state = state::connected;


                tcpheader data;
                data.tcph_flags = flags.ack;

                server->write((char*)&data,tcp_length);
                server->waitForBytesWritten(-1);

            }
        }

        else if(socket_state == state::connected)
        {
            qInfo() << "receiving data" << QString::number(recSize);

            tcpheader* _tcpheader = (struct tcpheader*)buf.left(tcp_length).data();
            buf.remove(0,tcp_length);


            if(buf.contains("finished"))
            {
                tcpheader data;
                data.tcph_flags = flags.fin;

                server->write((char*)&data,tcp_length);
                server->waitForBytesWritten(-1);

                stop = QTime::currentTime();
                break;




            }
            else
            {
                short int checksum = _tcpheader->tcph_chksum;

                if(validateData(checksum,buf.data()))
                {
                    recSize += buf.size();
                    tcpheader data;
                    data.tcph_acknum = _tcpheader->tcph_seqnum;
                    data.tcph_flags = flags.ack;

                    server->write((char*)&data,tcp_length);
                    server->waitForBytesWritten(-1);
                    buf.clear();

                }
                /*
                 * else{}
                 * means packet is corrupted, need to be resend
                 */
            }

        }




    }


    qInfo() << start.msecsTo(stop) << " ms";
    qInfo() << QString::number(recSize) << " byte";


    return 0;
}

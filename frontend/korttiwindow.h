#ifndef KORTTIWINDOW_H
#define KORTTIWINDOW_H

#include <QDialog>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>

namespace Ui {
class KorttiWindow;
}

class KorttiWindow : public QDialog
{
    Q_OBJECT

public:
    explicit KorttiWindow(QString id_kortti,QWidget *parent = nullptr);
    ~KorttiWindow();

    const QByteArray &getWebToken() const;
    void setWebToken(const QByteArray &newWebToken);
    QTimer * pQTimer; //esitellään koosteolio QTimer luokkaan

signals:
    void timeout(); //yhdistety uloskirjautumisen slottiin
    void saldo_signal(QByteArray,QString);
    void tilitapahtumat_nayta(QStringList);
    void saldo_nayta(QStringList);

public slots:
    void tulosta_Tilitapahtumat(QStringList);
    void tulosta_saldo(QStringList);
    void handleTimeout(); //jäsenfunktio jolla käsitellään QTimerin toimintoa

private slots:
    void tilitSlot(QNetworkReply *reply);
    void tiliPaivitysSlot(QNetworkReply *reply);
    void on_comboTili_activated(int index);
    void getOmistajaSlot (QNetworkReply *reply);
    void tiliOperaatio (QNetworkReply *reply);
    void nostoMaaraPaivitys(QString);
    void on_btn_uudemmat_clicked();
    void on_btn_vanhemmat_clicked();
    void on_btnTilitapahtumat_clicked();
    void on_btnSaldo_clicked();
    void on_btnNostaRahaa_clicked();
    void on_btnSiirraRahaa_clicked();
    void on_btnReturn_clicked();
    void on_btnLogout_clicked();
    void on_btn20e_clicked();
    void on_btn40e_clicked();
    void on_btn60e_clicked();
    void on_btn100e_clicked();
    void on_btn200e_clicked();
    void on_btn500e_clicked();
    void on_btnXe_clicked();
    void on_btnNosta_clicked();
    void on_btnTyhjenna_clicked();
    void tilitapahtumatSlot (QNetworkReply *reply);
    void saldoSlot (QNetworkReply *reply);
    void getAsiakasSlot(QNetworkReply *reply);

    void on_btnSiirto_clicked();

private:
    Ui::KorttiWindow *ui;
    QByteArray webToken;
    QString kortti;
    QNetworkReply *reply;
    QByteArray response_data;
    QStringList tilinumero, saldo, luotto, uusi_lista, tilin_omistaja_tiedot, tapahtumat;
    QString aTili, tilin_omistaja, saldo_string,omistaja_tiedot,luotto_string, maara, kortin_omistaja;
    bool bluotto;
    int max, i;
    double ii, luottomax;
    QNetworkAccessManager *korttiManager;

    short s, max_s;

};

#endif // KORTTIWINDOW_H

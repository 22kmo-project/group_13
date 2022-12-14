#include "korttiwindow.h"
#include "qdebug.h"
#include "ui_korttiwindow.h"
#include "myurl.h"

KorttiWindow::KorttiWindow(QString id_kortti, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KorttiWindow)
{
    ui->setupUi(this);
    kortti=id_kortti;
    ui->labelidkortti->setText(kortti+" (Valikko)");
    ui->stackedWidget->setCurrentIndex(0);      //KorttiWindowin valikko
    ui->btnReturn->hide();
    max = 10;
    i = 0;

    pQTimer = new QTimer;
    s=0;
    max_s=30;
    connect(pQTimer,SIGNAL(timeout()),
            this,SLOT(handleTimeout()) );

}

KorttiWindow::~KorttiWindow()
{
    delete ui;
}

const QByteArray &KorttiWindow::getWebToken() const
{
    return webToken;
}

void KorttiWindow::setWebToken(const QByteArray &newWebToken)
{
    webToken = newWebToken;
}

void KorttiWindow::handleTimeout()
{
   s++;
   qDebug()<<s;
   if (s == max_s ) {
       if ( max_s == 10) {
          max_s=30;
          s=0;
          ui->labelidkortti->setText(kortti+" (Valikko)");
          ui->stackedWidget->setCurrentIndex(0);
          ui->btnReturn->hide();
          ui->textTilitapahtumat->clear();
          ui->textSaldo->clear();
          if(tilinumero.size()>1){ui->comboTili->setEnabled(true);}
          ui->comboSiirtoTili->clear();
          i=0;
          max=10;
       }
     else { emit timeout();
            this->close();
            s=0;
            max_s=30;
            pQTimer->stop();
       }

   }
}


void KorttiWindow::tulosta_Tilitapahtumat(QStringList tapahtumat)
{
    //tähän slottiin lähetetään kaikki tilitapahtumien tulostustiedot.
    qDebug()<<"tulosta signaali vastaanotettu tapahtumista";
    qDebug()<<tapahtumat;
    uusi_lista=tapahtumat;
    ui->textTilitapahtumat->setEnabled(false);
    ui->btn_uudemmat->setEnabled(false);

    if (bluotto) { ui->label_tilitapahtumat->setText("Tilin omistaja: "+tilin_omistaja+" Luottoa jäljellä: "+QString::number(luotto_string.toDouble()-saldo_string.toDouble(),'f',2)+" Tilinumero: "+aTili); }
    else {ui->label_tilitapahtumat->setText("Tilin omistaja: "+tilin_omistaja+" Saldo: "+saldo_string+" Tilinumero: "+aTili); }

    QString tulostus="";
    qDebug()<<uusi_lista.length();

    if (uusi_lista.length()>0) {

    if (uusi_lista.length()>i && uusi_lista.length()>max) {//tarkistetaan että tapahtumia on tarpeeksi jotta voidaan muodostaa uudempien 10 tapahtuman stringi
            for (int x=i; x<max; x++){              //muuten tulee error
                tulostus+=uusi_lista[x];  } }
    else if (uusi_lista.length()<11 && uusi_lista.length()>0) {
        ui->btn_vanhemmat->setEnabled(false);
        for (int x=0; x<uusi_lista.length(); x++){
                    tulostus+=uusi_lista[x]; } }

        ui->textTilitapahtumat->setText(tulostus);
    }


    else {
        ui->btn_vanhemmat->setEnabled(false);
        ui->textTilitapahtumat->setText("Ei tilitapahtumia");
    }

}

void KorttiWindow::tulosta_saldo(QStringList lista)
{
    //tähän slottiin lähetetään kaikki saldo tulostustiedot.
    qDebug()<<"tulosta signaali vastaanotettu saldosta";
    qDebug()<<lista.length();
    ui->textSaldo->setEnabled(false);
    if (bluotto) { ui->label_saldo->setText("Luottoa jäljellä: "+QString::number(luotto_string.toDouble()-saldo_string.toDouble(),'f',2)+" Tilinumero: "+aTili); }
    else {ui->label_saldo->setText("Saldo: "+saldo_string+" Tilinumero: "+aTili); }

    QString tapahtumat;

    if (lista.length()>0) {

    if(lista.length()>4) {
    for (int x=0; x<5; x++){
                tapahtumat+=lista[x]; } }
    else if (lista.length()<5 && lista.length()>0) {
        for (int x=0; x<lista.length(); x++){
                    tapahtumat+=lista[x]; } }

    ui->textSaldo->setText("Tilin omistajan tiedot: \n"+omistaja_tiedot+"\n\n"+tapahtumat);
   }

    else {   ui->textSaldo->setText("Tilin omistajan tiedot: \n"+omistaja_tiedot+"\n\nEi tilitapahtumia");
    }
}



void KorttiWindow::on_btnTilitapahtumat_clicked()
{   ui->comboTili->setEnabled(false);
    ui->labelidkortti->setText(kortti+" (Tilitapahtumat)");
    ui->stackedWidget->setCurrentIndex(2);
    ui->btnReturn->show();
       qDebug()<<aTili;
       QString site_url=MyUrl::getBaseUrl()+"/selaa_tilitapahtumia/"+aTili;
       QNetworkRequest request((site_url));
       //WEBTOKEN ALKU
       request.setRawHeader(QByteArray("Authorization"),(this->getWebToken()));
       //WEBTOKEN LOPPU
       korttiManager = new QNetworkAccessManager(this);

       connect(korttiManager, SIGNAL(finished (QNetworkReply*)), this, SLOT(tilitapahtumatSlot(QNetworkReply*)));

       reply = korttiManager->get(request);

    ui->btn_uudemmat->setEnabled(false);
    ui->btn_vanhemmat->setEnabled(true);

    s=0;
    max_s=10;
}


void KorttiWindow::on_btnSaldo_clicked()
{   ui->comboTili->setEnabled(false);
    ui->labelidkortti->setText(kortti+" (Saldo)");
    ui->stackedWidget->setCurrentIndex(1);
    ui->btnReturn->show();


    QString site_url=MyUrl::getBaseUrl()+"/selaa_tilitapahtumia/"+aTili;
    QNetworkRequest request((site_url));
    //WEBTOKEN ALKU
    request.setRawHeader(QByteArray("Authorization"),(this->getWebToken()));
    //WEBTOKEN LOPPU
    korttiManager = new QNetworkAccessManager(this);

    connect(korttiManager, SIGNAL(finished (QNetworkReply*)), this, SLOT(saldoSlot(QNetworkReply*)));

    reply = korttiManager->get(request);

    s=0;
    max_s=10;

}


void KorttiWindow::on_btnNostaRahaa_clicked()
{
    ui->comboTili->setEnabled(false);
    ui->labelidkortti->setText(kortti+" (Nosto)");
    if (bluotto) { ui->label_tiliInfo->setText(" Luottoa jäljellä: "+QString::number(luotto_string.toDouble()-saldo_string.toDouble())+" Tilinumero: "+aTili); }
    else {ui->label_tiliInfo->setText(" Saldo: "+saldo_string+" Tilinumero: "+aTili); }
    ui->stackedWidget->setCurrentIndex(3);
    ui->btnReturn->show();

    s=0;
    max_s=10;
}


void KorttiWindow::on_btnSiirraRahaa_clicked()
{
    ui->comboTili->setEnabled(false);
    ui->labelidkortti->setText(kortti+" (Siirto)");
    ui->stackedWidget->setCurrentIndex(4);
    ui->btnReturn->show();
    if (bluotto) { ui->label_siirra->setText("Luottoa jäljellä: "+QString::number(luotto_string.toDouble()-saldo_string.toDouble(),'f',2)); }
    else {ui->label_siirra->setText("Saldo: "+saldo_string); }
    for(i=0;i<tilinumero.size();i++){
        if(tilinumero[i]==aTili){
            //ei lisätä aktiivista tiliä
        }
        else{
            if(luotto[i].toDouble()>0){ui->comboSiirtoTili->addItem("CREDIT "+tilinumero[i]+" ("+QString::number(luotto[i].toDouble()-saldo[i].toDouble(),'f',2)+")",tilinumero[i]);}
            else{ui->comboSiirtoTili->addItem("DEBIT "+tilinumero[i]+" ("+saldo[i]+")",tilinumero[i]);}

        }
    }

    s=0;
    max_s=10;
}


void KorttiWindow::on_btnReturn_clicked()
{
    ui->labelidkortti->setText(kortti+" (Valikko)");
    ui->stackedWidget->setCurrentIndex(0);
    ui->btnReturn->hide();
    ui->textTilitapahtumat->clear();
    ui->textSaldo->clear();
    if(tilinumero.size()>1){ui->comboTili->setEnabled(true);}
    ui->comboSiirtoTili->clear();
    on_btnTyhjenna_clicked();
    i=0;
    max=10;
    pQTimer->start(1000);
    s=0;
    max_s=30;
}

void KorttiWindow::on_btnLogout_clicked()
{
    qDebug()<<"logout signal";
    emit timeout();
    this->close();
    pQTimer->stop();
    s=0;
    max_s=30;
}

void KorttiWindow::tilitSlot(QNetworkReply *reply)
{
   //Haetaan kaikki tilit johon kortin haltijalla on oikeus

   QByteArray response_data=reply->readAll();
   QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
   QJsonArray json_array = json_doc.array();
  //siirretään haetut tiedot QStringListiin
      foreach (const QJsonValue &value, json_array) {
          QJsonObject json_obj = value.toObject();
          luotto+=QString::number(json_obj["luottoraja"].toInt());
          saldo+=QString::number(json_obj["saldo"].toDouble(),'f',2);
          tilinumero+=QString::number(json_obj["id_tilinumero"].toInt());
          if(json_obj["luottoraja"].toInt()>0){
              ui->comboTili->addItem("CREDIT "+QString::number(json_obj["id_tilinumero"].toInt())+" ("+QString::number(json_obj["luottoraja"].toDouble()-json_obj["saldo"].toDouble(),'f',2)+")",json_obj["id_tilinumero"].toString());
              }
              else{
              ui->comboTili->addItem("DEBIT "+QString::number(json_obj["id_tilinumero"].toInt())+" ("+QString::number(json_obj["saldo"].toDouble(),'f',2)+")",json_obj["id_tilinumero"].toString());
          }
      }
      if(tilinumero.size()>1){qDebug()<<"useampi tili löydetty";}
      else{qDebug()<<"yksi tili löydetty";}
      if(tilinumero.size()==1){
          ui->comboTili->setDisabled(1);
          ui->btnSiirraRahaa->setDisabled(1);
      }
      on_comboTili_activated(0);    //kutsutaan jotta saadaan tarvittavat tilitiedot haettua
}

void KorttiWindow::tiliPaivitysSlot(QNetworkReply *reply)
{
   //Haetaan kaikki tilit johon kortin haltijalla on oikeus

   QByteArray response_data=reply->readAll();
   QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
   QJsonArray json_array = json_doc.array();
    luotto.clear();
    saldo.clear();
    tilinumero.clear();
    int x=ui->comboTili->currentIndex();
    ui->comboTili->clear();

  //siirretään haetut tiedot QStringListiin
      foreach (const QJsonValue &value, json_array) {
          QJsonObject json_obj = value.toObject();
          luotto+=QString::number(json_obj["luottoraja"].toInt());
          saldo+=QString::number(json_obj["saldo"].toDouble());
          tilinumero+=QString::number(json_obj["id_tilinumero"].toInt());
          if(json_obj["luottoraja"].toInt()>0){
              ui->comboTili->addItem("CREDIT "+QString::number(json_obj["id_tilinumero"].toInt())+" ("+QString::number(json_obj["luottoraja"].toDouble()-json_obj["saldo"].toDouble(),'f',2)+")",json_obj["id_tilinumero"].toString());
              }
              else{
              ui->comboTili->addItem("DEBIT "+QString::number(json_obj["id_tilinumero"].toInt())+" ("+QString::number(json_obj["saldo"].toDouble(),'f',2)+")",json_obj["id_tilinumero"].toString());
          }
      }
      ui->comboTili->setCurrentIndex(x);
      aTili=tilinumero[x];
      saldo_string=saldo[x];
      luotto_string=luotto[x];
      on_comboTili_activated(x);    //kutsutaan jotta saadaan tarvittavat tilitiedot haettua
}

void KorttiWindow::on_comboTili_activated(int index)    //Kun comboboxissa tehdään valinta
{
    if(tilinumero[index]!=aTili){
    aTili=tilinumero[index];
    saldo_string=saldo[index];
    luotto_string=luotto[index];
    qDebug()<<"aktiivinen tili: "+aTili;

    //Tarkistetaan onko valittu tili Credit vai Debit
    if(luotto[index].toDouble()==0){bluotto=0;}
    else{bluotto=1;}
    //

    QString site_url=MyUrl::getBaseUrl()+"/tili/checkOmistaja/"+aTili;
    QNetworkRequest request((site_url));
    //WEBTOKEN ALKU
    request.setRawHeader(QByteArray("Authorization"),(this->getWebToken()));
    //WEBTOKEN LOPPU
    korttiManager = new QNetworkAccessManager(this);
    connect(korttiManager, SIGNAL(finished (QNetworkReply*)), this, SLOT(getOmistajaSlot(QNetworkReply*)));
    reply = korttiManager->get(request);
    }
}


void KorttiWindow::on_btn_uudemmat_clicked() //tilitapahtumian < nuoli
{
    ui->btn_vanhemmat->setEnabled(true);
    max -=10;
    i-=10;

    if (max == 10 && i == 0) {
        ui->btn_uudemmat->setEnabled(false); }

    QString tulostus="";

    if (uusi_lista.length()>i && uusi_lista.length()>max) //tarkistetaan että tapahtumia on tarpeeksi jotta voidaan muodostaa uudempien 10 tapahtuman stringi
        for (int x=i; x<max; x++){              //muuten tulee error
            tulostus+=uusi_lista[x];
      }


    ui->textTilitapahtumat->setText(tulostus);

}

void KorttiWindow::on_btn_vanhemmat_clicked() //tilitapahtumien > nuoli
{
   ui->btn_uudemmat->setEnabled(true);
   max +=10;
   i+=10;


   int pituus = uusi_lista.length();
   int loput = max-pituus;
   int uusi_max=max-loput;
   QString tulostus="";
   int y= 1;
   if (uusi_lista.length()>i && uusi_lista.length()>max) //tarkistetaan että tapahtumia on tarpeeksi jotta voidaan muodostaa uudempien 10 tapahtuman stringi
       for (int x=i; x<max; x++){              //muuten tulee error
           tulostus+=uusi_lista[x];
           y=0;
     }

   else if(y == 1) {
       for (int x=i; x<uusi_max; x++){
           tulostus+=uusi_lista[x];
   } ui->btn_vanhemmat->setEnabled(false);
    }
    ui->textTilitapahtumat->setText(tulostus);
    pQTimer->stop();
}

void KorttiWindow::on_btn20e_clicked(){nostoMaaraPaivitys("20"); pQTimer->stop();}
void KorttiWindow::on_btn40e_clicked(){nostoMaaraPaivitys("40"); pQTimer->stop();}
void KorttiWindow::on_btn60e_clicked(){nostoMaaraPaivitys("60"); pQTimer->stop();}
void KorttiWindow::on_btn100e_clicked(){nostoMaaraPaivitys("100"); pQTimer->stop();}
void KorttiWindow::on_btn200e_clicked(){nostoMaaraPaivitys("200"); pQTimer->stop();}
void KorttiWindow::on_btn500e_clicked(){nostoMaaraPaivitys("500"); pQTimer->stop();}

void KorttiWindow::on_btnXe_clicked()
{   pQTimer->stop();
    bool ok;
    if(bluotto){ii=QInputDialog::getDouble(this,"Nosto","0-"+QString::number(luotto_string.toDouble()-saldo_string.toDouble()), 0, 0, luotto_string.toDouble()-saldo_string.toDouble(), 2, &ok);}
    else{ii=QInputDialog::getDouble(this,"Nosto","0-"+saldo_string, 0, 0, saldo_string.toDouble(), 2, &ok);}
    if (ok){if(ii>0){nostoMaaraPaivitys(QString::number(ii,'f',2));};}
}

void KorttiWindow::on_btnTyhjenna_clicked()
{
    ui->lineNostoMaara->clear();
    ui->btnNosta->setDisabled(1);
    ui->btnTyhjenna->setDisabled(1);
}

void KorttiWindow::on_btnNosta_clicked()
{
    maara=ui->lineNostoMaara->text();
    if(bluotto==false && saldo_string.toDouble()-maara.toDouble()<0){
        QMessageBox::warning(this,"Varoitus","Tilillä ei ole tarpeeksi rahaa");
    }
    else if(bluotto && saldo_string.toDouble()+maara.toDouble()>luotto_string.toDouble()){
        QMessageBox::warning(this,"Varoitus","Tilillä ei ole tarpeeksi luottoa");
    }

    else{
    qDebug()<<"nostetaan "+maara;
    //
    //nosto osuus
    qDebug()<<kortti;
    qDebug()<<aTili;
    QJsonObject jsonObj;
    jsonObj.insert("id_kortti",kortti);
    jsonObj.insert("id_tilinumero",aTili);
    jsonObj.insert("maara",maara);
    QString site_url=MyUrl::getBaseUrl()+"/tili/nosto/";
    QNetworkRequest request((site_url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader(QByteArray("Authorization"),(this->getWebToken()));
    korttiManager = new QNetworkAccessManager(this);
    connect(korttiManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(tiliOperaatio(QNetworkReply*)));
    reply = korttiManager->post(request, QJsonDocument(jsonObj).toJson());
    //
    }
    on_btnTyhjenna_clicked();
}

void KorttiWindow::nostoMaaraPaivitys(QString s)
{
    ui->lineNostoMaara->setText(s);
    ui->btnNosta->setEnabled(1);
    ui->btnTyhjenna->setEnabled(1);
}

void KorttiWindow::getOmistajaSlot(QNetworkReply *reply)
{
    QByteArray response_data=reply->readAll();
       QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
       QJsonArray json_array = json_doc.array();

       QStringList tilin_omistaja_tiedot = {""};
       tilin_omistaja_tiedot.clear();
   //siirretään haetut tiedot muuttujiin
       foreach (const QJsonValue &value, json_array) {
           QJsonObject json_obj = value.toObject();
           tilin_omistaja=json_obj["tilin omistaja"].toString();
           tilin_omistaja_tiedot+=json_obj["tilin omistaja"].toString()+"\n"+json_obj["osoite"].toString()+"\n"+json_obj["puhnum"].toString();
       }

       omistaja_tiedot=tilin_omistaja_tiedot[0];
       reply->deleteLater();
       korttiManager->deleteLater();

       int i=0;
             if(i==0){

                 QString site_url=MyUrl::getBaseUrl()+"/tili/checkAsiakas/"+kortti;
                 QNetworkRequest request((site_url));
                 //WEBTOKEN ALKU
                 request.setRawHeader(QByteArray("Authorization"),(this->getWebToken()));
                 //WEBTOKEN LOPPU
                 korttiManager = new QNetworkAccessManager(this);
                 connect(korttiManager, SIGNAL(finished (QNetworkReply*)), this, SLOT(getAsiakasSlot(QNetworkReply*)),Qt::QueuedConnection);
                 reply = korttiManager->get(request);
             }

}

void KorttiWindow::tiliOperaatio(QNetworkReply *reply)  //Tulevien nosto ja siirtotapahtumien tarkistaminen
{
    response_data=reply->readAll();
    qDebug()<<response_data;

    if(QString::compare(response_data,"true")==0){
        on_btnReturn_clicked();
        QMessageBox msgBox;
        msgBox.setText("Tilitapahtuma suoritettu onnistuneesti!");
        msgBox.exec();
        //Haetaan tilitiedot
        QString site_url=MyUrl::getBaseUrl()+"/tili/checkTilit/"+kortti;
        QNetworkRequest request((site_url));
        request.setRawHeader(QByteArray("Authorization"),(this->getWebToken()));
        korttiManager = new QNetworkAccessManager(this);
        connect(korttiManager, SIGNAL(finished (QNetworkReply*)), this, SLOT(tiliPaivitysSlot(QNetworkReply*)));
        reply = korttiManager->get(request);
    }
    else{
        QMessageBox::critical(this,"Virhe","Virhe nostoyhteydessä");
    }
}

void KorttiWindow::on_btnSiirto_clicked()
{	pQTimer->stop(); // timeri pysähtyy kun painaa siirtonappia. en tiiä saako pysähtymään kun combotilisiirtoa painaa?
    bool ok=false;
    if(saldo_string.toDouble()-luotto_string.toDouble()!=0){
    if(bluotto){ii=QInputDialog::getDouble(this,"Siirto","Paljonko Siirretään?\n0-"+QString::number(luotto_string.toDouble()-saldo_string.toDouble(),'f',2), 0, 0, luotto_string.toDouble()-saldo_string.toDouble(), 2, &ok);}
    else{
        luottomax=saldo_string.toDouble();
        qDebug()<<luottomax;
        for(int x=0;x<tilinumero.size();x++){
            qDebug()<<ui->comboSiirtoTili->itemData(x);
            qDebug()<<ui->comboSiirtoTili->currentData().toInt();
            qDebug()<<tilinumero[x];
            qDebug()<<saldo[x];
            if(tilinumero[x].toInt()==ui->comboSiirtoTili->currentData().toInt() && luotto[x].toDouble()>0){
                luottomax=saldo[x].toDouble();
                if(luottomax==0){    //jos luotto on nolla
                    luottomax=-1;
                }
            }
        }
        qDebug()<<luottomax;
        if(luottomax>0){
            ii=QInputDialog::getDouble(this,"Siirto","Paljonko Siirretään?\n0-"+QString::number(std::min(luottomax,saldo_string.toDouble())), 0, 0, std::min(luottomax,saldo_string.toDouble()), 2, &ok);

            //ii=QInputDialog::getInt(this,"Siirto","Paljonko Siirretään?\n0-"+QString::number(std::min(luottomax,saldo_string.toInt())), 0, 0, std::min(luottomax,saldo_string.toInt()), 1, &ok);
        }
        else{
            QMessageBox::warning(this,"Varoitus","Valitulle tilille ei voida siirtää rahaa\n(luottoraja saavutettu)");
        }
    }
    if (ok){if(ii>0){
            QMessageBox msgBox;
            msgBox.setText("Siirretään");
            msgBox.setInformativeText(ui->comboTili->currentText()+"\t-"+QString::number(ii)+"\n\tVVV\n"+ui->comboSiirtoTili->currentText()+"\t+"+QString::number(ii)+"\n\nOletko Varma?");
            QAbstractButton* pButtonYes = msgBox.addButton("Kyllä",QMessageBox::YesRole);
            QAbstractButton* pButtonNo = msgBox.addButton("Ei",QMessageBox::NoRole);
            msgBox.setDefaultButton(QMessageBox::No);
            msgBox.exec();
            if(msgBox.clickedButton()==pButtonYes) {qDebug()<<"Kylla";
                qDebug()<<kortti+" "+aTili+" "+ui->comboSiirtoTili->currentData().toString();
                //Tilisiirto paketti osio
                QJsonObject jsonObj;
                jsonObj.insert("id_kortti",kortti);
                jsonObj.insert("id_miinustili",aTili);
                jsonObj.insert("id_plustili",ui->comboSiirtoTili->currentData().toString());
                jsonObj.insert("maara",QString::number(ii,'f',2));
                QString site_url=MyUrl::getBaseUrl()+"/tili/siirto/";
                QNetworkRequest request((site_url));
                request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
                request.setRawHeader(QByteArray("Authorization"),(this->getWebToken()));
                korttiManager = new QNetworkAccessManager(this);
                connect(korttiManager, SIGNAL(finished (QNetworkReply*)), this, SLOT(tiliOperaatio(QNetworkReply*)));
                reply = korttiManager->post(request, QJsonDocument(jsonObj).toJson());
            }
            if(msgBox.clickedButton()==pButtonNo) {qDebug()<<"Ei";}
        };}
    }
    else{
        if(saldo_string.toInt()==0){QMessageBox::warning(this,"Varoitus","Tilillä ei ole tarpeeksi rahaa");}
        else{QMessageBox::warning(this,"Varoitus","Luottoraja saavutettu, et voi siirtää luottorajan yli ("+luotto_string+")");}
    }
}

void KorttiWindow::tilitapahtumatSlot(QNetworkReply *reply)
{
    //response_data=reply->readAll();
    //qDebug()<<response_data;

     QByteArray response_data=reply->readAll();
        QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
        QJsonArray json_array = json_doc.array();

         QStringList tapahtumat = {""};
         tapahtumat.clear();

    //siirretään haetut tiedot muuttujiin
        foreach (const QJsonValue &value, json_array) {
            QJsonObject json_obj = value.toObject();
            tapahtumat+="Tapahtuma: "+json_obj["tapahtuma"].toString()+" "+"Paiva: "+json_obj["paiva"].toString()+" "+"Aika: "+json_obj["aika"].toString()+" "+"Summa: "+QString::number(json_obj["summa"].toDouble(),'f',2)+"\r";
        }

        qDebug()<<"\n"<<tapahtumat;

        qDebug()<<"lahetan nayta signal";
        qDebug()<<"TAPAHTUMAT TULOSTEETTUUUUUUU  \n";

        //yhdistetään signaali
        connect(this,SIGNAL(tilitapahtumat_nayta(QStringList)),
                this, SLOT(tulosta_Tilitapahtumat(QStringList)), Qt::DirectConnection);

        emit tilitapahtumat_nayta(tapahtumat); //lähetetään haetut tiedot tilitapahtumien tulostus slottiin korttiwindowille.

        reply->deleteLater();
        korttiManager->deleteLater();
}

void KorttiWindow::saldoSlot(QNetworkReply *reply)
{
    //response_data=reply->readAll();
    //qDebug()<<response_data;

     QByteArray response_data=reply->readAll();
        QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
        QJsonArray json_array = json_doc.array();

        QStringList tapahtumat = {""};
        tapahtumat.clear();

    //siirretään haetut tiedot muuttujiin
        foreach (const QJsonValue &value, json_array) {
            QJsonObject json_obj = value.toObject();
            tapahtumat+="Tapahtuma: "+json_obj["tapahtuma"].toString()+" "+"Paiva: "+json_obj["paiva"].toString()+" "+"Aika: "+json_obj["aika"].toString()+" "+"Summa: "+QString::number(json_obj["summa"].toDouble(),'f',2)+"\r";
        }

        qDebug()<<"\n"<<tapahtumat;

        qDebug()<<"lahetan nayta signal";
        qDebug()<<"TAPAHTUMAT TULOSTEETTUUUUUUU  \n";
        //yhdistetään signaali
        connect(this,SIGNAL(saldo_nayta(QStringList)),
                this, SLOT(tulosta_saldo(QStringList)), Qt::DirectConnection);
        emit saldo_nayta(tapahtumat); //lähetetään haetut tiedot tilitapahtumien tulostus slottiin korttiwindowille.

        reply->deleteLater();
        korttiManager->deleteLater();
}

void KorttiWindow::getAsiakasSlot(QNetworkReply *reply)
{  qDebug()<<"ASIAKAS";
       QByteArray response_data=reply->readAll();
       QJsonDocument json_doc = QJsonDocument::fromJson(response_data);
       QJsonArray json_array = json_doc.array();

       foreach (const QJsonValue &value, json_array) {
           QJsonObject json_obj = value.toObject();
           kortin_omistaja=json_obj["kortin omistaja"].toString();
       }
       reply->deleteLater();
       korttiManager->deleteLater();
       qDebug()<<kortin_omistaja;
       ui->label_valikko->setText("Kortin omistaja: "+kortin_omistaja);
}

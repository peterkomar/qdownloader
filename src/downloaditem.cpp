#include "downloaditem.h"
#include <QProgressBar>
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include "fileutils.h"

DownloadItem::DownloadItem(QTreeWidget * parent, int type )
    :QTreeWidgetItem(parent, type)
    ,bError(false)
    ,m_state(WAIT_FOR_DOWNLOAD)
{
  bar     = 0;
  m_reply = 0;
  m_file  = 0;
}

void DownloadItem::slot_download(qint64 bytesReceived, qint64 bytesTotal)
{
  if( bar ) {
    bar->setMaximum(bytesTotal);
    bar->setValue(bytesReceived);
  }
}

void DownloadItem::releaseResources()
{
    printDebug("Start release resources");

    treeWidget()->removeItemWidget(this, 1);
    delete bar;
    bar = 0;

    disconnect( m_reply, SIGNAL(readyRead()), this, SLOT(slotReady()) );
    disconnect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this,
              SLOT(slot_download(qint64,qint64)));
    disconnect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
             SLOT(slot_reply_error(QNetworkReply::NetworkError)));
    disconnect(m_reply, SIGNAL(finished()), this, SLOT(slot_finish()));

    delete m_file;
    m_file = 0;
    QNetworkAccessManager *manager = m_reply->manager();
    m_reply->deleteLater();
    manager->deleteLater();
    printDebug("End release resources");
}

void DownloadItem::slot_finish()
{
  printDebug("Finish download "+m_file->fileName());
  m_reply->close();

  QString filePath = m_file->fileName();
  m_file->close();

  releaseResources();


  mainUtils *utils = new mainUtils;
  utils->resizeImage(filePath, text(4).toInt(), text(5).toInt(), m_options.idResizeRule );

  if( m_options.bConvert ) {
      utils->convertToJPG( filePath );
  }

  delete utils;

  if( !bError && ( m_options.copy_option == PrefDialog::COPY_LOCAL ) ) {
    this->setHidden( true );
  }

  setState( DOWNLOAD_COMPLETED );
  setText(1, tr("waiting ..."));
  setToolTip(1, tr("waiting for transfer to target"));

  emit finish_download();
}

void DownloadItem::slot_reply_error(QNetworkReply::NetworkError error)
{

  QString error_string = "";
  switch( error) {
    case QNetworkReply::ConnectionRefusedError :
        error_string = tr("the remote server refused the connection (the server is not accepting requests)");
        break;

    case QNetworkReply::HostNotFoundError :
        error_string = tr("the remote host name was not found (invalid hostname)");
        break;

    default :
        error_string =  m_reply->errorString();
        break;
  }

  printDebug("Error download "+m_file->fileName() +" : "+ error_string);

  m_file->close();
  m_file->remove();

  releaseResources();

  this->setText(1,"Error");
  this->setToolTip(1, error_string);

  setState( ERROR_DOWNLOAD );
  emit finish_download();
}

void DownloadItem::slotReady()
{
  m_file->write(m_reply->readAll());
}

bool DownloadItem::startDownload( QNetworkReply *reply,  const QString& name, DownloadOptions options)
{
  m_options = options;

  m_file = new QFile(name);
  m_reply = reply;

  if (!m_file->open(QIODevice::WriteOnly)) {
    m_reply->abort();
    releaseResources();

    this->setText(1,"Error");
    this->setToolTip(1, "Can't open file "+m_file->fileName() );

    printDebug(" Cant open file ");
    return false;                 // skip this download
  }

  connect( m_reply, SIGNAL(readyRead()), this, SLOT(slotReady()) );
  connect( m_reply, SIGNAL(downloadProgress(qint64,qint64)), this,
            SLOT(slot_download(qint64,qint64)));
  connect( m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
           SLOT(slot_reply_error(QNetworkReply::NetworkError)));
  connect( m_reply, SIGNAL(finished()), this, SLOT(slot_finish()));

  printDebug("Set connected m_reply");
  printDebug("Start download item");

  bar = new QProgressBar;
  treeWidget()->setItemWidget(this,1, bar );
  setState( IN_PROGRESS_DOWNLOAD );

  return true;
}

void DownloadItem::slot_stop()
{
  printDebug( "Call void slot_stop");
  m_reply->abort();
}

void DownloadItem::printDebug(const QString& message )
{
  Q_UNUSED( message );
#ifdef DEBUG_MODE
  qDebug() << message;
#endif
}

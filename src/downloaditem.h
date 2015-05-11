#ifndef DOWNLOADITEM_H
#define DOWNLOADITEM_H

//set debug mode
//#define DEBUG_MODE

#include <QObject>
#include <QTreeWidgetItem>
#include <QNetworkReply>

#include "prefdialog.h"

class QProgressBar;
class QFile;

class DownloadItem : public QObject, public QTreeWidgetItem
{
  Q_OBJECT
  public:
    enum State {WAIT_FOR_DOWNLOAD=0, ERROR_DOWNLOAD, IN_PROGRESS_DOWNLOAD, DOWNLOAD_COMPLETED};
    DownloadItem(QTreeWidget * parent, int type = Type);
    void setState( State state ) { m_state = state; };
    State getState() const { return m_state; };
    void clearError() { bError = false; };

    bool startDownload( QNetworkReply *reply,  const QString& name, DownloadOptions options);

  protected Q_SLOTS:
    void slot_download(qint64 bytesReceived, qint64 bytesTotal);
    void slot_finish();
    void slot_reply_error(QNetworkReply::NetworkError eror);
    void slotReady();
    void slot_stop();

  Q_SIGNALS:
    void finish_download();

  private:
    bool bError;
    DownloadOptions m_options;
    State m_state;
    QProgressBar *bar;
    QNetworkReply *m_reply;
    QFile *m_file;

    void releaseResources();
    void printDebug(const QString& );
};

#endif // DOWNLOADITEM_H

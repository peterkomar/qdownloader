#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "fileutils.h"
#include "prefdialog.h"

class ListTasks;
class QSpinBox;
class QLineEdit;
class QPushButton;
class QLabel;
class QProgressBar;
class QDockWidget;
class FtpTransfer;

class MainWindow : public QMainWindow
{
 Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void GUI();
    void ToolBar();
    void Connect();
    void setDownloadState(bool );
    void loadList( csv list );
    void start_transfer();
    void clearStatusesItems();
    void clearItems();

    //variables
    bool         m_bstop_transfer;
    int          m_count_threads;
    bool         m_inProgressDownload;
    int          m_currentItemsCount;
    DownloadOptions m_options;

    ListTasks    *m_task_list;
    QToolBar     *m_navigationBar;
    QSpinBox     *m_countThread;
    QLineEdit    *m_targetUrl;
    QLabel       *m_label2;
    QString      m_currentDir;
    QProgressBar *m_TotalProgress;
    QDockWidget  *m_info;
    FtpTransfer  *m_ftpTransfer;

    //actions
    QAction *aOpen;
    QAction *aSaveList;
    QAction *aStart;
    QAction *aStop;
    QAction *aConnectDB;
    QAction *aPreferences;

private slots:
    void slot_open();
    void slot_connect_db();
    void slot_save_list();
    void slot_select_destination();
    void slot_preferences();

    void slot_start();
    void slot_stop();
    void slot_end();

    void slot_finish();
    void slot_increment_total();

Q_SIGNALS:
    void signal_stop();

protected:
    void closeEvent( QCloseEvent * event );
};

#endif // MAINWINDOW_H

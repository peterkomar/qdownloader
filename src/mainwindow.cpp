#include "mainwindow.h"
#include "listtasks.h"
#include "downloaditem.h"
#include "sqldlg.h"

#include <QtGui>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    ,m_bstop_transfer(false)
    ,m_count_threads(0)
    ,m_inProgressDownload(false)
    ,m_currentItemsCount(0)
{
    GUI();
    ToolBar();
    Connect();

    m_currentDir = QApplication::applicationDirPath();
    aStop->setDisabled(true);

    m_options.bConvert        = false;
    m_options.copy_option     = PrefDialog::COPY_LOCAL;
    m_options.idResizeRule    = "prestashop-0";
    m_options.s_delimiter     = "\",\"";
    m_options.bReplaceIfExist = false;
    m_options.bOpenOnlyFailed = true;
    m_options.sourcePosition  = 2;
    m_options.targetPosition  = 3;
    m_options.widthPosition   = 4;
    m_options.heightPosition  = 5;
    m_options.statusPosition  = 6;

}

MainWindow::~MainWindow()
{    
}

void MainWindow::GUI()
{
  setUnifiedTitleAndToolBarOnMac(true);

  m_task_list = new ListTasks;
  setCentralWidget(m_task_list);
  setWindowTitle(tr("QDownloader v0.2"));

  setMinimumSize(800, 600);
  setWindowIcon(QIcon(":/img/ico.png"));

}

void MainWindow::ToolBar()
{
  m_navigationBar = addToolBar(tr("Toolbar"));
  //m_navigationBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  m_navigationBar->setIconSize( QSize( 24, 24 ) );

  aOpen = new QAction(tr("Open File Links..."), this);

  aOpen->setIcon( QIcon(":/img/open.png") );
  m_navigationBar->addAction(aOpen);

  aSaveList    = new QAction(QIcon(":/img/save.png"), tr("Save list"), this );
  aStart       = new QAction(QIcon(":/img/start.png"),  tr("Start download"), this );
  aStop        = new QAction(QIcon(":/img/stop.png"),  tr("Stop download"), this );
  aConnectDB   = new QAction(tr("Connect DB..."), this);
  aPreferences = new QAction(tr("Preferences"), this);

  QMenu *m = new QMenu;
  m->addAction(aConnectDB);
  aOpen->setMenu(m);

  m_countThread = new QSpinBox;
  m_countThread->setMinimum(1);
  //m_countThread->setMaximum(10);
  QLabel *label = new QLabel(tr("Count threads"));

  m_label2 = new QLabel(QString("<a href=\"link\">%1<\a>").arg(tr("Destination")));

  m_targetUrl = new QLineEdit;
  m_targetUrl->setText(QApplication::applicationDirPath());

  m_navigationBar->addAction(aSaveList);
  m_navigationBar->addSeparator();
  m_navigationBar->addWidget(label);
  m_navigationBar->addWidget(m_countThread);
  m_navigationBar->addAction(aStart);
  m_navigationBar->addAction(aStop);
  m_navigationBar->addSeparator();
  m_navigationBar->addWidget(m_label2);
  m_navigationBar->addWidget(m_targetUrl);
  m_navigationBar->addAction(aPreferences);

  m_info = new QDockWidget("Total progress",this, Qt::WindowTitleHint);
  m_info->setFeatures( QDockWidget::DockWidgetMovable |QDockWidget::DockWidgetFloatable );
  m_TotalProgress = new QProgressBar;
  m_info->setWidget( m_TotalProgress );
  addDockWidget( Qt::BottomDockWidgetArea, m_info );
  m_info->hide();
}

void MainWindow::Connect()
{
  connect( aOpen, SIGNAL(triggered()), this, SLOT(slot_open()) );
  connect( aSaveList, SIGNAL(triggered()), this, SLOT(slot_save_list()) );
  connect( aStart, SIGNAL(triggered()), this, SLOT(slot_start()) );
  connect( aStop, SIGNAL(triggered()), this, SLOT(slot_stop()) );
  connect( m_label2, SIGNAL(linkActivated(QString)), this, SLOT( slot_select_destination() ) );
  connect( aConnectDB, SIGNAL(triggered()), this, SLOT(slot_connect_db()) );
  connect( aPreferences, SIGNAL(triggered()), this, SLOT(slot_preferences()));
}

void MainWindow::slot_preferences()
{    
    PrefDialog *dlg = new PrefDialog( m_options, this);

    dlg->exec();

    m_options = dlg->getOptions();
    delete dlg;
}


void MainWindow::slot_open()
{
   QString fileName = QFileDialog::getOpenFileName(this,"RLF file",
                                             m_currentDir, "csv Files (*.csv)"
                                             ";;htm Files(*.htm, *.html)"
                                             ";;Dep repos(*.debr)");
    fileName = fileName.simplified();
    if( fileName.isEmpty() ) {
        return;
    }

    QFileInfo fifo( fileName );
    m_currentDir = fifo.absolutePath();

    if( fileName.endsWith(".html") || fileName.endsWith(".htm") ) {
      loadList( mainUtils::parseHtml( fileName ) );
      return;
    }

    if( fileName.endsWith(".debr") ) {
      loadList( mainUtils::parseDeb( fileName ) );
      return;
    }

    if( !fileName.endsWith(".csv") ) {
      QMessageBox::information(this, "Sory", "This not support yet");
      return;
    }

    loadList(mainUtils::parseCSV(fileName, m_options ));
}


void MainWindow::slot_save_list()
{
  if( m_task_list->topLevelItemCount() == 0 ) {
    return;
  }

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                            m_currentDir,
                            tr("csv Files (*.csv)"));
  if( fileName.isEmpty() ) {
    return;
  }

  QFileInfo fifo( fileName );
  m_currentDir = fifo.absolutePath();

  if( !fileName.endsWith(".csv") ) {
    fileName += ".csv";
  }

  QString body;

  for( int i=0; i< m_task_list->topLevelItemCount();i++) {
    QTreeWidgetItem *item = m_task_list->topLevelItem(i);
    body += "\""+item->text(0)+"\",\"\",\""+item->text(2)+"\",\""+item->text(3)+"\",\"\"\n";
  }

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
    return;
  }

  QTextStream out(&file);
  out << body;
  file.close();
}

void MainWindow::slot_select_destination()
{
  if( m_options.copy_option == PrefDialog::COPY_LOCAL ) {
      QString fileName = QFileDialog::getExistingDirectory(this,"Select destination",
                                                 QDir::currentPath());
      fileName = fileName.simplified();
      if( fileName.isEmpty() ) {
        return;
      }

      m_targetUrl->setText(fileName);
  }
}

void MainWindow::setDownloadState(bool bSet)
{
  m_info->setVisible( bSet );
  m_countThread->setDisabled(bSet);
  m_label2->setDisabled(bSet);
  m_targetUrl->setDisabled(bSet);
  aOpen->setDisabled(bSet);
  aSaveList->setDisabled(bSet);
  aStart->setDisabled(bSet);
  aStop->setDisabled(!bSet);
  m_inProgressDownload = bSet;
}

void MainWindow::start_transfer()
{
  setDownloadState( true );

  int countItems = m_task_list->count();
  m_currentItemsCount = 0;
  m_TotalProgress->setMaximum( countItems );
  m_TotalProgress->setValue(m_currentItemsCount);


  for( int i=0; i < m_task_list->topLevelItemCount(); i++ ) {

    while( m_count_threads >= m_countThread->value() ) {//wait if threads big of limit
      QApplication::processEvents();
    }

    if( m_bstop_transfer ) {
      break;
    }

    DownloadItem *item = (DownloadItem*)m_task_list->topLevelItem(i);
    if( item->isHidden() ) {
      continue;
    }

    QString source = item->text(2);
    QString target = m_targetUrl->text()+"/"+item->text(3);

    if( !m_options.bReplaceIfExist ) {
      if( QFile::exists( target ) ) {// skip if file is exist
        item->setState( DownloadItem::DOWNLOAD_COMPLETED );
        if( m_options.copy_option == PrefDialog::COPY_LOCAL ) {
          item->setHidden(true);
          m_TotalProgress->setValue( ++m_currentItemsCount );
        }
        continue;
      }
    }

    QString message;
    if( !mainUtils::makePath(target, message ) ) {
      QMessageBox::critical(this,"Error", message);
      break;
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *m_reply = manager->get(QNetworkRequest(QUrl( source )));

    if( !item->startDownload( m_reply, target, m_options ) ) {
      continue;
    }

    connect( item, SIGNAL(finish_download()), this, SLOT(slot_finish()) );
    connect( this, SIGNAL(signal_stop()), item, SLOT(slot_stop()));

    m_count_threads++;
    QApplication::processEvents();
  }

  m_TotalProgress->setValue( countItems );

  while( m_count_threads > 0 ){ //wait for end all threads
      QApplication::processEvents();
  }
  slot_end();

}

void MainWindow::slot_end()
{
  clearItems();
  setDownloadState( false );
}

void MainWindow::slot_increment_total()
{
  m_TotalProgress->setValue(++m_currentItemsCount);
}

void MainWindow::slot_finish()
{
  if( m_options.copy_option== PrefDialog::COPY_LOCAL ) {
    slot_increment_total();
  }

  DownloadItem *transfer = dynamic_cast<DownloadItem *>(sender());
  disconnect( transfer, SIGNAL(finish_download()), this, SLOT(slot_finish()) );
  disconnect( this, SIGNAL(signal_stop()), transfer, SLOT(slot_stop()));
  --m_count_threads;
}

void MainWindow::slot_start()
{
  m_count_threads = 0;
  m_bstop_transfer = false;
  clearStatusesItems();
  start_transfer();
}

void MainWindow::slot_stop()
{
  emit signal_stop();
  m_bstop_transfer = true;
}

void MainWindow::clearStatusesItems()
{    
  for(int i=0;i<m_task_list->topLevelItemCount();i++){
    DownloadItem *item = dynamic_cast<DownloadItem*>(m_task_list->topLevelItem(i));
    item->setText(1, "");
    item->setToolTip(1, "");
    item->setState( DownloadItem::WAIT_FOR_DOWNLOAD );
  }
}

void MainWindow::clearItems()
{
  for(int i=0;i<m_task_list->topLevelItemCount();i++){
    DownloadItem *item = dynamic_cast<DownloadItem*>(m_task_list->topLevelItem(i));
    if( item->isHidden() ) {
      delete item;
    }
  }
}

void MainWindow::slot_connect_db()
{
  SQLDlg *dlg = new SQLDlg;

  dlg->exec();
  if( dlg->isAccepted() ) {
    loadList( dlg->getFilesQueue() );
  }
  delete dlg;
}

void MainWindow::loadList( csv list )
{
  m_task_list->clear();

  while( !list.isEmpty() ) {
    CSV d = list.takeFirst();

    DownloadItem *item = new DownloadItem(m_task_list);
    item->setText( 0,QString::number(d.number) );
    item->setText( 2, d.sorce);
    item->setText( 3, d.target);
    item->setText(4, QString::number(d.width));
    item->setText(5, QString::number(d.height));
  }

  m_task_list->resizeColumnToContents(2);
  m_task_list->resizeColumnToContents(3);
  m_task_list->setCurrentItem(m_task_list->topLevelItem(0));
}

void MainWindow::closeEvent( QCloseEvent * event )
{
  if( m_inProgressDownload ) {
    int id = QMessageBox::question ( this, "Question?", "Terminate process download?", QMessageBox::Yes, QMessageBox::No);
    if( id == QMessageBox::Yes ){
      emit signal_stop();
      m_bstop_transfer= true;

      event->accept();
    }else {
      event->ignore();
    }
  }else {
    event->accept();
  }
  QMainWindow::changeEvent( event );
}

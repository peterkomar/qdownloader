#include "sqldlg.h"
#include "ui_sqldlg.h"
#include "fileutils.h"

#include <QtGui>

SQLDlg::SQLDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SQLDlg)
    ,bAcepted(false)
{
    ui->setupUi(this);

    dbUtils = new mainUtils();

    connect( ui->connectBtn, SIGNAL(clicked()), this, SLOT(slot_connect()) );
    connect( ui->okBtn, SIGNAL(clicked()), this, SLOT(slot_ok()) );
    connect( ui->m_DBselect, SIGNAL(currentIndexChanged(QString)), this, SLOT(slot_changedb(QString)));

    ui->m_driver->addItems( dbUtils->getDBDrivers() );
    ui->dataProgress->setHidden(true);
}

SQLDlg::~SQLDlg()
{
    delete dbUtils;
    delete ui;
}

void SQLDlg::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SQLDlg::slot_connect()
{
  if(!dbUtils->connectDB( ui->m_lineHost->text(),ui->m_lineUser->text(),ui->m_linePassword->text(),
                          ui->m_driver->currentText() )) {

    return;
  }

  ui->m_DBselect->clear();
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui->m_DBselect->addItems( dbUtils->getDataBasesList() );
  QApplication::restoreOverrideCursor();
}

void SQLDlg::slot_changedb(const QString & text )
{
  ui->m_Tableselect->clear();
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  ui->m_Tableselect->addItems( dbUtils->getTablesList( text ) );
  QApplication::restoreOverrideCursor();
}

void SQLDlg::slot_ok()
{
  bAcepted = true;
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  queue = dbUtils->getFilesQueueFromDB(ui->m_DBselect->currentText(), ui->m_Tableselect->currentText(), ui->dataProgress, ui->checkExlude->isChecked());
  QApplication::restoreOverrideCursor();
  accept();
}

csv SQLDlg::getFilesQueue()
{
  return queue;
}

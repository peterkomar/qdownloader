#include "listtasks.h"
#include "downloaditem.h"

#include <QtGui>

ListTasks::ListTasks(QWidget *parent)
        :QTreeWidget(parent)
{
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  //setAllColumnsShowFocus(true);
  setUniformRowHeights(true);
  setWordWrap(false);
  setAlternatingRowColors(true);
  setRootIsDecorated(false);

  QStringList labels;
  labels << "ID" << ""<< tr("Sorce") << tr("Target") << tr("Width") << tr("Height");;
  setHeaderLabels(labels);

  setColumnWidth(0,90);
  header()->setResizeMode(0, QHeaderView::Fixed);

  localMenu = new QMenu;

  aDeleteItem     = new QAction(tr("Remove selected.."), this);
  QAction *aSelect     = new QAction(tr("Select..."), this );

  QAction *a2 = new QAction(tr("Change path in selected items.."), this);
  QAction *a3 = new QAction(tr("Change item.."), this);
  QAction *a4 = new QAction(tr("Change path All.."), this);
  QAction *a5 = new QAction(tr("Delete dublicates"), this);
  QAction *prefixAction = new QAction(tr("Add prefix.."), this);

  connect(a2, SIGNAL(triggered()), this, SLOT(slot_change_path()));
  connect(a3, SIGNAL(triggered()), this, SLOT(slot_change_item()));
  connect(a4, SIGNAL(triggered()), this, SLOT(slot_change_path_all_filter()));
  connect(aDeleteItem, SIGNAL(triggered()), this, SLOT(slot_remove_selected()));
  connect(aSelect, SIGNAL(triggered()), this, SLOT(slot_select_filter()));
  connect( a5, SIGNAL(triggered()), this, SLOT(slot_removeDublicate()) );
  connect( prefixAction, SIGNAL(triggered()), this, SLOT(slot_addPrefix()) );

  localMenu->addAction(aSelect);
  localMenu->addAction(aDeleteItem );
  localMenu->addSeparator();
  localMenu->addAction(a2);
  localMenu->addAction(a3);
  localMenu->addAction(a4);
  localMenu->addSeparator();
  localMenu->addAction( a5 );
  localMenu->addAction(prefixAction);

}

void ListTasks::slot_remove_selected()
{
    QList<DownloadItem *> list;

    for( int i=0; i<topLevelItemCount(); i++ ) {
      DownloadItem *item = (DownloadItem*)topLevelItem(i);
      if( item->isSelected() ) {
        list.append(item);
      }
    }


    QString s;

    for( int i=0; i< list.size();i++) {
      s+=list.at(i)->text(0)+":"+list.at(i)->text(2)+"\n";
    }

    QMessageBox msgBox;
    msgBox.setText("Remove items.");
    msgBox.setInformativeText("Do you want remove selected items ?");
    msgBox.setDetailedText(s);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if(ret == QMessageBox::No) {
        return;
    }

    while(!list.isEmpty()) {
        DownloadItem* item = (DownloadItem *)list.takeFirst();
        if( item->getState() != DownloadItem::IN_PROGRESS_DOWNLOAD ) {
          delete item;
        }
    }

}

void ListTasks::slot_select_filter()
{
  QString s = QInputDialog::getText(this,tr("Select items"), tr("Enter filter for select items"));

  if( s != "" ) {
    QProgressDialog progress("Select items...", "Abort", 0, topLevelItemCount(), this);
    progress.setWindowModality(Qt::WindowModal);

    QList<QTreeWidgetItem *> list = this->findItems(s, Qt::MatchContains, 2);
    for( int i=0;i<list.count();i++ ) {
      progress.setValue(i);
      QTreeWidgetItem *item = (QTreeWidgetItem*)list.at(i);
      item->setSelected( true );
      if( progress.wasCanceled() ){
        break;
      }
      QApplication::processEvents();
    }
    progress.setValue(list.count());
  }
}

void ListTasks::mousePressEvent ( QMouseEvent * event )
{
  if( topLevelItemCount() != 0 ) {
    if(event->button() == Qt::RightButton) {
      localMenu->exec(event->globalPos());
    }
  }

  QTreeWidget::mousePressEvent(event);
}

void ListTasks::slot_addPrefix()
{
    int ci = currentColumn();

    if( ci != 2 && ci != 3 ) {
        ci = 2;
    }

    bool ok;
    QString text = QInputDialog::getText(this, tr("Add prefix"),
                                          "Prefix:", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        for( int i=0; i < topLevelItemCount(); i++ ) {
            QTreeWidgetItem *item = topLevelItem(i);
            item->setText(ci, text + item->text(ci));
        }
    }
}

void ListTasks::slot_change_path()
{
    QList<QTreeWidgetItem*> list = selectedItems();

    if(list.isEmpty())
        return;

    int ci = currentColumn();

    QString s = list.first()->text(((ci == 2) || (ci == 3))? ci: 2);

    QFileInfo fifo(s);
    QString path     = fifo.path();
    QString filename = fifo.fileName();

    bool ok;
    QString text = QInputDialog::getText(this, tr("Change path"),
                                          "", QLineEdit::Normal,
                                         path, &ok);
    if (ok && !text.isEmpty()) {
      for( int i=0; i< list.size();i++) {
        list.at(i)->setText(ci, text+"/"+filename);
      }
    }
}

void ListTasks::slot_change_item()
{
    QTreeWidgetItem *item = currentItem();
    int ci = currentColumn();

    QString s = item->text(((ci == 2) || (ci == 3))? ci: 2);
    bool ok;
    QString text = QInputDialog::getText(this, tr("Change path"),
                                          "", QLineEdit::Normal,
                                         s, &ok);
    if (ok && !text.isEmpty()) {
        item->setText(ci, text);
    }
}

void ListTasks::slot_change_path_all_filter()
{
    QTreeWidgetItem *item = currentItem();
    int ci = currentColumn();

    QString s = item->text(((ci == 2) || (ci == 3))? ci: 2);
    QFileInfo fifo(s);
    QString path     = fifo.path();

    bool ok;
    QString text = QInputDialog::getText(this, tr("Change path"),
                                          "<b>"+path+"</b>"+tr(" Replease on:"), QLineEdit::Normal,
                                         path, &ok);
    if (ok && !text.isEmpty()) {
      for( int i=0; i< topLevelItemCount();i++) {
        item = topLevelItem(i);

        QFileInfo f1(item->text(ci));

        if(f1.path() == path )
            item->setText(ci, text+"/"+f1.fileName());
        }
    }

}

void ListTasks::slot_removeDublicate()
{
  QProgressDialog progress("Remove dublicates...", "Abort", 0, topLevelItemCount(), this);
  progress.setWindowModality(Qt::WindowModal);
  for( int i=0; i<topLevelItemCount()-1; i++ ) {
    DownloadItem *currentItem = (DownloadItem*)topLevelItem(i);
    progress.setValue( i );
    QApplication::processEvents();
    for( int j=i+1; j<topLevelItemCount(); j++ ) {
      if( progress.wasCanceled() ) {
         return;
      }
      DownloadItem *item = (DownloadItem*)topLevelItem(j);
      QString s1 = currentItem->text(2) + currentItem->text(3);
      QString s2 = item->text(2) + item->text(3);
      if( s1 == s2 ) {
        --j;
        delete item;
      }
      QApplication::processEvents();
    }
  }
  progress.setValue(topLevelItemCount());
}

int ListTasks::count()
{
  int m_count = 0;
  for( int i =0; i<topLevelItemCount(); i++ ) {
    DownloadItem *item = (DownloadItem*)topLevelItem(i);
    if( !item->isHidden() ) {
      ++m_count;
    }
  }

  return m_count;
}

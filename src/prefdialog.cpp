#include "prefdialog.h"
#include "ui_prefdialog.h"

#include <QtGui>

class SimpleDelegate : public QItemDelegate
{
  public:
    SimpleDelegate(QObject*pobj=0) : QItemDelegate(pobj)
    {
    }

    QWidget* createEditor( QWidget * parent, const QStyleOptionViewItem& /*option*/, const QModelIndex & index ) const
    {
      int i = index.column();

      if(!i ) {
        return NULL;
      }

      int inumber = index.model()->data(index.sibling(index.row(),1), Qt::EditRole).toInt();

      QSpinBox *ed = new QSpinBox(parent);
      ed->setValue( inumber );
      return ed;
      //return QItemDelegate::createEditor(parent, option, index );
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
      int icolumn = index.column();

      if(icolumn == 1)
      {
        QSpinBox *ed = static_cast<QSpinBox*>(editor);
        int inumber = index.model()->data(index, Qt::EditRole).toInt();
        ed->setValue( inumber );
        return;
      }

      QItemDelegate::setEditorData(editor, index);

    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
      int icolumn = index.column();

      if( icolumn == 1 )
      {
        QSpinBox *ed = static_cast<QSpinBox*>(editor);
        QString value = QString::number( ed->value() );
        model->setData(index, value, Qt::EditRole);
        return;
      }

      QItemDelegate::setModelData(editor, model, index);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex & /*index*/ ) const
    {
      editor->setGeometry(option.rect);
    }

    bool editorEvent ( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index )
    {
      return QItemDelegate::editorEvent(event, model, option, index);
    }
};

PrefDialog::PrefDialog( DownloadOptions options, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrefDialog)
{
    ui->setupUi(this);

    ui->m_delimiter->setText(options.s_delimiter);
    ui->m_presta->setChecked(options.bConvert);

    switch( options.copy_option ) {
      case COPY_LOCAL  : ui->chLocal->setChecked(true); break;
      case COPY_FTP    : ui->chFtp->setChecked(true); break;
      case COPY_BRIDGE : ui->chBridges->setChecked(true); break;
    }

    if( options.bReplaceIfExist ) {
      ui->rReplace->setChecked(true);
    } else {
      ui->rSkipCopy->setChecked(true);
    }

    if( options.bOpenOnlyFailed ) {
      ui->chFailed->setChecked(true);
    } else {
      ui->chFailed->setChecked(false);
    }

    if( options.idResizeRule == "normal"
        || options.idResizeRule == "" ) {
      ui->normalMethod->setChecked(true);
    } else {
      ui->prestaMethod->setChecked(true);

      QStringList l = options.idResizeRule.split( "-" );
      ui->numPrestaMethod->setValue( l.at(1).toInt() );
    }

    connect(ui->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));

    ui->listFields->setItemDelegate( new SimpleDelegate(this) );

    QStandardItemModel *model = new QStandardItemModel(0, 2, this);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Field"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Position"));

    model->insertRow(0);
    model->insertRow(1);
    model->insertRow(2);
    model->insertRow(3);
    model->insertRow(4);

    model->setData(model->index(0, 0), tr("Source url"));
    model->setData(model->index(0, 1), options.sourcePosition);
    model->setData(model->index(1, 0), tr("Target url"));
    model->setData(model->index(1, 1), options.targetPosition);
    model->setData(model->index(2, 0), tr("Width"));
    model->setData(model->index(2, 1), options.widthPosition);
    model->setData(model->index(3, 0), tr("Height"));
    model->setData(model->index(3, 1), options.heightPosition);
    model->setData(model->index(4, 0), tr("Status"));
    model->setData(model->index(4, 1), options.statusPosition);

    ui->listFields->setModel(model);
}

PrefDialog::~PrefDialog()
{
    delete ui;
}

void PrefDialog::changeEvent(QEvent *e)
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

QString PrefDialog::getDelimiter()
{
    return ui->m_delimiter->text();
}

bool PrefDialog::isNeddedConvertImage()
{
  return ui->m_presta->isChecked();
}

bool PrefDialog::isRepleaceIfExist()
{
  if( ui->rReplace->isChecked() ) {
    return true;
  }

  return false;
}

DownloadOptions PrefDialog::getOptions()
{
  DownloadOptions options;
  options.bConvert       = isNeddedConvertImage();
  options.s_delimiter    = getDelimiter();
  options.copy_option    = getCopyOption();
  options.idResizeRule   = getResizeMethod();
  options.bOpenOnlyFailed = ui->chFailed->isChecked();
  options.bReplaceIfExist = isRepleaceIfExist();

  QStandardItemModel *model = (QStandardItemModel*)ui->listFields->model();

  options.sourcePosition = model->data(model->index(0, 1), Qt::EditRole).toInt();
  options.targetPosition = model->data(model->index(1, 1), Qt::EditRole).toInt();
  options.widthPosition  = model->data(model->index(2, 1), Qt::EditRole).toInt();
  options.heightPosition = model->data(model->index(3, 1), Qt::EditRole).toInt();
  options.statusPosition = model->data(model->index(4, 1), Qt::EditRole).toInt();

  return options;
}

QString PrefDialog::getResizeMethod()
{
  QString method = "unknown";

  if( ui->normalMethod->isChecked() ) {
    method = "normal";
  }

  if( ui->prestaMethod->isChecked() ) {
    method = "prestashop";
    method += "-"+QString::number(ui->numPrestaMethod->value());
  }

  return method;
}

PrefDialog::copy_option PrefDialog::getCopyOption()
{
    if( ui->chLocal->isChecked() ) {
        return COPY_LOCAL;
    }

    if( ui->chFtp->isChecked() ) {
        return COPY_FTP;
    }

    return COPY_BRIDGE;
}

void PrefDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
   if (!current)
      current = previous;

    ui->stackedWidget->setCurrentIndex(ui->listWidget->row(current));
}

#ifndef PREFDIALOG_H
#define PREFDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
    class PrefDialog;
}

class QListWidgetItem;
class QLineEdit;
class QLabel;

struct DownloadOptions {
  int copy_option;
  QString s_delimiter;
  bool bConvert;
  QString idResizeRule;
  bool bOpenOnlyFailed;
  bool bReplaceIfExist;
  int sourcePosition;
  int targetPosition;
  int widthPosition;
  int heightPosition;
  int statusPosition;
};

class PrefDialog : public QDialog {
    Q_OBJECT
public:
    enum copy_option{ COPY_LOCAL=0, COPY_FTP, COPY_BRIDGE };
    PrefDialog(DownloadOptions option, QWidget *parent = 0);
    ~PrefDialog();

    QString getDelimiter();
    bool isNeddedConvertImage();
    PrefDialog::copy_option getCopyOption();
    bool isRepleaceIfExist();
    DownloadOptions getOptions();
    QString getResizeMethod();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::PrefDialog *ui;

public slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
};

#endif // PREFDIALOG_H

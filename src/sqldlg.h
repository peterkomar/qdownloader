#ifndef SQLDLG_H
#define SQLDLG_H

#include <QDialog>
#include "fileutils.h"

namespace Ui {
    class SQLDlg;
}


class SQLDlg : public QDialog {
    Q_OBJECT
public:
    SQLDlg(QWidget *parent = 0);
    ~SQLDlg();

    bool isAccepted() const { return bAcepted; };
    csv getFilesQueue();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::SQLDlg *ui;
    bool bAcepted;
    csv queue;
    mainUtils *dbUtils;

private slots:
  void slot_connect();
  void slot_changedb(const QString & text );
  void slot_ok();
};

#endif // SQLDLG_H

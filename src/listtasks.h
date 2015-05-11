#ifndef LISTTASKS_H
#define LISTTASKS_H

#include <QTreeWidget>
class QMenu;
class QAction;

class ListTasks : public QTreeWidget
{
    Q_OBJECT
public:
    ListTasks(QWidget *parent = 0);
    int count();

private Q_SLOTS:
    void slot_remove_selected();
    void slot_select_filter();
    void slot_change_path();
    void slot_change_item();
    void slot_change_path_all_filter();
    void slot_removeDublicate();
    void slot_addPrefix();

protected:
     void mousePressEvent ( QMouseEvent * event );
     QMenu *localMenu;
     QAction *aDeleteItem;
};

#endif // LISTTASKS_H

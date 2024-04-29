/*
 * @file NewTreeView.h
 * 
 * @brief This is used for finding clicks on the tree view.
 */
#ifndef NEWTREEVIEW_H
#define NEWTREEVIEW_H

#include <QTreeView>
#include <QMouseEvent>

class NewTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit NewTreeView(QWidget *parent = nullptr) : QTreeView(parent) {}

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // NEWTREEVIEW_H

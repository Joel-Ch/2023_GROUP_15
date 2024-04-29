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
    /** 
	* @brief Constructor
    */
    explicit NewTreeView(QWidget *parent = nullptr) : QTreeView(parent) {}

protected:
	/**
	* @brief This function is called when the mouse is pressed
    */
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // NEWTREEVIEW_H

/**
 *  @file NewTreeView.h
 * 
 * @brief This is used for finding clicks on the tree view.
 */
#ifndef NEWTREEVIEW_H
#define NEWTREEVIEW_H

#include <QTreeView>
#include <QMouseEvent>

/**
 * @class NewTreeView
 * @brief This class is used for finding clicks on the tree view.
 */
class NewTreeView : public QTreeView
{
    Q_OBJECT
public:
    /** 
	* @brief Constructor
    * @param parent The parent widget.
    * @return A new tree view object.
    */
    explicit NewTreeView(QWidget *parent = nullptr) : QTreeView(parent) {}

protected:
	/**
	* @brief This function is called when the mouse is pressed
    * @param event The mouse event.
    */
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // NEWTREEVIEW_H

/**     @file NewTreeView.cpp
 *
 *     EEEE2076 - Software Engineering & VR Project
 *
 */

#include "NewTreeView.h"

void NewTreeView::mousePressEvent(QMouseEvent *event)
{
    QModelIndex item = indexAt(event->pos());
    if (!item.isValid())
    {
        emit clicked(QModelIndex());
    }
    QTreeView::mousePressEvent(event);
}
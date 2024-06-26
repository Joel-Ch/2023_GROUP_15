/**     @file ModelPartList.h
 *
 *     EEEE2076 - Software Engineering & VR Project
 *
 *     Template for model part list that will be used to create the trewview.
 *
 *     P Evans 2022
 *
 *     @brief This class represents a list of parts in the model treeview
 */

#ifndef VIEWER_MODELPARTLIST_H
#define VIEWER_MODELPARTLIST_H

#include "ModelPart.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QString>
#include <QList>

class ModelPart;

/**
 * @class ModelPartList
 * @brief This class represents a list of parts in the model treeview
 */
class ModelPartList : public QAbstractItemModel
{
  Q_OBJECT /**< A special Qt tag used to indicate that this is a special Qt class that might require preprocessing before compiling. */
      public :
      /**
       * @brief Constructor
       *  Arguments are standard arguments for this type of class but are not used in this example.
       * @param data is not used
       * @param parent is used by the parent class constructor
       */
      ModelPartList(const QString &data, QObject *parent = NULL);

  /**
   * @brief Destructor
   */
  ~ModelPartList();

  /**
   * @brief Return column count
   * @param parent is not used
   * @return number of columns in the tree view - "Part" and "Visible", i.e. 2 in this case
   */
  int columnCount(const QModelIndex &parent) const;

  /**
   * @brief This returns the value of a particular row (i.e. the item index)
   * and columns (i.e. either the "Part" or "Visible" property).
   * @param index in a stucture Qt uses to specify the row and column it wants data for
   * @param role is how Qt specifies what it wants to do with the data
   * @return a QVariant which is a generic variable used to represent any Qt class type, in this case the QVariant will be a string
   */
  QVariant data(const QModelIndex &index, int role) const;

  /**
   * @brief Standard function used by Qt internally.
   * @param index in a stucture Qt uses to specify the row and column it wants data for
   * @return a Qt item flags
   */
  Qt::ItemFlags flags(const QModelIndex &index) const;

  /**
   * @brief Standard function used by Qt internally.
   * @param section is the column index
   * @param orientation is the row index
   * @param role is how Qt specifies what it wants to do with the data
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  /** @brief Get a valid QModelIndex for a location in the tree (row is the row in the tree under "parent"
   * or under the root of the tree if parent isnt specified. Column is either 0 = "Part" or 1 = "Visible"
   * in this example
   * @param row is the item index
   * @param column is 0 or 1 - part name or visible stringstream
   * @param parent where the row is referenced from, usually the tree root
   * @return the QModelIndex structure
   */
  QModelIndex index(int row, int column, const QModelIndex &parent) const;

  /**
   * @brief Take a QModelIndex for an item, get a QModel Index for its parent
   * @param index of item
   * @return index of parent
   */
  QModelIndex parent(const QModelIndex &index) const;

  /**
   *  @brief Get number of rows (items) under an item in tree
   *  @param is the parent, all items under this will be counted
   *  @return number of children
   */
  int rowCount(const QModelIndex &parent) const;

  /**
   * @brief Get a pointer to the root item of the tree
   * @return the root item pointer
   */
  ModelPart *getRootItem();

  /**
   * @brief Get the index of the root item
   * @return the index of the root item
   * @param parent the parent index
   * @param data the data to append
   */
  QModelIndex appendChild(QModelIndex &parent, const QList<QVariant> &data);

  /**
   * @brief Remove a row from the tree
   * @param row the row to remove
   * @param parent the parent index
   * @return true if the row was removed
   */
  bool removeRow(int row, const QModelIndex &parent);

  /**
   * @brief Get the index of the root item from the part
   * @return the index of the root item
   * @param part the model part
   * @param parent the parent index
   */
  QModelIndex index(ModelPart *part, const QModelIndex &parent);

private:
  /**
   * @brief Pointer to the root item of the tree
   *
   */
  ModelPart *rootItem;
};
#endif

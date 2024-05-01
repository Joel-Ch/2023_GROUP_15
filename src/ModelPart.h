/**     @file ModelPart.h
 *
 *     EEEE2076 - Software Engineering & VR Project
 *
 *     Template for model parts that will be added as treeview items
 *
 *     P Evans 2022
 * 
 * @brief This class represents a part in the model treeview
 */

#ifndef VIEWER_MODELPART_H
#define VIEWER_MODELPART_H

#include <QString>
#include <QList>
#include <QVariant>
#include <QColor>
#include <QModelIndex>
#include <vtkSmartPointer.h>
#include <vtkSTLReader.h>
#include <vtkMapper.h>
#include <vtkActor.h>
#include <vtkColor.h>
#include <vtkPolyDataMapper.h>

/** ModelPart class
 * @brief This class represents a part in the model treeview
 */
class ModelPart
{
public:
  /** Constructor
   * @param data is a List (array) of strings for each property of this item (part name, visiblity and colour in our case)
   * @param parent is the parent of this item (one level up in tree)
   */
  ModelPart(const QList<QVariant> &data, ModelPart *parent = nullptr);

  /** Destructor
   * @brief Needs to free array of child items
   */
  ~ModelPart();

  /** Add a child to this item.
   * @param item Pointer to child object (must already be allocated using new)
   */
  void appendChild(ModelPart *item);

  /** Return child at position 'row' below this item
   * @param row is the row number (below this item)
   * @return pointer to the item requested.
   */
  ModelPart *child(int row);

  /** Return number of children to this item
   * @return number of children
   */
  int childCount() const; /* Note on the 'const' keyword - it means that this function is valid for
                           * constant instances of this class. If a class is declared 'const' then it
                           * cannot be modifed, this means that 'set' type functions are usually not
                           * valid, but 'get' type functions are.
                           */

  /** Get number of data items (2 - part name and visibility string) in this case.
   * @return number of visible data columns
   */
  int columnCount() const;

  /** Return the data item at a particular column for this item.
   * i.e. either part name of visibility
   * used by Qt when displaying tree
   * @param column is column index
   * @return the QVariant (represents string)
   */
  QVariant data(int column) const;

  /** Default function required by Qt to allow setting of part
   * properties within treeview.
   * @param column is the index of the property to set
   * @param value is the value to apply
   */
  void set(int column, const QVariant &value);

  /** Get pointer to parent item
   * @return pointer to parent item
   */
  ModelPart *parentItem();

  /** Get row index of item, relative to parent item
   * @return row index
   */
  int row() const;

  /** Set colour
   * @param colour is the colour to set
   */
  void setColour(const QColor &colour);

  /** Get colour
   * @return colour as QColor
   */
  QColor colour() const;

  /** Set visible flag
   * @param isVisible sets visible/non-visible
   */
  void setVisible(bool isVisible);

  /** Get visible flag
   * @return visible flag as boolean
   */
  bool visible() const;

  /** Set part name
   * @param name is the name to set
   */
  void setName(const QString &name);

  /** Get part name
   * @return part name as QString
   */
  QString name() const;

  /** Set folder flag
	* @brief Set this item as a folder
    */
  void setFolder();

  /** Get folder flag
	* @return true if this item is a folder
	*/
  bool isFolder();

  /** Remove a child from this item
   * @param child is the child to remove
   */
  void removeChild(ModelPart *child);

  /** Load STL file
   * @param fileName is the name of the file to load
   */
  void loadSTL(QString fileName);

  /** Return actor
   * @return pointer to default actor for GUI rendering
   */
  vtkSmartPointer<vtkActor> getActor() const;

  /** Return new actor for use in VR
   * @return pointer to new actor
   */
  vtkActor* getNewActor();

private:
  QList<ModelPart *> m_childItems; /**< List (array) of child items */
  QList<QVariant> m_itemData;      /**< List (array of column data for item */
  ModelPart *m_parentItem;         /**< Pointer to parent */

  bool folderFlag; /**< True if this item is a folder */

  /* These are some part properties */
  /*NB: DO NOT USE THESE: m_itemData contains the data in the order name,visible, colour. DO NOT USE MULTIPLE VARIABLES FOR THE SAME INFORMATION*/

  // bool                                        isVisible;          /**< True/false to indicate if should be visible in model rendering */
  // QColor 								        QtColour;             /**< Colour of part */
  // QString                                     partName;           /**< Name of part */

  /* These are vtk properties that will be used to load/render a model of this part */

  vtkSmartPointer<vtkSTLReader> file; /**< Datafile from which part loaded */
  vtkSmartPointer<vtkMapper> mapper;  /**< Mapper for rendering */
  vtkSmartPointer<vtkMapper> VRMapper;  /**< Mapper for rendering in VR*/
  vtkSmartPointer<vtkActor> actor;    /**< Actor for rendering */
  vtkActor *VRActor;    /**< Actor for rendering in VR*/
  vtkColor3<unsigned char> vtkColour; /**< User defineable colour */
};

#endif

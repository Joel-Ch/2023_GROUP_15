/**     @file ModelPart.cpp
 *
 *     EEEE2076 - Software Engineering & VR Project
 *
 */

#include "ModelPart.h"

ModelPart::ModelPart(const QList<QVariant> &data, ModelPart *parent)
    : m_itemData(data), m_parentItem(parent)
{
}

ModelPart::~ModelPart()
{
    qDeleteAll(m_childItems);
}

void ModelPart::appendChild(ModelPart *item)
{
    /* Add another model part as a child of this part
     * (it will appear as a sub-branch in the treeview)
     */
    item->m_parentItem = this;
    m_childItems.append(item);
}

ModelPart *ModelPart::child(int row)
{
    /* Return pointer to child item in row below this item.
     */
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int ModelPart::childCount() const
{
    /* Count number of child items
     */
    return m_childItems.count();
}

int ModelPart::columnCount() const
{
    /* Count number of columns (properties) that this item has.
     */
    return m_itemData.count();
}

QVariant ModelPart::data(int column) const
{
    /* Return the data associated with a column of this item
     *  Note on the QVariant type - it is a generic placeholder type
     *  that can take on the type of most Qt classes. It allows each
     *  column or property to store data of an arbitrary type.
     */
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}

void ModelPart::set(int column, const QVariant &value)
{
    /* Set the data associated with a column of this item
     */
    if (column < 0 || column >= m_itemData.size())
        return;

    m_itemData.replace(column, value);
}

ModelPart *ModelPart::parentItem()
{
    return m_parentItem;
}

int ModelPart::row() const
{
    /* Return the row index of this item, relative to it's parent.
     */
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<ModelPart *>(this));
    return 0;
}

void ModelPart::setColour(const QColor &colour)
{
    m_itemData[2] = colour;
}

QColor ModelPart::colour() const
{
    return m_itemData.at(2).value<QColor>();
}

void ModelPart::setVisible(bool visibility)
{
    m_itemData[1] = visibility;
}

bool ModelPart::visible() const
{
    return m_itemData.at(1).value<bool>();
}

void ModelPart::setName(const QString &name)
{
    m_itemData[0] = name;
}

QString ModelPart::name() const
{
    return m_itemData.at(0).value<QString>();
}

void ModelPart::removeChild(ModelPart *child)
{
    int index = m_childItems.indexOf(child);
    if (index != -1)
    {
        m_childItems.removeAt(index);
    }
}

void ModelPart::loadSTL(QString fileName)
{
    // 1. Use the vtkSTLReader class to load the STL file
    file = vtkSmartPointer<vtkSTLReader>::New();
    file->SetFileName(fileName.toStdString().c_str());
    file->Update();

    // 2. Initialise the part's vtkMapper and link it to the STL reader
    mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection(file->GetOutputPort());

    // 3. Initialise the part's vtkActor and link to the mapper
    actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
}

vtkSmartPointer<vtkActor> ModelPart::getActor() const
{

    /* Needs to return a smart pointer to the vtkActor to allow
     * part to be rendered.
     */
    return actor;
}

// vtkActor* ModelPart::getNewActor() {
/* This is a placeholder function that will be used in the next worksheet.
 *
 * The default mapper/actor combination can only be used to render the part in
 * the GUI, it CANNOT also be used to render the part in VR. This means you need
 * to create a second mapper/actor combination for use in VR - that is the role
 * of this function. */

/* 1. Create new mapper */

/* 2. Create new actor and link to mapper */

/* 3. Link the vtkProperties of the original actor to the new actor. This means
 *    if you change properties of the original part (colour, position, etc), the
 *    changes will be reflected in the GUI AND VR rendering.
 *
 *    See the vtkActor documentation, particularly the GetProperty() and SetProperty()
 *    functions.
 */

/* The new vtkActor pointer must be returned here */
//    return nullptr;

//}

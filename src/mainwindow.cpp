/**     @file mainwindow.cpp
 *
 *     EEEE2076 - Software Engineering & VR Project
 *
 */


#include "mainwindow.h"
#include "./ui_mainwindow.h"

// Constructors Destructors etc
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeView = findChild<NewTreeView *>("treeView");
    ui->treeView->addAction(ui->actionItem_Options);
    ui->treeView->addAction(ui->actionDelete_Item);
    ui->treeView->addAction(ui->actionClip_Filter);
    ui->treeView->addAction(ui->actionShrink_Filter);

    // connections
    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);
    connect(ui->pushButton, &QPushButton::released, this, &MainWindow::handleButton2);
    connect(ui->pushButton_2, &QPushButton::released, this, &MainWindow::handleButton1);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeClicked);
    connect(ui->actionItem_Options, &QAction::triggered, this, &MainWindow::on_actionItem_Options_triggered);
    connect(ui->actionDelete_Item, &QAction::triggered, this, &MainWindow::on_actionDelete_Item_triggered);
    connect(ui->actionStart_VR, &QAction::triggered, this, &MainWindow::on_actionStart_VR_triggered);
    connect(ui->actionStop_VR, &QAction::triggered, this, &MainWindow::on_actionStop_VR_triggered);
    connect(ui->actionSync_VR, &QAction::triggered, this, &MainWindow::on_actionSync_VR_triggered);
    connect(ui->actionClip_Filter, &QAction::triggered, this, &MainWindow::on_actionClip_Filter_triggered);
    connect(ui->actionShrink_Filter, &QAction::triggered, this, &MainWindow::on_actionShrink_Filter_triggered);

    /* Create/allocate the ModelList */
    this->partList = new ModelPartList("Parts List");

    /* Link it to the tree view in the GUI */
    ui->treeView->setModel(this->partList);

    /* Link a render window with the Qt widget */
    renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
    ui->vtkWidget->setRenderWindow(renderWindow);

    // Create a render window interactor ------------------------------------------------------------------

    // Create a prop picker
    vtkSmartPointer<vtkPropPicker> PropPicker = vtkSmartPointer<vtkPropPicker>::New();

    // Set the tolerance for the picker
    // PropPicker->SetTolerance(0.0005);

    // Connect the picker to the render window
    renderWindow->GetInteractor()->SetPicker(PropPicker);

    // Connect the event with a callback function
    vtkSmartPointer<vtkCallbackCommand> clickCallback = vtkSmartPointer<vtkCallbackCommand>::New();

    // Create a lambda function that captures clicks and calls onClick
    auto onClickLambda = [](vtkObject *caller, long unsigned int eventId, void *clientData, void *callData)
    {
        static_cast<MainWindow *>(clientData)->onClick(caller, eventId, clientData, callData);
    };

    // Set the callback to the lambda function
    clickCallback->SetClientData(this);
    clickCallback->SetCallback(onClickLambda);

    renderWindow->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, clickCallback);

    // ---------------------------------------------------------------------------------------------------

    // Create a lambda function that captures rotation and calls onEndInteraction
    auto onEndInteractionLambda = [](vtkObject *caller, long unsigned int eventId, void *clientData, void *callData)
    {
        static_cast<MainWindow *>(clientData)->onEndInteraction(caller, eventId, clientData, callData);
    };

    // Set the callback to the lambda function
    vtkSmartPointer<vtkCallbackCommand> interactionEndCallback = vtkSmartPointer<vtkCallbackCommand>::New();
    interactionEndCallback->SetClientData(this);
    interactionEndCallback->SetCallback(onEndInteractionLambda);
    renderWindow->GetInteractor()->AddObserver(vtkCommand::EndInteractionEvent, interactionEndCallback);

    // Add a renderer
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
    light->SetLightTypeToSceneLight();
    light->SetPosition(5, 5, 15);
    light->SetPositional(true);
    light->SetConeAngle(90);
    light->SetFocalPoint(0, 0, 0);
    light->SetDiffuseColor(1, 1, 1);
    light->SetAmbientColor(1, 1, 1);
    light->SetSpecularColor(1, 1, 1);
    light->SetIntensity(0.5);

    renderer->AddLight(light);

    vrThread = new VRRenderThread();
	connect(vrThread, &VRRenderThread::sendVRMessage, this, &MainWindow::handleVRMessage);
    /*
    // Create a skybox ------------------------------------------------------------------
    vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
    texture->CubeMapOn();

    vtkSmartPointer<vtkJPEGReader> reader = vtkSmartPointer<vtkJPEGReader>::New();

    std::string imageFileNames[6] = {
        "skybox/posx.jpg",
        "skybox/negx.jpg",
        "skybox/posy.jpg",
        "skybox/negy.jpg",
        "skybox/posz.jpg",
        "skybox/negz.jpg"
    };

    // Load the six images
    for (int i = 0; i < 6; i++)
    {
        reader->SetFileName(imageFileNames[i].c_str());
        reader->Update();
        texture->SetInputData(i, reader->GetOutput());
    }

    vtkSmartPointer<vtkSkybox> skybox = vtkSmartPointer<vtkSkybox>::New();
    skybox->SetTexture(texture);

    // Add the skybox to the renderer
    renderer->AddActor(skybox);
    */
}

MainWindow::~MainWindow()
{
    delete ui;
    delete partList;
    delete vrThread;
}

// -----------------------------------------------------------------------------------------------
// Slots
void MainWindow::handleButton1()
{
    // do nothing
    emit statusUpdateMessage(QString("NO ACTION"), 0);
}

void MainWindow::handleButton2()
{
    vrThread->issueCommand(VRRenderThread::REMOVE_FILTERS);
    emit statusUpdateMessage(QString("Filters removed"), 0);
}

void MainWindow::handleTreeClicked(const QModelIndex &index)
{
    /* Check if an item was clicked */
    if (!index.isValid())
    {
        ui->treeView->clearSelection();
        emit statusUpdateMessage(QString("No item selected"), 0);
        return;
    }

    /* Get a pointer to the item from the index */
    ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());

    /* In this case, we will retrieve the name string from the internal QVariant data array */
    QString text = selectedPart->data(0).toString();

    if (selectedPart->isFolder())
        emit statusUpdateMessage(QString("Folder: ") + text, 0);
    else
        emit statusUpdateMessage(QString("Item: ") + text, 0);
}

void MainWindow::on_actionDelete_Item_triggered()
{
    // Disconnect the action's signal - otherwise it goes twice
    disconnect(ui->actionDelete_Item, &QAction::triggered, this, &MainWindow::on_actionDelete_Item_triggered);

    // Status Bar Message
    emit statusUpdateMessage(QString("Delete Item"), 0);

    // Get the selected item
    QModelIndex index = ui->treeView->currentIndex();
    ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());

    if (!selectedPart)
    {
        emit statusUpdateMessage(QString("No item selected"), 0);
        return;
    }

    // Remove the actor from the map
    actorToModelPart.erase(selectedPart->getActor());

    // Delete the selected item
    QModelIndex parentIndex = index.parent();
    int row = index.row();
    if (partList->removeRow(row, parentIndex))
    {
        emit statusUpdateMessage(QString("Item Deleted"), 0);
    }
    else
    {
        emit statusUpdateMessage(QString("Item Not Deleted"), 0);
    }

    // Reconnect the action's signal
    connect(ui->actionDelete_Item, &QAction::triggered, this, &MainWindow::on_actionDelete_Item_triggered);

    // Update the tree view
    partList->dataChanged(parentIndex, parentIndex);

    // Update the render window
    updateRender();
}

// -----------------------------------------------------------------------------------------------
// File Menus

void MainWindow::on_actionOpen_File_triggered()
{
    emit statusUpdateMessage(QString("Opening File"), 0);

    // Open a file dialog
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "C:\\",
        tr("STL Files(*.stl);;Text Files(*.txt)"));

    QModelIndex index;
    openFile(filePath, index);

    updateRender();

    emit statusUpdateMessage(QString("File Opened: ") + filePath, 0);
}

void MainWindow::on_actionOpen_Folder_triggered()
{
    emit statusUpdateMessage(QString("Opening Folder"), 0);

    // Open a directory dialog
    QString dirName = QFileDialog::getExistingDirectory(
        this,
        tr("Open Directory"),
        "C:\\",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    // Check if a directory was selected
    if (!dirName.isEmpty())
    {
        emit statusUpdateMessage(QString("Folder Opened: ") + dirName, 0);

        // Get all STL files in the directory
        QDir directory(dirName);
        QStringList stlFiles = directory.entryList(QStringList() << "*.stl"
                                                                 << "*.STL",
                                                   QDir::Files);

        // Create a QProgressDialog
        QProgressDialog progress("Loading Files...", "Cancel", 0, stlFiles.size(), this);
        progress.setWindowModality(Qt::WindowModal);

        QModelIndex parentIndex;
        if (ui->treeView->selectionModel()->hasSelection())
        {
            parentIndex = ui->treeView->currentIndex();
        }
        // Create a new parent item with the folder name
        QList<QVariant> parentData = {dirName /*, true, QColor(255, 255, 255) */};
        QModelIndex folderIndex = partList->appendChild(parentIndex, parentData);

        // Set the folder flag
        static_cast<ModelPart *>(folderIndex.internalPointer())->setFolder();

        int i = 0;
        foreach (QString fileName, stlFiles)
        {
            // Update the progress dialog
            progress.setValue(i++);
            if (progress.wasCanceled())
                break;
            emit statusUpdateMessage(QString("File Opened: ") + fileName, 0);
            openFile(dirName + "/" + fileName, folderIndex);
        }
        progress.setValue(stlFiles.size());

        // Update the tree view
        partList->dataChanged(QModelIndex(), QModelIndex());

        updateRender();
    }
    // If no directory was selected
    else
    {
        emit statusUpdateMessage(QString("Folder Open Cancelled"), 0);
    }
}

void MainWindow::openFile(const QString &filePath, QModelIndex &parentIndex)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }

    // Get the current index
    QModelIndex index = ui->treeView->currentIndex();

    QString fileName = QFileInfo(filePath).fileName();

    // Default values for the new item
    QString visible("true");
    QColor colour(255, 255, 255);

    // Create a new blank item
    ModelPart *newItem{};

    // Check if a parent index is provided (if a folder is opened)
    if (parentIndex.isValid())
    {
        // initialise the new item with the parent item's data
        newItem = new ModelPart({fileName, visible, colour});
        // Get the parent item
        ModelPart *parentPart = static_cast<ModelPart *>(parentIndex.internalPointer());
        // Append the new item to the parent item
        parentPart->appendChild(newItem);
    }
    // Check if an item is selected (if a file is opened as a child)
    else if (ui->treeView->selectionModel()->hasSelection())
    {
        // If no parent index is provided but an item is selected, append the new item to the selected item
        QModelIndex selectedIndex = ui->treeView->currentIndex();
        // initialise the new item with the selected item's data
        newItem = new ModelPart({fileName, visible, colour});
        // Get the selected item
        ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());
        // Append the new item to the selected item
        selectedPart->appendChild(newItem);
    }
    // Check if no item is selected (if a file is opened as a top-level item)
    else
    {
        // If no parent index is provided and no item is selected, create a new top-level item
        QList<QVariant> data = {fileName, visible, colour};
        QModelIndex newIndex = partList->appendChild(parentIndex, data);
        // Get the new item
        newItem = static_cast<ModelPart *>(newIndex.internalPointer());
    }

    // Update the tree view
    partList->dataChanged(index, index);

    // Load the STL file
    newItem->loadSTL(filePath);

    // Add the actor to the map
    actorToModelPart[newItem->getActor()] = newItem;
}

// -----------------------------------------------------------------------------------------------
// Dialog Box

void MainWindow::on_actionItem_Options_triggered()
{
    // Disconnect the action's signal - otherwise it goes twice
    disconnect(ui->actionItem_Options, &QAction::triggered, this, &MainWindow::on_actionItem_Options_triggered);

    // Status Bar Message
    emit statusUpdateMessage(QString("Item Options"), 0);

    // Get the selected item
    QModelIndex index = ui->treeView->currentIndex();
    ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());

    if (!index.isValid())
    {
        emit statusUpdateMessage(QString("No item selected"), 0);
        return;
    }

    if (selectedPart->isFolder())
    {
        emit statusUpdateMessage(QString("Cannot edit a folder"), 0);
        return;
    }

    // Open the dialog with the selected item's data
    openDialog(selectedPart->name(), selectedPart->visible(), selectedPart->colour());

    // Reconnect the action's signal
    connect(ui->actionItem_Options, &QAction::triggered, this, &MainWindow::on_actionItem_Options_triggered);
}

void MainWindow::openDialog(const QString &name, const bool &isVisible, const QColor &colour)
{
    // Create a dialog object
    Dialog _dialog(this);

    // Connect the dialog's signal to the MainWindow's slot
    connect(&_dialog, &Dialog::sendingData, this, &MainWindow::receiveDialogData);

    // Set the dialog's initial values
    _dialog.setInitialValues(name, isVisible, colour);

    // Show the dialog
    _dialog.exec();
}

void MainWindow::receiveDialogData(const QString &name, const bool &visible, const QColor &colour)
{

    // Display the data in the status bar
    emit statusUpdateMessage(QString("Colour: R%1 G%2 B%3")
                                     .arg(colour.red())
                                     .arg(colour.green())
                                     .arg(colour.blue()) +
                                 QString(" Name: ") + name +
                                 QString(" Visible? ") + QString::number(visible),
                             0);

    // Get the selected item
    QModelIndex index = ui->treeView->currentIndex();
    ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());

    // Set the selected item's data
    selectedPart->setName(name);
    selectedPart->setVisible(visible);
    selectedPart->setColour(colour);

    vrThread->issueCommand(VRRenderThread::SYNC_RENDER);

    updateRender();
}

// -----------------------------------------------------------------------------------------------
// Render Window

void MainWindow::updateRender()
{
    renderer->RemoveAllViewProps();
    for (int i = 0; i < partList->rowCount(QModelIndex()); i++)
    {
        updateRenderFromTree(partList->index(i, 0, QModelIndex()));
    }

    // This line won't render any more than the first item in the list
    // updateRenderFromTree(partList->index(0, 0, QModelIndex()));

    // Reset Camera
    renderer->ResetCamera();
    renderer->ResetCameraClippingRange();
    renderer->Render();

    /*if (vrThread->isRunning())
    {
        vrThread->syncVRActors(actorToModelPart);
    }*/
}

void MainWindow::updateRenderFromTree(const QModelIndex &index)
{
    if (index.isValid())
    {
        ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());
        if (selectedPart->isFolder())
        {
            return;
        }
        // Retrieve actor from selected part and add to renderer
        vtkSmartPointer<vtkActor> actor = selectedPart->getActor();
        if (actor)
        {
            QColor qcolor = selectedPart->colour();
            double red = qcolor.redF();
            double green = qcolor.greenF();
            double blue = qcolor.blueF();

            // Set the color of the actor
            actor->GetProperty()->SetColor(red, green, blue);

            // Check visibility and add or remove actor from renderer
            if (selectedPart->visible())
            {
                if (!renderer->HasViewProp(actor))
                {
                    renderer->AddActor(actor);
                }
            }
            else
            {
                renderer->RemoveActor(actor);
            }
        }
    }

    /* Check to see if this part has any children */
    if (!partList->hasChildren(index) || (index.flags() & Qt::ItemNeverHasChildren))
    {
        return;
    }

    /* Loop through children and add their actors */
    int rows = partList->rowCount(index);
    for (int i = 0; i < rows; i++)
    {
        updateRenderFromTree(partList->index(i, 0, index));
    }
}

void MainWindow::onClick(vtkObject *caller, long unsigned int eventId, void *clientData, void *callData)
{
    // create interactor
    vtkRenderWindowInteractor *interactor = vtkRenderWindowInteractor::SafeDownCast(caller);
    if (interactor)
    {
        int *clickPos = interactor->GetEventPosition();

        // create picker
        vtkSmartPointer<vtkPropPicker> picker = vtkSmartPointer<vtkPropPicker>::New();
        if (picker->Pick(clickPos[0], clickPos[1], 0, renderer))
        {
            // get the clicked actor
            vtkActor *actor = picker->GetActor();
            if (actor)
            {
                // get the corresponding item
                ModelPart *selectedPart = actorToModelPart[actor];
                emit statusUpdateMessage(QString("Clicked on: ") + selectedPart->name(), 0);
                QModelIndex index = partList->index(selectedPart, QModelIndex());
                // select the corresponding item in the tree view
                if (index.isValid())
                {
                    ui->treeView->setCurrentIndex(index);
                }
            }
        }
        else
        {
            // if no actor was clicked
            emit statusUpdateMessage(QString(""), 0);
            ui->treeView->clearSelection();
        }
    }
}

// -----------------------------------------------------------------------------------------------
// VR

void MainWindow::on_actionStart_VR_triggered()
{
    disconnect(ui->actionStart_VR, &QAction::triggered, this, &MainWindow::on_actionStart_VR_triggered);

    emit statusUpdateMessage(QString("Starting VR"), 0);

    // Add list of actors using addActorOffline()
    // (Link these to new mappers)
    for (int i = 0; i < partList->rowCount(QModelIndex()); i++)
    {
        QModelIndex index = partList->index(i, 0, QModelIndex());
        ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());
        vtkActor *actor = selectedPart->getNewActor();
        if (actor)
        {
            // Copy properties from m_itemData to the new actor
            // TODO

            // Add the actor to the VR renderer
            vrThread->addActorOffline(actor, selectedPart);
        }
    }

    // Start the thread
    vrThread->start();

    // TODO: Update the VR render window?
}

void MainWindow::on_actionStop_VR_triggered()
{
    disconnect(ui->actionStop_VR, &QAction::triggered, this, &MainWindow::on_actionStop_VR_triggered);
    connect(ui->actionStart_VR, &QAction::triggered, this, &MainWindow::on_actionStart_VR_triggered);
    emit statusUpdateMessage(QString("Stopping VR"), 0);
    vrThread->issueCommand(VRRenderThread::END_RENDER);

    connect(ui->actionStop_VR, &QAction::triggered, this, &MainWindow::on_actionStop_VR_triggered);
}

void MainWindow::on_actionSync_VR_triggered()
{
    disconnect(ui->actionSync_VR, &QAction::triggered, this, &MainWindow::on_actionSync_VR_triggered);

    emit statusUpdateMessage(QString("Syncing VR"), 0);
    vrThread->issueCommand(VRRenderThread::SYNC_RENDER);

    connect(ui->actionSync_VR, &QAction::triggered, this, &MainWindow::on_actionSync_VR_triggered);
}

void MainWindow::handleVRMessage(const QString& text)
{
    // Show a message to the user
    emit statusUpdateMessage(text,0);
}

// Runs after the user has finished interacting with the render window
void MainWindow::onEndInteraction(vtkObject *caller, long unsigned int eventId, void *clientData, void *callData)
{
    vtkRenderWindowInteractor *interactor = vtkRenderWindowInteractor::SafeDownCast(caller);
    vtkRenderer *renderer = interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    if (renderer)
    {
        vtkCamera *camera = renderer->GetActiveCamera();
        if (camera)
        {
            const double *currentOrientation = camera->GetOrientation();

            // Only issue the rotation commands if the orientation has changed
            if (currentOrientation[0] != previousOrientation[0] ||
                currentOrientation[1] != previousOrientation[1] ||
                currentOrientation[2] != previousOrientation[2])
            {
                vrThread->issueCommand(VRRenderThread::ROTATE_X, currentOrientation[0]);
                vrThread->issueCommand(VRRenderThread::ROTATE_Y, currentOrientation[1]);
                vrThread->issueCommand(VRRenderThread::ROTATE_Z, currentOrientation[2]);
            }
            std::copy(currentOrientation, currentOrientation + 3, previousOrientation);
        }
    }
}

void MainWindow::on_actionClip_Filter_triggered()
{
    disconnect(ui->actionClip_Filter, &QAction::triggered, this, &MainWindow::on_actionClip_Filter_triggered);
    emit statusUpdateMessage(QString("Applying Clip Filter"), 0);
    QModelIndex index = ui->treeView->currentIndex();
    ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());

    if (!selectedPart)
    {
        emit statusUpdateMessage(QString("No item selected"), 0);
    }
    else if (!vrThread->isRunning())
    {
        emit statusUpdateMessage(QString("VR not running"), 0);
    }
    else
    {
        vrThread->applyClipFilter(selectedPart);
        emit statusUpdateMessage(QString("Clip Filter Applied"), 0);
    }

    connect(ui->actionClip_Filter, &QAction::triggered, this, &MainWindow::on_actionClip_Filter_triggered);
}

void MainWindow::on_actionShrink_Filter_triggered()

{
    disconnect(ui->actionShrink_Filter, &QAction::triggered, this, &MainWindow::on_actionShrink_Filter_triggered);

    emit statusUpdateMessage(QString("Applying Clip Filter"), 0);
    QModelIndex index = ui->treeView->currentIndex();
    ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());

    if (!selectedPart)
    {
        emit statusUpdateMessage(QString("No item selected"), 0);
    }
    else if (!vrThread->isRunning())
    {
        emit statusUpdateMessage(QString("VR not running"), 0);
    }
    else
    {
        vrThread->applyShrinkFilter(selectedPart);
        emit statusUpdateMessage(QString("Clip Filter Applied"), 0);
    }

    connect(ui->actionShrink_Filter, &QAction::triggered, this, &MainWindow::on_actionShrink_Filter_triggered);
}

/*
List of cool bonus features:

- Deselection of items
- Opening a folder and adding all files in the folder as children of the folder
- Deleting an item
- Dialog box has a colour display and a text box for the hex code
- Dialog box is cancellable
- Status bar updates with messages
- Clicking on an actor in the render window selects the corresponding item in the tree view
*/
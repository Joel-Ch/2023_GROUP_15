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

    // connections
    connect(this, &MainWindow::statusUpdateMessage, ui->statusbar, &QStatusBar::showMessage);
    connect(ui->pushButton, &QPushButton::released, this, &MainWindow::handleButton2);
    connect(ui->pushButton_2, &QPushButton::released, this, &MainWindow::handleButton1);
    connect(ui->treeView, &QTreeView::clicked, this, &MainWindow::handleTreeClicked);
    connect(ui->actionItem_Options, &QAction::triggered, this, &MainWindow::on_actionItem_Options_triggered);
    connect(ui->actionDelete_Item, &QAction::triggered, this, &MainWindow::on_actionDelete_Item_triggered);
	connect(ui->actionStart_VR, &QAction::triggered, this, &MainWindow::on_actionStart_VR_triggered);
	connect(ui->actionStop_VR, &QAction::triggered, this, &MainWindow::on_actionStop_VR_triggered);

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
    vtkSmartPointer<vtkCallbackCommand> callback = vtkSmartPointer<vtkCallbackCommand>::New();

    // Create a lambda function that captures this and calls onClick
    auto onClickLambda = [](vtkObject *caller, long unsigned int eventId, void *clientData, void *callData)
    {
        static_cast<MainWindow *>(clientData)->onClick(caller, eventId, clientData, callData);
    };

    // Set the callback to the lambda function
    callback->SetClientData(this);
    callback->SetCallback(onClickLambda);

    renderWindow->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, callback);

    // ---------------------------------------------------------------------------------------------------

    // Add a renderer
    renderer = vtkSmartPointer<vtkRenderer>::New();
    renderWindow->AddRenderer(renderer);

    vrThread = new VRRenderThread();
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
    // THIS IS JUST A FILLER FUNCTION - Displays item data
    QModelIndex index = ui->treeView->currentIndex();
    ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());
    if (!index.isValid())
    {
        emit statusUpdateMessage("No Item Selected", 0);
        return;
    }

    QString name = selectedPart->data(0).toString();
    bool visible = selectedPart->data(1).toBool();
    QString colour = selectedPart->data(2).toString();
    // THIS FUNCTION FINDS OUT THE LAST SELECTED ITEM, EVEN IF IT HAS BEEN UNSELECTED - HOW?!

    // This causes MainWindow to emit the signal that will then be received by the statusbar’s slot
    emit statusUpdateMessage(name + QString::number(visible) + colour, 0);
}

void MainWindow::handleButton2()
{
    // FILLER FUNCTION - Clears selection
    ui->treeView->clearSelection();
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

    emit statusUpdateMessage(QString("The selected item is: ") + text, 0);
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

	openFile(filePath);

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
        ModelPart *folderItem = static_cast<ModelPart *>(partList->appendChild(parentIndex, parentData).internalPointer());

        int i = 0;
        foreach (QString fileName, stlFiles)
        {
            // Update the progress dialog
            progress.setValue(i++);
            if (progress.wasCanceled())
                break;
            emit statusUpdateMessage(QString("File Opened: ") + fileName, 0);
			openFile(dirName + "/" + fileName);
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

void MainWindow::openFile(const QString& filePath)
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
    ModelPart* newItem{};

    // Check if an item is selected
    if (ui->treeView->selectionModel()->hasSelection())
    {
        // initialise the new item with the selected item's data
        newItem = new ModelPart({ fileName, visible, colour });
        // Get the selected item
        ModelPart* selectedPart = static_cast<ModelPart*>(index.internalPointer());
        // Append the new item to the selected item
        selectedPart->appendChild(newItem);
    }
    else
    {
        // If no item is selected, create a new top-level item
        QList<QVariant> data = { fileName, visible, colour };
        QModelIndex parent; // An invalid QModelIndex so the item is added to the root
        QModelIndex newIndex = partList->appendChild(parent, data);
        // Get the new item
        newItem = static_cast<ModelPart*>(newIndex.internalPointer());
    }

    // Update the tree view
    partList->dataChanged(index, index);

            // Load the STL file
    newItem->loadSTL(filePath);

            // Add the actor to the map
            actorToModelPart[newItem->getActor()] = newItem;

	vrThread->addActorOffline(newItem->getActor(), newItem);

        updateRender();
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

	vrThread->issueCommand(VRRenderThread::SYNC_RENDER, 0.);

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
}

void MainWindow::updateRenderFromTree(const QModelIndex &index)
{
    if (index.isValid())
    {
        ModelPart *selectedPart = static_cast<ModelPart *>(index.internalPointer());
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
            //TODO

			// Add the actor to the VR renderer
            vrThread->addActorOffline(actor, selectedPart);
        }
    }

    // Start the thread
    vrThread->start();

	//TODO: Update the VR render window?
}

void MainWindow::on_actionStop_VR_triggered()
{
	connect(ui->actionStart_VR, &QAction::triggered, this, &MainWindow::on_actionStart_VR_triggered);
	emit statusUpdateMessage(QString("Stopping VR"), 0);
    vrThread->issueCommand(VRRenderThread::END_RENDER, 0.);
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
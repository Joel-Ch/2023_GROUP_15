/**
 * @file mainwindow.h
 *
 * @brief This file contains the definition of the MainWindow class.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ModelPartList.h"
#include "ModelPart.h"
#include "dialog.h"
#include "NewTreeView.h"
#include <QMessageBox>
#include <qfiledialog.h>
#include <qprogressdialog.h>
#include <vtkCylinderSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkCamera.h>
#include <vtkProperty.h>
#include <vtkPropPicker.h>
#include <vtkCallbackCommand.h>
#include "VRRenderThread.h"
#include <vtkRendererCollection.h>


QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE /**< End of the Qt namespace. */

    /**
     * @brief The MainWindow class represents the main window of the application.
     */
    class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent The parent widget.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor
     */
    ~MainWindow();

    /**
     * @brief Opens a dialog
     * @param name The name data
     * @param visible The visibility data
     * @param colour The color data
     */
    void openDialog(const QString &name, const bool &visible, const QColor &colour);


    /**
	* @brief Opens a file.
	* @param fileName The file name.
	* @param parentIndex The parent index. (set as null if no parent index)
	*/
	void openFile(const QString& fileName, QModelIndex& parentIndex);

    /**
     * @brief Updates the render window.
     */
    void updateRender();

    /**
     * @brief Updates the render window from the tree.
     * @param index The index of the tree.
     */
    void updateRenderFromTree(const QModelIndex &index);

    /**
     * @brief function called when an actor is clicked.
     * @param caller The caller object.
     * @param eventId The event id.
     * @param clientData The client data.
     * @param callData The call data.
     */
    void onClick(vtkObject *caller, long unsigned int eventId, void *clientData, void *callData);

signals:
    /**
     * @brief Emits a status update message.
     * @param message The message to be displayed.
     * @param timeout The timeout duration in milliseconds.
     */
    void statusUpdateMessage(const QString &message, int timeout);
    /**
     * @brief Signal emitted when the dialog data is ready to be sent.
     * @param name The name sent back.
     * @param visible The visibility state.
     * @param colour The selected color.
     */
    void sendingDialogData(const QString &name, const bool &visible, const QColor &colour);

public slots:
    /**
     * @brief Handles the button1 click event.
     */
    void handleButton1();

    /**
     * @brief Handles the button2 click event.
     */
    void handleButton2();

    /**
     * @brief Handles the tree clicked event.
     * @param index The index of the tree.
     */
    void handleTreeClicked(const QModelIndex &index);

    /**
     * @brief Handles the open file event.
     */
    void on_actionOpen_File_triggered();

    /**
     * @brief Handles the open folder event.
     */
    void on_actionOpen_Folder_triggered();

    /**
     * @brief Item Options dropdown event.
     */
    void on_actionItem_Options_triggered();

    /**
     * @brief Delete item dropdown event.
     */
    void on_actionDelete_Item_triggered();


	/**
    * @brief Handles the start VR event.
	*/
	void on_actionStart_VR_triggered();

    /**
    * @brief Handles the stop VR event.
    */
	void on_actionStop_VR_triggered();

    /**
    * @brief Handles the sync VR event.
    */
    void on_actionSync_VR_triggered();


	/**
	* @brief Handles the end interaction event.
	* @param caller The caller object.
	* @param eventId The event id.
	* @param clientData The client data.
	* @param callData The call data.
	*/
    void onEndInteraction(vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

    /**
     * @brief Receives dialog data.
     * @param name The name data.
     * @param visible The visibility data.
     * @param colour The color data.
     */
    void receiveDialogData(const QString &name, const bool &visible, const QColor &colour);

private:
    vtkSmartPointer<vtkRenderer> renderer;
    vtkSmartPointer<vtkGenericOpenGLRenderWindow> renderWindow;

    std::unordered_map<vtkActor *, ModelPart *> actorToModelPart;

    Ui::MainWindow *ui;
    ModelPartList *partList;

	VRRenderThread *vrThread;

	double previousOrientation[3] = { 0, 0, 0 };
};
#endif // MAINWINDOW_H

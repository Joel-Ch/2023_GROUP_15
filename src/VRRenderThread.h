/**		@file VRRenderThread.h
  *
  *		EEEE2046 - Software Engineering & VR Project
  *
  *		Template to add VR rendering to your application.
  *
  *		P Evans 2022
  * 
  * @brief This class is used to create a thread that will handle the VR rendering
  */
#ifndef VR_RENDER_THREAD_H
#define VR_RENDER_THREAD_H

  /* Project headers */

  /* Qt headers */
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

/* Vtk headers */
#include <vtkActor.h>
#include <vtkOpenVRRenderWindow.h>				
#include <vtkOpenVRRenderWindowInteractor.h>	
#include <vtkOpenVRRenderer.h>					
#include <vtkOpenVRCamera.h>	
#include <vtkActorCollection.h>
#include <vtkCommand.h>

/* Other headers */
#include "ModelPart.h"



/* Note that this class inherits from the Qt class QThread which allows it to be a parallel thread
 * to the main() thread, and also from vtkCommand which allows it to act as a "callback" for the
 * vtkRenderWindowInteractor. This callback functionallity means that once the renderWindowInteractor
 * takes control of this thread to enable VR, it can callback to a function in the class to check to see
 * if the user has requested any changes
 */
class VRRenderThread : public QThread {
    Q_OBJECT

public:
    /**
	* @brief List of commands that can be issued to the VR thread
	* @param END_RENDER Stop the rendering
	* @param ROTATE_X Rotate the scene around the X axis
	* @param ROTATE_Y Rotate the scene around the Y axis
	* @param ROTATE_Z Rotate the scene around the Z axis
	* @param SYNC_RENDER Synchronise the rendering
	* @param NO_COMMAND No command issued
	*/
    enum {
        END_RENDER,
        ROTATE_X,
        ROTATE_Y,
        ROTATE_Z,
        SYNC_RENDER,
        NO_COMMAND
    } Command;


    /**  
    * @brief Constructor
    */
    VRRenderThread(QObject* parent = nullptr);

    /**
	* @brief Destructor
    */
    ~VRRenderThread();

    /** 
	* @brief Add an actor to the VR scene before the rendering starts
	* @param actor The actor to add
	* @param part The model part corresponding to the actor
    */
    void addActorOffline(vtkActor* actor, ModelPart* part);


    /** 
	* @brief Issue a command to the VR thread
	* @param cmd The command to issue
	* @param value The value to pass with the command
    */
    void issueCommand(int cmd, double value = 0);

    /**
	* @brief Map a model part to an actor
	* @param actor The actor to map
	* @param part The model part to map
    */
    void addActorModelPartMapping(vtkActor* actor, ModelPart* part);

protected:
    /** This is a re-implementation of a QThread function
      */
    void run() override;

private:
    /* Standard VTK VR Classes */
    vtkSmartPointer<vtkOpenVRRenderWindow>              window;
    vtkSmartPointer<vtkOpenVRRenderWindowInteractor>    interactor;
    vtkSmartPointer<vtkOpenVRRenderer>                  renderer;
    vtkSmartPointer<vtkOpenVRCamera>                    camera;

    /* Use to synchronise passing of data to VR thread */
    QMutex                                              mutex;
    QWaitCondition                                      condition;

    /** List of actors that will need to be added to the VR scene */
    vtkSmartPointer<vtkActorCollection>                 actors;

    /** A timer to help implement animations and visual effects */
    std::chrono::time_point<std::chrono::steady_clock>  t_last;

    /** This will be set to false by the constructor, if it is set to true
      * by the GUI then the rendering will end
      */
    bool                                                endRender;

    /* Some variables to indicate animation actions to apply.
     *
     */
    double rotateX;         /*< Degrees to rotate around X axis (per time-step) */
    double rotateY;         /*< Degrees to rotate around Y axis (per time-step) */
    double rotateZ;         /*< Degrees to rotate around Z axis (per time-step) */

	/* A map to link actors to model parts */
    std::map<vtkActor*, ModelPart*> actorToModelPart;
};



#endif
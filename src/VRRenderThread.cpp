/**		@file VRRenderThread.cpp
  *
  *		EEEE2046 - Software Engineering & VR Project
  *
  *		Template to add VR rendering to your application.
  *
  *		P Evans 2022
  */

#include "VRRenderThread.h"


  /* Vtk headers */
#include <vtkActor.h>
#include <vtkOpenVRRenderWindow.h>				
#include <vtkOpenVRRenderWindowInteractor.h>	
#include <vtkOpenVRRenderer.h>					
#include <vtkOpenVRCamera.h>	

#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkNamedColors.h>
#include <vtkCylinderSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSTLReader.h>
#include <vtkDataSetmapper.h>
#include <vtkCallbackCommand.h>


/* The class constructor is called by MainWindow and runs in the primary program thread, this thread
 * will go on to handle the GUI (mouse clicks, etc). The OpenVRRenderWindowInteractor cannot be start()ed
 * in the constructor, as it will take control of the main thread to handle the VR interaction (headset
 * rotation etc. This means that a second thread is needed to handle the VR.
 */
VRRenderThread::VRRenderThread(QObject* parent)
{
	/* Initialise actor list */
	actors = vtkActorCollection::New();

	/* Initialise command variables */
	rotateX = 0.;
	rotateY = 0.;
	rotateZ = 0.;

	//TODO how do we initialise the command enum

	endRender = true;
	syncRender = false;
	removeFiltersFlag = false;
}


/* Standard destructor - this is important here as the class will be destroyed when the user
 * stops the VR thread, and recreated when the user starts it again. If class variables are
 * not deallocated properly then there will be a memory leak, where the program's total memory
 * usage will increase for each start/stop thread cycle.
 */
VRRenderThread::~VRRenderThread() {
	if (actors != nullptr)
		actors->Delete();
	if (renderer != nullptr)
		renderer->Delete();
	if (window != nullptr)
		window->Delete();
	if (camera != nullptr)
		camera->Delete();
	if (interactor != nullptr)
		interactor->Delete();
}


void VRRenderThread::addActorOffline(vtkActor* actor, ModelPart* part) {
	QMutexLocker locker(&mutex);
	/* Check to see if render thread is running */
	if (!this->isRunning()) {
		double* ac = actor->GetOrigin();

		actorMap[actor] = part;
		part->setOriginalData(actor->GetMapper()->GetInput());

		/* I have found that these initial transforms will position the FS
		 * car model in a sensible position but you can experiment
		 */
		actor->RotateX(-90);
		actor->AddPosition(-ac[0] + 0, -ac[1] - 100, -ac[2] - 200);

		actors->AddItem(actor);

	}
}



void VRRenderThread::issueCommand(int cmd, double value) {

	/* Update class variables according to command */
	switch (cmd) {

	case END_RENDER:
		this->endRender = true;
		break;

	case ROTATE_X:
		this->rotateX = value;
		break;

	case ROTATE_Y:
		this->rotateY = value;
		break;

	case ROTATE_Z:
		this->rotateZ = value;
		break;

	case SYNC_RENDER:
		this->syncRender = true;
		break;

	case REMOVE_FILTERS:
		this->removeFiltersFlag = true;
		break;
	}
}


//void VRRenderThread::syncVRActors(std::unordered_map<vtkActor*, ModelPart*>& mainSceneMap) {
//	// Find model parts in the main scene but not in the VR scene and add corresponding actors to the VR scene
//	QMutexLocker locker(&mutex);
//	for (const auto& pair : mainSceneMap) {
//		if (actorMap.find(pair.first) == actorMap.end()) {
//			// Add the actor to the VR scene
//			vtkSmartPointer<vtkActor> vrActor = pair.second->getNewActor();
//			renderer->AddActor(vrActor);
//			actorMap[vrActor] = pair.second;
//		}
//	}
//
//	// Find model parts in the VR scene but not in the main scene and remove corresponding actors from the VR scene
//	for (auto it = actorMap.begin(); it != actorMap.end(); /* no increment here */) {
//		if (mainSceneMap.find(it->first) == mainSceneMap.end()) {
//			// Remove the actor from the VR scene
//			renderer->RemoveActor(it->first);
//			it = actorMap.erase(it); // erase returns the iterator to the next element
//		}
//		else {
//			++it;
//		}
//	}
//}

void VRRenderThread::applyShrinkFilter(ModelPart* selectedPart)
{
	QMutexLocker locker(&mutex);
	if (!this->isRunning()) {
		return;
	}
	vtkActor* a =selectedPart->getVRActor();
	vtkSmartPointer<vtkShrinkFilter> shrinkFilter = vtkSmartPointer<vtkShrinkFilter>::New();
	shrinkFilter->SetInputData(a->GetMapper()->GetInput());
	shrinkFilter->SetShrinkFactor(0.5);
	shrinkFilter->Update();
	a->GetMapper()->SetInputConnection(shrinkFilter->GetOutputPort());
}

void VRRenderThread::applyClipFilter(ModelPart* selectedPart)
{
	QMutexLocker locker(&mutex);
	if (!this->isRunning()) {
		return;
	}
	vtkActor* a = selectedPart->getVRActor();
	vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
	plane->SetOrigin(0, 0, 0);
	plane->SetNormal(-1, 0, 0);
	vtkSmartPointer<vtkClipDataSet> clipFilter = vtkSmartPointer<vtkClipDataSet>::New();
	clipFilter->SetInputData(a->GetMapper()->GetInput());
	clipFilter->SetClipFunction(plane.Get());
	clipFilter->Update();
	a->GetMapper()->SetInputConnection(clipFilter->GetOutputPort());
}

void VRRenderThread::removeFilters()
{
	QMutexLocker locker(&mutex);
	vtkActorCollection* actorList = renderer->GetActors();
	vtkActor* a;
	actorList->InitTraversal();
	while ((a = (vtkActor*)actorList->GetNextActor())) {
		if (a != nullptr) {
			vtkSmartPointer<vtkTrivialProducer> producer = vtkSmartPointer<vtkTrivialProducer>::New();
			producer->SetOutput(actorMap[a]->getOriginalData());
			a->GetMapper()->SetInputConnection(producer->GetOutputPort());
		}
	}
}



/* This function runs in a separate thread. This means that the program
 * can fork into two separate execution paths. This thread is triggered by
 * calling VRRenderThread::start()
 */
void VRRenderThread::run() {
	/* You might want to edit the 3D model once VR has started, however VTK is not "thread safe".
	 * This means if you try to edit the VR model from the GUI thread while the VR thread is
	 * running, the program could become corrupted and crash. The solution is to get the VR thread
	 * to edit the model. Any decision to change the VR model will come fromthe user via the GUI thread,
	 * so there needs to be a mechanism to pass data from the GUi thread to the VR thread.
	 */

	vtkNew<vtkNamedColors> colors;

	// Set the background color.
	std::array<unsigned char, 4> bkg{ {26, 51, 102, 255} };
	colors->SetColor("BkgColor", bkg.data());


	// The renderer generates the image
	// which is then displayed on the render window.
	// It can be thought of as a scene to which the actor is added
	renderer = vtkOpenVRRenderer::New();


	renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());

	/* Loop through list of actors provided and add to scene */
	vtkActor* a;
	actors->InitTraversal();
	while ((a = (vtkActor*)actors->GetNextActor())) {
		renderer->AddActor(a);
	}

	/* The render window is the actual GUI window
	 * that appears on the computer screen
	 */
	window = vtkOpenVRRenderWindow::New();

	if (!window->IsHMDPresent()) {
		return;
	}

	window->Initialize();
	window->AddRenderer(renderer);

	/* Create Open VR Camera */
	camera = vtkOpenVRCamera::New();
	renderer->SetActiveCamera(camera);

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
	
	/* Create Skybox*/
	/*
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
	/* The render window interactor captures mouse events
	 * and will perform appropriate camera or actor manipulation
	 * depending on the nature of the events.
	 */
	interactor = vtkOpenVRRenderWindowInteractor::New();
	interactor->SetRenderWindow(window);
	interactor->Initialize();
	window->Render();

	/* Now start the VR - we will implement the command loop manually
	 * so it can be interrupted to make modifications to the actors
	 * (i.e. to implement animation)
	 */
	endRender = false;
	t_last = std::chrono::steady_clock::now();

	while (!interactor->GetDone() && !this->endRender) {
			interactor->DoOneEvent(window, renderer);


			/* Check to see if enough time has elapsed since last update
			 * This looks overcomplicated (and it is, C++ loves to make things unecessarily complicated!) but
			 * is really just checking if more than 20ms have elaspsed since the last animation step. The
			 * complications comes from the fact that numbers representing time on computers don't usually have
			 * standard second/millisecond units. Because everything is a class in C++, the converion from
			 * computer units to seconds/milliseconds ends up looking like what you see below.
			 *
			 * My choice of 20ms is arbitrary, if this value is too small the animation calculations could begin to
			 * interfere with the interator processes and make the simulation unresponsive. If it is too large
			 * the animations will be jerky. Play with the value to see what works best.
			 */
			if (std::chrono::duration_cast <std::chrono::milliseconds> (std::chrono::steady_clock::now() - t_last).count() > 20) {

				/* Do things that might need doing ... */
				vtkActorCollection* actorList = renderer->GetActors();
				vtkActor* a;

				/* X Rotation */
				actorList->InitTraversal();
				while ((a = (vtkActor*)actorList->GetNextActor())) {
					if (a != nullptr) {
						a->RotateX(rotateX);
					}
				}
				rotateX = 0;

				/* Y Rotation */
				actorList->InitTraversal();
				while ((a = (vtkActor*)actorList->GetNextActor())) {
					if (a != nullptr) {
						a->RotateY(rotateY);
					}
				}
				rotateY = 0;

				/* Z Rotation */
				actorList->InitTraversal();
				while ((a = (vtkActor*)actorList->GetNextActor())) {
					if (a != nullptr) {
						a->RotateZ(rotateZ);
					}
				}
				rotateZ = 0;

				if (syncRender)
				{
					mutex.lock();
					// Update all actors
					actors->InitTraversal();
					vtkActor* actor = nullptr;
					while ((actor = actors->GetNextActor()) != nullptr)
					{
						ModelPart* part = actorMap[actor];
						QColor colour = part->colour();
						bool visible = part->visible();
						actor->GetProperty()->SetColor(colour.redF(), colour.greenF(), colour.blueF());
						actor->SetVisibility(visible);
					}

					// Reset the command
					syncRender = false;
					mutex.unlock();
				}

				if (removeFiltersFlag)
				{
					removeFilters();
					removeFiltersFlag = false;
				}

				/* Remember time now */
				t_last = std::chrono::steady_clock::now();
			}
	}

	window->Finalize();
	renderer->RemoveAllViewProps();
	renderer->RemoveAllLights();
}
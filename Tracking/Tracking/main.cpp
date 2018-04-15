
//
#include "stdafx.h"
#include "detection.h"
#include "tracker.h"
#include "affichage.h"

#include <chrono>
#include <numeric>
#include <fstream>
#include <thread>

using namespace std;
using namespace cv;

#define WIDTH_RESIZE 360;
#define TRACKER_MAJ 4;


int main(int argc, const char * argv[])
{
	const short FPS = 25;
	const double every_second = 1;
	int frame_count = 0;
	VideoCapture video("C:/Users/Rafik/Downloads/ETS Cours/H18/INF749/Projet/projet_inf749/Tracking/Tracking/Videos/terrace1-c3.avi");; // Mettre 0 pour webcam
	int asup = 1800;


	// INITALISATION ****************************************************
	if (!video.isOpened())
	{
		cout << "Impossible d'ouvrir la video" << endl;
		return -1;
	}

	ostringstream fpsFlow;

	// Creation du tracker
	TrackingManager trackingManager = TrackingManager::getInstance();

	// Liste des humains à traquer
	vector<Rect2d> list_humans;
	vector<double> levelWeights;
	vector<int> rejectLevels;


	// Detection
	Mat frame;
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector()); // Retourne les coefficients du classifieur entrainé pour la detection de gens

	String fullbody_cascade_name = "D:/OpenCV/build/install_debug/etc/haarcascades/haarcascade_fullbody.xml";
	CascadeClassifier fullbody_cascade;
	// Chargement des cascades
	if (!fullbody_cascade.load(fullbody_cascade_name)) { printf("--(!)Erreur de chargement\n"); return -1; };

	video >> frame;
	frame_count++;
	cv::Size video_size = frame.size();
	float resize_factor = video_size.width / WIDTH_RESIZE;

	int count = FPS * every_second;

	int tracker_count = TRACKER_MAJ;
	int detection_count = 1;
	int tracker_cnt = 1;
	int maj_rectangles = 4;


	// DEBUT ****************************************************

	//Creation de la classe affichage
	//Affichage windows(resize_factor, FPS);
	Affichage * windows = new Affichage(resize_factor, FPS);

	//dans un nouveau thread, lancer la fonction d'affichage
	thread t1(&Affichage::afficher, windows);

	//On lance le traitement des frames
	while (video.read(frame) && asup > 0)
	{
		asup--;
		detection_count--;
		tracker_cnt--;
		maj_rectangles--;

		// On envoie la frame a l'affichage
		Mat tmp = frame.clone();
		windows->push_frame(tmp);
		cv::Mat resized;
		cv::resize(tmp, resized, cv::Size(video_size.width / resize_factor, video_size.height / resize_factor));

		// Si il est temps d'effectuer une détection (une fois toutes les 24 frames)
		if (detection_count == 0)
		{
			//Redimensionnement de la frame (pour avoir une detection plus rapide)
			//cv::resize(tmp, resized, cv::Size(video_size.width / resize_factor, video_size.height / resize_factor));

			// On effectue la détection
			detection2(fullbody_cascade, resized, list_humans, 1.05, levelWeights, rejectLevels, cv::Size(40, 80));

			// Algorithme NMS
			vector <Rect2d> list_objects_tracker = trackingManager.getListObjects();
			vector <Rect2d> list_objects, nms_list_objects, new_list_objects;

			for (int i = 0; i < list_humans.size(); i++)
				list_objects.push_back(list_humans[i]);

			for (int i = 0; i < list_objects_tracker.size(); i++)
				list_objects.push_back(list_objects_tracker[i]);

			nms(list_humans, list_objects_tracker, nms_list_objects, 0.2);

			for (int i = 0; i < nms_list_objects.size(); i++)
			{
				Rect2d r = nms_list_objects[i];
				int j;

				for (j = 0; j < list_objects_tracker.size(); j++)
					if (list_objects_tracker[j] == r)
						break;

				if (j == list_objects_tracker.size())
					new_list_objects.push_back(r);
			}

			// Initialisation du tracker avec les bons rectangles
			trackingManager.setListObjects(new_list_objects);
			trackingManager.initTracking();
			detection_count = 24;
		}

		// Si il est temps de mettre à jour le tracker (mais qu'aucune détection n'a eu lieu, sinon redondance)

		// Mise à jour du tracker
		trackingManager.launchTracking(resized);

		if (maj_rectangles == 0)
		{
			// On transmet les rectangles du tracker dans l'objet d'affichage

			vector<Rect2d> list_objects_tracker = trackingManager.getListObjects();
				
			windows->push_rectangles(list_objects_tracker);
			maj_rectangles = 4;
		}
	}

	cout << "Detections terminees ! En attente de la fin de l'affichage..." << endl;

	//windows->afficher();
	t1.join();

	trackingManager.endTracking();
}
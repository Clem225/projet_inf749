
//
#include "stdafx.h"
#include "detection.h"
//#include "tracking.h"
#include "tracker.h"

#include <chrono>
#include <numeric>

using namespace std;
using namespace cv;

#define WIDTH_RESIZE 360;
#define TRACKER_MAJ 4;

int main(int argc, const char * argv[])
{
	const short FPS = 50;
	const double every_second = 1;
	int frame_count = 0;
	VideoCapture video("Videos/terrace1-c3.avi"); // Mettre 0 pour webcam

	if (!video.isOpened())
	{
		cout << "Impossible d'ouvrir la video" << endl;
		return -1;
	}

	namedWindow("Tracking de la PLS", CV_WINDOW_AUTOSIZE);
	ostringstream fpsFlow;

	// Creation du tracker
	TrackingManager trackingManager = TrackingManager::getInstance();

	// Liste des humains à traquer
	vector<Rect2d> list_humans;


	// Detection
	Mat frame;
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector()); // Retourne les coefficients du classifieur entrainé pour la detection de gens

	String fullbody_cascade_name = "D:/OpenCV/build/install_debug/etc/haarcascades/haarcascade_fullbody.xml";
	CascadeClassifier fullbody_cascade;
	// Chargement des cascades
	if (!fullbody_cascade.load(fullbody_cascade_name)) { printf("--(!)Erreur de chargemet\n"); return -1; };

	video >> frame;
	frame_count++;
	cv::Size video_size = frame.size();
	float resize_factor = video_size.width / WIDTH_RESIZE;

	/*detection2(hog, frame, list_humans);
	trackingManager.setListObjects(list_humans);
	trackingManager.initTracking();*/
	
	vector<long long> temps;
	int count = FPS * every_second;

	int tracker_count = TRACKER_MAJ;

	while (video.read(frame))
	{
		auto debut = chrono::high_resolution_clock::now();

		// Timer
		int64 timer = getTickCount();

		frame_count++;

		//Redimensionnement de la frame (pour avoir une detection plus rapide)
		cv::Mat resized;
		cv::resize(frame, resized, cv::Size(video_size.width / resize_factor, video_size.height / resize_factor));

		count--;
		if (count == 0)
		{
			count = FPS * every_second;
			/*detection2(hog, resized, list_humans);
			trackingManager.setListObjects(list_humans);
			trackingManager.initTracking();*/

			// Detection et mesure du temps de detection
			auto start = chrono::high_resolution_clock::now();
			detection2(fullbody_cascade, resized, list_humans, 1.05, cv::Size(40, 80));
			//detection(hog, frame, list_humans);
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
			temps.push_back(duration.count());
			auto average = accumulate(temps.begin(), temps.end(), 0.0) / temps.size();

			//Affichage des informations
			cout << "Temps de detection moyen : " << average << endl;
			cout << "Nombre de personnes detectees : " << list_humans.size() << endl;
			cout << "Taille des boites : " << endl;

			unsigned int i = 0;
			for (vector<Rect2d>::iterator it = list_humans.begin(); it != list_humans.end(); it++, i++) {
				cout << "Rectangle n=" << i << " : " << "largeur = " << it->width << ", hauteur : " << it->height << endl;
			}
			cout << "\n" << endl; // Saut de ligne

			// Tracking
			trackingManager.setListObjects(list_humans);
			trackingManager.initTracking();
		}


		// Mettre à jour le tracker
		bool ok = false;
		tracker_count--;
		if (!tracker_count)
		{
			//cout << "MAJ , frame = " << frame_count << endl;
			ok = trackingManager.launchTracking(resized);
			tracker_count = TRACKER_MAJ;
		}

		cv::Rect2d bboxResized;

		//if (ok)
		{
			// BROUILLON FAUDRA FAIRE UNE CLASSE SPECIALE POUR DESSINER LES BOITES
			MultiTrackers multiTracker = trackingManager.getMultiTrackers();

			for_each(multiTracker.getMultiTracker().begin(), multiTracker.getMultiTracker().end(), [&](auto tracker) {

				bboxResized = tracker.get()->getBbox();

				bboxResized.x *= resize_factor;
				bboxResized.y *= resize_factor;
				bboxResized.width *= resize_factor;
				bboxResized.height *= resize_factor;

				rectangle(frame, bboxResized, Scalar(255, 0, 0), 2, 1);
			});
		}

		// On affiche le temps écoulé
		putText(frame, "Temps : " + to_string(double(frame_count) / 30.0) + "s", Point(100, 100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

		// Calcul des FPS
		double fps = getTickFrequency() / ((double)getTickCount() - timer);

		// On affiche les FPS
		fpsFlow << int(fps);
		putText(frame, "FPS : " + fpsFlow.str(), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);
		fpsFlow.str("");

		imshow("Tracking de la PLS", frame);

		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - debut);
		auto sec = 1000/FPS - duration.count();

		if (sec > 1)
			waitKey(sec);
		else
			waitKey(1);

		/*if (waitKey() >= 0)
			break;*/

	}


	/*// ------ DETECTION -------
	Mat frame;

	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector()); // Retourne les coefficients du classifieur entrainé pour la detection de gens

	String fullbody_cascade_name = "D:/OpenCV/build/install_debug/etc/haarcascades/haarcascade_fullbody.xml";
	CascadeClassifier fullbody_cascade;
	// Chargement des cascades
	if (!fullbody_cascade.load(fullbody_cascade_name)) { printf("--(!)Erreur de chargemet\n"); return -1; };

	video >> frame;
	cv::Size video_size = frame.size();
	float resize_factor = video_size.width / 360;

	frame_count++;

	// ******** TEST DE CLEMENT POUR LA DETECTION

	vector<long long> temps;
	int count = FPS * every_second;

	while (video.read(frame))
	{
	// Mise à jour du numéro de frame
	frame_count++;

	//Redimensionnement de la frame (pour avoir une detection plus rapide)
	cv::Mat resized;
	cv::resize(frame, resized, cv::Size(video_size.width / resize_factor, video_size.height / resize_factor));

	//Mise à jour du conteur
	count--;
	if (count == 0) // Si il doit y avoir une detection de faite sur cette frame
	{
	count = FPS * every_second; // Remise à zéro du conteur

	// Detection et mesure du temps de detection
	auto start = chrono::high_resolution_clock::now();
	detection2(fullbody_cascade, resized, list_humans, 1.05, cv::Size(40, 80));
	//detection(hog, frame, list_humans);
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
	temps.push_back(duration.count());
	auto average = accumulate(temps.begin(), temps.end(), 0.0) / temps.size();

	//Affichage des informations
	cout << "Temps de detection moyen : " << average << endl;
	cout << "Nombre de personnes detectees : " << list_humans.size() << endl;
	cout << "Taille des boites : " << endl;
	unsigned int i = 0;
	for (vector<Rect2d>::iterator it = list_humans.begin(); it != list_humans.end(); it++, i++) {
	cout << "Rectangle n=" << i << " : " << "largeur = " << it->width << ", hauteur : " << it->height << endl;
	// Remise à l'échellle originale (pour affichage ulterieur, puisque la detection est terminee)
	it->x *= resize_factor;
	it->y *= resize_factor;
	it->width *= resize_factor;
	it->height *= resize_factor;
	}
	cout << "\n" << endl; // Saut de ligne
	}

	// Dessiner les boites
	for (unsigned i = 0; i < list_humans.size(); i++)
	rectangle(frame, list_humans[i], Scalar(255, 0, 0), 2, 1);

	// On affiche le temps de vidéo écoulé
	putText(frame, "Temps : " + to_string(double(frame_count) / 30.0) + "s", Point(100, 100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

	imshow("test", frame);
	if (waitKey(20) >= 0)
	break;
	}*/




	/*const short FPS = 30;
	const double every_second = 3.5;
	int frame_count = 0;
	VideoCapture video("Videos/lolilol.mp4"); // Mettre 0 pour webcam
	//VideoCapture video(0); // Mettre 0 pour webcam


	if (!video.isOpened())
	{
		cout << "Impossible d'ouvrir la video" << endl;
		return -1;
	}

	namedWindow("Tracking de la PLS", CV_WINDOW_AUTOSIZE);
	ostringstream fpsFlow;

	// Creation du tracker
	string trackerAlgo = "KCF";
	MultiTracker trackers;

	// Liste des humains à traquer
	vector<Rect2d> list_humans;

	//Ptr<Tracker> tracker; // Equivalent à shared_ptr
	//tracker = TrackerKCF::create();  // KCF => meilleur en temps/performance

	// Detection
	Mat frame;
	HOGDescriptor hog;
	hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector()); // Retourne les coefficients du classifieur entrainé pour la detection de gens

	// Initialisation du tracker
	std::vector< Ptr<Tracker> > list_trackers; // Ptr = shared_ptr

	video >> frame;
	frame_count++;
	
	detection(hog, frame, list_humans, trackers);
	cout << list_humans.size() << endl;
	tracking(trackers, list_trackers, trackerAlgo, list_humans, frame);

	int count = FPS * every_second;

	while (video.read(frame))
	{
		frame_count++;
		count--;
		if (count == 0)
		{
			count = FPS * every_second;
			detection(hog, frame, list_humans, trackers);
			cout << list_humans.size() << endl;
			tracking(trackers, list_trackers, trackerAlgo, list_humans, frame);
		}

		// Timer
		int64 timer = getTickCount();

		// Mettre à jour le tracker
		bool ok = trackers.update(frame);

		// Calcul des FPS
		double fps = getTickFrequency() / ((double)getTickCount() - timer);

		if (ok)
		{
			// Dessiner les boites
			for (unsigned i = 0; i < trackers.getObjects().size(); i++)
				rectangle(frame, trackers.getObjects()[i], Scalar(255, 0, 0), 2, 1);
		}

		// On affiche les FPS
		fpsFlow << int(fps);
		putText(frame, "FPS : " + fpsFlow.str(), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);
		fpsFlow.str("");

		// On affiche le temps écoulé
		putText(frame, "Temps : " + to_string(double(frame_count)/30.0) + "s", Point(100, 100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

		imshow("Tracking de la PLS", frame);
		if (waitKey(20) >= 0)
			break;
	}*/

	/*while (video.read(frame))
	{
		// Timer
		int64 timer = getTickCount();

		vector<Rect> humans, humans2;

		double t = (double)getTickCount();
		hog.detectMultiScale(frame, humans, 0, Size(8, 8), Size(32, 32), 1.1, 2); // detection hog
		t = (double)getTickCount() - t;
		cout << "temps de detection = " << (t*1000. / getTickFrequency()) << " ms" << endl;

		unsigned int i, j;
		for (i = 0; i < humans.size(); i++)
		{
			Rect r = humans[i];	// on crée un rectangle pour l'humain i

			for (j = 0; j < humans.size(); j++)       // pas de petit rectangle dans un grand rectangle (pas de chevauchement)
				if (j != i && (r & humans[j]) == r) 
					break;

			if (j == humans.size())	
				humans2.push_back(r);
		}

		cout << humans2.size() << endl;

		// dessine rectangle pour chaque humain
		for (i = 0; i < humans2.size(); i++)
		{
			Rect r = humans2[i];
			r.x += cvRound(r.width*0.1);
			r.width = cvRound(r.width*0.8);
			r.y += cvRound(r.height*0.06);
			r.height = cvRound(r.height*0.9);

			Rect2d bbox(r.x, r.y, r.width, r.height);

			// Initialiser le tracker
			tracker->init(frame, bbox);

			//list_rect.push_back(bbox);

			//rectangle(frame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);

		}

		// Initialiser le tracker
		//tracker->init(frame, list_rect.at(0));

		// Mettre à jour le tracker
		bool ok = tracker->update(frame, list_rect.at(0));

		if (ok)
		{
			// Dessiner la boite
			rectangle(frame, list_rect.at(0), Scalar(255, 0, 0), 2, 1);
		}

		// Calcul des FPS
		double fps = getTickFrequency() / ((double)getTickCount() - timer);

		fpsFlow << int(fps);
		//cout << fps << endl;

		// On affiche les FPS
		putText(frame, "FPS : " + fpsFlow.str(), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);
		fpsFlow.str("");
		//fpsFlow.clear();

		imshow("Tracking de la PLS", frame);
		if (waitKey(20) >= 0)
			break;
	}*/

	return 0;
}

// programme de test des différent tracker

/*#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>

using namespace cv;
using namespace std;

// Convert to string
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()

int main(int argc, char **argv)
{

	// List of tracker types in OpenCV 3.2
	// NOTE : GOTURN implementation is buggy and does not work.
	string trackerTypes[6] = { "BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN" };
	// vector <string> trackerTypes(types, std::end(types));

	// Create a tracker
	string trackerType = trackerTypes[2];

	Ptr<Tracker> tracker;

#if (CV_MINOR_VERSION < 3)
	{
		tracker = Tracker::create(trackerType);
}
#else
	{
		if (trackerType == "BOOSTING")
			tracker = TrackerBoosting::create();
		if (trackerType == "MIL")
			tracker = TrackerMIL::create();
		if (trackerType == "KCF")
			tracker = TrackerKCF::create();
		if (trackerType == "TLD")
			tracker = TrackerTLD::create();
		if (trackerType == "MEDIANFLOW")
			tracker = TrackerMedianFlow::create();
		if (trackerType == "GOTURN")
			tracker = TrackerGOTURN::create();
	}
#endif
	// Read video
	VideoCapture video("videos/chaplin.mp4");

	// Exit if video is not opened
	if (!video.isOpened())
	{
		cout << "Could not read video file" << endl;
		return 1;

	}

	// Read first frame
	Mat frame;
	bool ok = video.read(frame);

	// Define initial boundibg box
	Rect2d bbox(287, 23, 86, 320);

	// Uncomment the line below to select a different bounding box
	bbox = selectROI(frame, false);

	// Display bounding box.
	rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);
	imshow("Tracking", frame);

	tracker->init(frame, bbox);

	while (video.read(frame))
	{
		// Start timer
		double timer = (double)getTickCount();

		// Update the tracking result
		bool ok = tracker->update(frame, bbox);

		// Calculate Frames per second (FPS)
		float fps = getTickFrequency() / ((double)getTickCount() - timer);

		if (ok)
		{
			// Tracking success : Draw the tracked object
			rectangle(frame, bbox, Scalar(255, 0, 0), 2, 1);
		}
		else
		{
			// Tracking failure detected.
			putText(frame, "Tracking failure detected", Point(100, 80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
		}

		// Display tracker type on frame
		putText(frame, trackerType + " Tracker", Point(100, 20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

		// Display FPS on frame
		putText(frame, "FPS : " + SSTR(int(fps)), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

		// Display frame.
		imshow("Tracking", frame);

		// Exit if ESC pressed.
		int k = waitKey(1);
		if (k == 27)
		{
			break;
		}

	}
}*/
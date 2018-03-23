
//
#include "stdafx.h"
#include "detection.h"
//#include "tracking.h"

#include "tracker.h"

using namespace std;
using namespace cv;

int main(int argc, const char * argv[])
{
	const short FPS = 30;
	const double every_second = 3.5;
	int frame_count = 0;
	VideoCapture video("Videos/lolilol.mp4"); // Mettre 0 pour webcam

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

	video >> frame;
	frame_count++;

	detection2(hog, frame, list_humans);
	trackingManager.setListObjects(list_humans);
	trackingManager.initTracking();
	
	int count = FPS * every_second;

	//cout << frame.size() << endl;

	while (video.read(frame))
	{
		frame_count++;
		count--;
		if (count == 0)
		{
			count = FPS * every_second;
			detection2(hog, frame, list_humans);
			cout << list_humans.size() << endl;

			trackingManager.setListObjects(list_humans);

			trackingManager.initTracking();
			cout << trackingManager.getMultiTrackers().getMultiTracker().size() << endl;
		}

		// Timer
		int64 timer = getTickCount();

		// Mettre à jour le tracker
		bool ok = trackingManager.launchTracking(frame);

		// Calcul des FPS
		double fps = getTickFrequency() / ((double)getTickCount() - timer);


		if (ok)
		{
			// BROUILLON FAUDRA FAIRE UNE CLASSE SPECIALE POUR DESSINER LES BOITES
			MultiTrackers multiTracker = trackingManager.getMultiTrackers();

			for_each(multiTracker.getMultiTracker().begin(), multiTracker.getMultiTracker().end(), [&frame](auto tracker) {
				rectangle(frame, tracker.get()->getBbox(), Scalar(255, 0, 0), 2, 1);
			});
		}

		// On affiche les FPS
		fpsFlow << int(fps);
		putText(frame, "FPS : " + fpsFlow.str(), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);
		fpsFlow.str("");

		// On affiche le temps écoulé
		putText(frame, "Temps : " + to_string(double(frame_count) / 30.0) + "s", Point(100, 100), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);

		imshow("Tracking de la PLS", frame);
		if (waitKey(20) >= 0)
			break;
	}



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
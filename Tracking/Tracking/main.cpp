
//
#include "stdafx.h"

#include <iostream>
#include <cstring>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

using namespace std;
using namespace cv;

Ptr<Tracker> createTracker(string name)
{
	Ptr<Tracker> tracker;//lol

	if (name == "KCF")
		tracker = cv::TrackerKCF::create();
	else if (name == "TLD")
		tracker = cv::TrackerTLD::create();
	else if (name == "BOOSTING")
		tracker = cv::TrackerBoosting::create();
	else if (name == "MEDIAN_FLOW")
		tracker = cv::TrackerMedianFlow::create();
	else if (name == "MIL")
		tracker = cv::TrackerMIL::create();
	else if (name == "GOTURN")
		tracker = cv::TrackerGOTURN::create();
	else if (name == "MOSSE")
		tracker = cv::TrackerMOSSE::create();
	else if (name == "CSRT")
		tracker = cv::TrackerCSRT::create();
	else
		CV_Error(Error::StsBadArg, "Nom de l'algorithme inconnu\n");

	return tracker;
}

int main(int argc, const char * argv[])
{
	//VideoCapture video("Videos/chaplin.mp4"); // Mettre 0 pour webcam
	VideoCapture video(0); // Mettre 0 pour webcam


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
	vector<Rect> humans, humans2;

	video >> frame;
	hog.detectMultiScale(frame, humans, 0, Size(8, 8), Size(32, 32), 1.1, 2); // detection hog

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

	// dessine rectangle pour chaque humain puis sauvegarde dans une liste
	for (i = 0; i < humans2.size(); i++)
	{
		Rect r = humans2[i];
		r.x += cvRound(r.width*0.1);
		r.width = cvRound(r.width*0.8);
		r.y += cvRound(r.height*0.07);
		r.height = cvRound(r.height*0.8);

		Rect2d bbox(r.x, r.y, r.width, r.height);
		list_humans.push_back(bbox);

		//rectangle(frame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
	}

	// Initialisation du tracker
	std::vector< Ptr<Tracker> > list_trackers; // Ptr = shared_ptr
	for (size_t i = 0; i < list_humans.size(); i++)
	{
		list_trackers.push_back(createTracker(trackerAlgo));
	}

	trackers.add(list_trackers, frame, list_humans);

	while (video.read(frame))
	{
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

		imshow("Tracking de la PLS", frame);
		if (waitKey(20) >= 0)
			break;
	}

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

	Ptr<Tracker> tracker; // Equivalent à shared_ptr

#if (CV_MINOR_VERSION < 3)
	{
		tracker = Tracker::create(trackerType);
	}
#else
	{
		if (trackerType == "BOOSTING")				// nul
			tracker = TrackerBoosting::create();
		if (trackerType == "MIL")					// hyper lent
			tracker = TrackerMIL::create(); 
		if (trackerType == "KCF")					// meilleur en temps/performance <<<<<
			tracker = TrackerKCF::create();
		if (trackerType == "TLD")					// assez lent
			tracker = TrackerTLD::create();
		if (trackerType == "MEDIANFLOW")			// hyper rapide mais performance nulle
			tracker = TrackerMedianFlow::create();
		if (trackerType == "GOTURN")				// marche pas
			tracker = TrackerGOTURN::create();
	}
#endif
	// Read video
	VideoCapture video("Videos/chaplin.mp4");

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
	//waitKey();

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
			cout << fps << endl;

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
	waitKey(0);
}*/
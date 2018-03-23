#include "stdafx.h"

using namespace std;
using namespace cv;

void detection(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans, MultiTracker &trackers)
{
	list_humans.clear();
	vector<Rect2d> old_humans = trackers.getObjects();
	vector<Rect> humans, humans2;
	hog.detectMultiScale(frame, humans, 0, Size(8, 8), Size(32, 32), 1.1, 2); // detection hog

	unsigned int i, j;
	for (i = 0; i < humans.size(); i++)
	{
		Rect r = humans[i];	// on crée un rectangle pour l'humain i

		for (j = 0; j < humans.size(); j++)       // pas de petit rectangle dans un grand rectangle (pas de chevauchement)
			if (j != i && (r & humans[j]) == r)
				break;

		if (j == humans.size()) //pas de chauvement dans les nouveaux rectangles
		{
			for (j = 0; j < old_humans.size(); j++)
				if (int(old_humans[j].x) == r.x && int(old_humans[j].y) == r.y && int(old_humans[j].width) == r.width && int(old_humans[j].height) == r.height)
					break;
			humans2.push_back(r);
		}
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
}


// Avec la nouvelle classe TrackingManager
void detection2(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans)
{
	list_humans.clear();
	vector<Rect> humans, humans2;
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
	}
}
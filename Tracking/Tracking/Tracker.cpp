// Tracking.cpp : classe implémentant l'algorithme de tracking
//

#include "stdafx.h"
#include "tracker.h"
#include <thread>
#include <chrono>
#include <mutex> 
using namespace std;

#define NOT_FOUND -1

/**** OneTracker ****/

OneTracker::OneTracker(const std::string& name, unsigned int objectID, cv::Rect2d bbox) : objectID(objectID), bbox(bbox), trackerActivated(false)
{
	//création du tracker
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
		CV_Error(cv::Error::StsBadArg, "Nom de l'algorithme inconnu\n");
}

bool OneTracker::initTracking(cv::Mat frame)
{
	if (frame.empty())
	{
		//CV_Error(cv::Error::StsBadArg, "La frame est vide\n");
		return false;
	}

	trackerActivated = tracker->init(frame, bbox);
	return trackerActivated;
}

bool OneTracker::updateTracking(cv::Mat frame)
{
	if (frame.empty())
	{
		//CV_Error(cv::Error::StsBadArg, "La frame est vide\n");
		return false;
	}

	// bbox est mise à jour
	return tracker->update(frame, bbox); // true = objet localisé dans la frame, false = non localisé = perte de l'objet
}

/**** MultiTrackers ****/

int MultiTrackers::getTracker(unsigned int object_id)
{
	MultiTrackers::vector_trackers::iterator it = std::find_if(multi_tracker.begin(), multi_tracker.end(), [&](auto tracker) -> bool {
		return (tracker.get()->getObjectID() == object_id);
	});

	// on renvoie sa position dans le vecteur
	if (it == multi_tracker.end())
		return NOT_FOUND; // = - 1
	else
		return it - multi_tracker.begin();
}

bool MultiTrackers::addTracker(cv::Rect2d init_bbox, unsigned int object_id)
{
	if (init_bbox.empty())
	{
		//CV_Error(cv::Error::StsBadArg, "Le rectangle est vide\n");
		return false;
	}

	// KCF par défaut = meilleur compromis temps/perf
	shared_ptr<OneTracker> newTracker(new OneTracker("KCF", object_id, init_bbox));
	multi_tracker.push_back(newTracker);

	return true;
}

bool MultiTrackers::deleteTracker(unsigned int object_id)
{
	int tracker_id = getTracker(object_id);

	if (tracker_id == NOT_FOUND)
	{
		//CV_Error(cv::Error::StsNoConv, "Le tracker associe a cet objet n'existe pas\n");
		return false;
	}

	// liberation de la mémoire du shared_ptr puis suppression du vecteur
	multi_tracker.at(tracker_id).reset();
	multi_tracker.erase(multi_tracker.begin() + tracker_id);

	return true;
}

std::vector<cv::Rect2d> MultiTrackers::getAllBoxes()
{
	vector<cv::Rect2d> allBoxes;

	for (int j = 0; j < multi_tracker.size(); j++)
		allBoxes.push_back(multi_tracker[j].get()->getBbox());

	return allBoxes;
}


/**** TrackingManager ****/

TrackingManager& TrackingManager::getInstance() noexcept
{
	static TrackingManager singleInstance;
	return singleInstance;
}

bool TrackingManager::initTracking(unsigned int object_id, cv::Rect2d init_bbox)
{
	if (multiTracker.addTracker(init_bbox, object_id))
	{
		return true;
	}
	else
	{
		//CV_Error(cv::Error::StsInternal, "Erreur lors de l'ajout d'un tracker\n");
		return false;
	}
}

bool TrackingManager::initTracking()
{
	bool ok;

	for (int i = 0; i < list_objects.size(); i++)
	{
		ok = initTracking(i, list_objects[i]);

		if (!ok)
			return false;
	}

	list_objects = multiTracker.getAllBoxes();

	return true;
}

bool TrackingManager::launchTracking(const cv::Mat& frame)
{
	if (frame.empty())
	{
		//CV_Error(cv::Error::StsBadArg, "La frame est vide\n");
		return false;
	}

	if (multiTracker.getMultiTracker().size() == 0)
	{
		//CV_Error(cv::Error::StsBadArg, "Le multiTracker est vide\n");
		return false;
	}

	setFrame(frame);

	// Initialisation de tous les trackers du multiTracker
	for_each(multiTracker.getMultiTracker().begin(), multiTracker.getMultiTracker().end(), [frame](auto tracker) {
		if (!(tracker.get()->getTrackerActivated()))
		{
			tracker.get()->initTracking(frame);
			tracker.get()->setTrackerActivated(true);
		}
	});

	// Multithreading
	vector<thread> threads;
	//mutex mtx;

	for_each(multiTracker.getMultiTracker().begin(), multiTracker.getMultiTracker().end(), [&threads, frame, this](auto tracker) {
		threads.emplace_back(thread{
			[tracker, frame, this]() {
				bool test = tracker.get()->updateTracking(frame);

				// perte de l'objet par le tracker => suppression du tracker
				if (!test)
				{
					//mtx.lock();
					multiTracker.deleteTracker(tracker.get()->getObjectID()); // gérer le partage
					//mtx.unlock();
				}
			} 
		});
	});

	for (int i = 0; i < threads.size(); i++)
		threads[i].join();
	
	list_objects = multiTracker.getAllBoxes();

	return true;
}

void TrackingManager::endTracking()
{
	for_each(multiTracker.getMultiTracker().begin(), multiTracker.getMultiTracker().end(), [](auto tracker) {
		tracker.reset();
	});

	cout << "Fin du programme de tracking" << endl;
}

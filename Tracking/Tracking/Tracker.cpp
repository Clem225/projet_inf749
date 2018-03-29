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
		CV_Error(cv::Error::StsBadArg, "La frame est vide\n");
		return false;
	}

	trackerActivated = tracker->init(frame, bbox);
	return trackerActivated;
}

bool OneTracker::updateTracking(cv::Mat frame)
{
	if (frame.empty())
	{
		CV_Error(cv::Error::StsBadArg, "La frame est vide\n");
		return false;
	}

	//auto avant = chrono::high_resolution_clock::now();
	return tracker->update(frame, bbox);	// true = localisé dans la frame, false = non localisé // bbox est mise à jour
	//auto apres = chrono::high_resolution_clock::now();

	//cout << chrono::duration_cast<chrono::milliseconds>(apres - avant).count() << endl;

	//return true;
}


/**** MultiTrackers ****/

int MultiTrackers::getTracker(unsigned int object_id)
{
	// lambda-expression juste pour faire genre on a compris son cours
	/*auto f = [&](auto tracker) -> bool
	{
			return (tracker.get()->getObjectID() == object_id); //tracker = shared_ptr<OneTracker> / get => OneTracker*
	};*/

	// on cherche le tracker qui valide la lambda-expression
	//MultiTrackers::vector_trackers::iterator it = std::find_if(multi_tracker.begin(), multi_tracker.end(), f);

	// c'est + rapide ça askip
	MultiTrackers::vector_trackers::iterator it = std::find_if(multi_tracker.begin(), multi_tracker.end(), [&](auto tracker) -> bool {
		return (tracker.get()->getObjectID() == object_id);
	});

	cout << "TEST\n";

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
		CV_Error(cv::Error::StsBadArg, "Le rectangle est vide\n");
		return false;
	}

	int tracker_id = getTracker(object_id);

	if (tracker_id != NOT_FOUND)
	{
		//CV_Error(cv::Error::StsNoConv, "Le tracker existe deja\n");
		return false;
	}

	// KCF par défaut
	shared_ptr<OneTracker> newTracker(new OneTracker("KCF", object_id, init_bbox));
	multi_tracker.push_back(newTracker);

	//cout << "test" << endl;

	return true;
}

bool MultiTrackers::deleteTracker(unsigned int object_id)
{
	cout << "DELETE TRACKER num : " << object_id << endl;

	int tracker_id = getTracker(object_id);

	if (tracker_id == NOT_FOUND)
	{
		CV_Error(cv::Error::StsNoConv, "Le tracker associe a cet objet n'existe pas\n");
		return false;
	}

	// liberation de la mémoire du shared_ptr puis suppression du vecteur
	multi_tracker.at(tracker_id).reset();
	multi_tracker.erase(multi_tracker.begin() + tracker_id);

	return true;
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
		return true;
	else
	{
		//CV_Error(cv::Error::StsInternal, "Erreur lors de l'ajout d'un tracker\n");
		//return false;
	}
}

// TODO : supprimer les boites qui sont deja presentes
bool TrackingManager::initTracking()
{
	bool ok;

	//multiTracker.getMultiTracker().clear(); // A VERIFIER

	for (int i = 0; i < list_objects.size(); i++)
	{
		ok = initTracking(i, list_objects[i]);

		//if (!ok)
		//	return false;
	}

	return true;
}

bool TrackingManager::launchTracking(const cv::Mat& frame)
{
	if (frame.empty())
	{
		CV_Error(cv::Error::StsBadArg, "La frame est vide\n");
		return false;
	}

	if (multiTracker.getMultiTracker().size() == 0)
	{
		//CV_Error(cv::Error::StsBadArg, "Le multiTracker est vide\n");
		return false;
	}

	setFrame(frame);

	// Initialisation de tous les trackers du multiTracker
	for_each(multiTracker.getMultiTracker().begin(), multiTracker.getMultiTracker().end(), [&](auto tracker) {
		if (!(tracker.get()->getTrackerActivated()))
		{
			tracker.get()->initTracking(frame);
			tracker.get()->setTrackerActivated(true);
		}
	});

	// Multithreading
	vector<thread> threads;
	bool test = false;

	mutex mtx;

	for_each(multiTracker.getMultiTracker().begin(), multiTracker.getMultiTracker().end(), [&threads, frame, &test, this, &mtx](auto tracker) {
		threads.emplace_back(thread{
			[tracker, frame, &test, this, &mtx]() {
				test = tracker.get()->updateTracking(frame);
				if (!test)
				{
					//cout << "delete" << endl;
					mtx.lock();
					multiTracker.deleteTracker(tracker.get()->getObjectID()); // gérer le partage
					mtx.unlock();
				}
			} 
		});
	});

	//cout << "nb threads : " << threads.size() << endl;

	for (int i = 0; i < threads.size(); i++)
		threads[i].join();
	
	return true;
}

/*void TrackingManager::deleteBbox()
{
	MultiTrackers::vector_trackers multiTracker = multiTracker.getMultiTrackers();

	for (int i = 0; i < list_objects.size(); i++)
	{
		for(int j = 0; j < multiTracker.getMultiTracker().size(); j++)
		est_superpose(list_object[i], )
	}
}*/

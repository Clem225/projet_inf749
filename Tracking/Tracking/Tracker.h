#pragma once

#include <memory>
#include <atomic>

// Un tracker = une cible
class OneTracker 
{

private:
	cv::Ptr<cv::Tracker> tracker; 
	unsigned int objectID; // Identifiant de la cible
	cv::Rect2d bbox; // Rectangle de la cible
	bool trackerActivated; // Si le tracker est initialisé

public:
	OneTracker(const std::string& name, unsigned int objectID, cv::Rect2d bbox);

	// Getters
	unsigned int getObjectID() const { return objectID; }
	cv::Rect2d getBbox() const { return bbox; }
	bool getTrackerActivated() const { return trackerActivated; }

	// Setters
	void setObjectID(unsigned int id) { objectID = id; }
	void setBbox(cv::Rect2d new_bbox) { bbox = new_bbox; }
	void setTrackerActivated(bool activated) { trackerActivated = activated; }

	// Lancement du tracker
	bool initTracking(cv::Mat frame);

	// Mise à jour du tracker
	bool updateTracking(cv::Mat frame);
};


// Classe contenant les trackers
class MultiTrackers 
{

public:
	using vector_trackers = std::vector< std::shared_ptr<OneTracker> >;

private:
	// Vecteur des trackers
	vector_trackers multi_tracker;

public:
	
    // Getter
	vector_trackers& getMultiTracker() { return multi_tracker; }
	size_t nbTrackers() const { return multi_tracker.size(); }

	// On trouve le tracker qui suit l'objet id
	int getTracker(unsigned int object_id);

	// On ajoute un tracker au vecteur
	bool addTracker(cv::Rect2d init_bbox, unsigned int object_id);

	// On delete le tracker du vecteur qui suit l'objet id
	bool deleteTracker(unsigned int object_id);

	// Renvoie toutes les boxes du multitracker
	std::vector<cv::Rect2d> getAllBoxes();

};


// Classe princiale => Manager des trackers
class TrackingManager
{
private:
	MultiTrackers multiTracker;
	cv::Mat frame;
	std::vector<cv::Rect2d> list_objects;

	TrackingManager() = default;

public:
	// Singleton
	static TrackingManager& getInstance() noexcept;

	// Getters
	MultiTrackers getMultiTrackers() const { return multiTracker; }
	cv::Mat getFrame() const { return frame; }
	std::vector<cv::Rect2d> getListObjects() const { return list_objects; }

	// Setters
	void setFrame(const cv::Mat new_frame) { frame = new_frame; }
	void setListObjects(const std::vector<cv::Rect2d> new_list) { list_objects = new_list; }


	// Initialisation d'un tracker pour un seul objet
	bool initTracking(unsigned int object_id, cv::Rect2d init_bbox);
	// Initialisation d'un tracker pour toute la liste d'objets
	bool initTracking();

	// Lancement du tracking sur tous les objets
	bool launchTracking(const cv::Mat& frame);

	// Nettoyage des trackers => fin du programme
	void endTracking();
};
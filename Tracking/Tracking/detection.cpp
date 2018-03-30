#include "stdafx.h"

using namespace std;
using namespace cv;


void detection(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans)
{
	list_humans.clear();
	vector<Rect> humans, humans2;
	hog.detectMultiScale(frame, humans, 0, Size(8, 8), Size(32, 32), 1.1, 2); // detection hog

	unsigned int i, j;
	for (i = 0; i < humans.size(); i++)
	{
		Rect r = humans[i];	// on crée un rectangle pour l'humain i

		for (j = 0; j < humans.size(); j++)       // pas de petit rectangle dans un grand rectangle (pas de chevauchement)
			if (j != i && (r & humans[j]).area() >= 0)
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
}

void detection2(CascadeClassifier &classif, Mat &frame, vector<Rect2d> &detect_list, double scaleFactor, vector<double>& levelWeights, vector<int>& rejectLevels, cv::Size min, cv::Size max)
{
	detect_list.clear();
	levelWeights.clear();
	rejectLevels.clear();

	//Passage en N&B et equalisation dhistogramme
	//Mat frame_gray;
	//cvtColor(frame, frame_gray, CV_BGR2GRAY);
	//equalizeHist(frame_gray, frame_gray);

	//-- Detection
	vector<Rect> humans, humans2;
	classif.detectMultiScale(frame, humans, rejectLevels, levelWeights, scaleFactor, 3, 0, min, max, true);

	/*for (int i = 0; i < levelWeights.size(); ++i)
	{
		cout << "levelWeights :" << levelWeights[i] << endl;
		cout << "rejectLevels :" << rejectLevels[i] << endl;
	}*/
		

	// -- Vérif des rectangles
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
		detect_list.push_back(bbox);

		//rectangle(frame, r.tl(), r.br(), cv::Scalar(0, 255, 0), 2);
	}
}


/* IL FAUT PRENDRE QUE LES BOITES DU TRACKER */
void nms(const std::vector<cv::Rect2d>& srcRects, std::vector<cv::Rect2d>& resRects, float thresh, int neighbors = 0)
{
	resRects.clear();

	const size_t size = srcRects.size();
	if (!size)
	{
		return;
	}

	// Sort the bounding boxes by the bottom - right y - coordinate of the bounding box
	std::multimap<int, size_t> idxs;
	for (size_t i = 0; i < size; ++i)
	{
		idxs.insert(std::pair<int, size_t>(srcRects[i].br().y, i)); // br = bottom right corner
	}

	// keep looping while some indexes still remain in the indexes list
	while (idxs.size() > 0)
	{
		// grab the last rectangle
		auto lastElem = --std::end(idxs);
		const cv::Rect2d& rect1 = srcRects[lastElem->second];

		int neigborsCount = 0;

		idxs.erase(lastElem);

		for (auto pos = std::begin(idxs); pos != std::end(idxs); )
		{
			// grab the current rectangle
			const cv::Rect2d& rect2 = srcRects[pos->second];

			double intArea = (rect1 & rect2).area();
			double unionArea = rect1.area() + rect2.area() - intArea;
			double overlap = intArea / unionArea;

			cout << rect1 << " - " << rect2 << " OVERLAP : " << overlap << endl;

			// if there is sufficient overlap, suppress the current bounding box
			if (overlap > thresh)
			{
				pos = idxs.erase(pos);
				++neigborsCount;
			}
			else
			{
				++pos;
			}
		}
		if (neigborsCount >= neighbors)
		{
			resRects.push_back(rect1);
		}
	}
}

void nms2(const std::vector<cv::Rect2d>& srcRects, const std::vector<float>& scores, std::vector<cv::Rect2d>& resRects, float thresh, int neighbors = 0, float minScoresSum = 0.f)
{
	resRects.clear();

	const size_t size = srcRects.size();
	if (!size)
	{
		return;
	}

	assert(srcRects.size() == scores.size());

	// Sort the bounding boxes by the detection score
	std::multimap<float, size_t> idxs;
	for (size_t i = 0; i < size; ++i)
	{
		idxs.insert(std::pair<float, size_t>(scores[i], i));
	}

	// keep looping while some indexes still remain in the indexes list
	while (idxs.size() > 0)
	{
		// grab the last rectangle
		auto lastElem = --std::end(idxs);
		const cv::Rect2d& rect1 = srcRects[lastElem->second];

		int neigborsCount = 0;
		float scoresSum = lastElem->first;

		idxs.erase(lastElem);

		for (auto pos = std::begin(idxs); pos != std::end(idxs); )
		{
			// grab the current rectangle
			const cv::Rect2d& rect2 = srcRects[pos->second];

			double intArea = (rect1 & rect2).area();
			double unionArea = rect1.area() + rect2.area() - intArea;
			double overlap = intArea / unionArea;

			// if there is sufficient overlap, suppress the current bounding box
			if (overlap > thresh)
			{
				scoresSum += pos->first;
				pos = idxs.erase(pos);
				++neigborsCount;
			}
			else
			{
				++pos;
			}
		}
		if (neigborsCount >= neighbors &&
			scoresSum >= minScoresSum)
		{
			resRects.push_back(rect1);
		}
	}
}

bool est_superpose(const cv::Rect2d &r1, const cv::Rect2d &r2)
{
	auto x1 = r1.x;
	auto y1 = r1.y;
	auto width1 = r1.width;
	auto height1 = r1.height;

	auto x2 = r2.x;
	auto y2 = r2.y;
	auto width2 = r2.width;
	auto height2 = r2.height;

	if (((x2 > x1 + width1 / 2) || (x2 < x1 - width1 / 2)) && (((y2 > y1 + height1 / 2) || (y2 < y1 - height1 / 2))))
		return false;
	else
		return true;
}


cv::Rect2d choix_rectangle(vector<cv::Rect2d> rec_superposes, vector<double> scores)
{
	double score_max = 0;
	int indice = 0;
	for (int i = 0; i < rec_superposes.size(); i++)
	{
		if (scores[i]>score_max)
		{
			score_max = scores[i];
			indice = i;
		}
	}
	return rec_superposes[indice];
}

// Avec la nouvelle classe TrackingManager
/*void detection2(HOGDescriptor &hog, Mat &frame, vector<Rect2d> &list_humans)
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
}*/
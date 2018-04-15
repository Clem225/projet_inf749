#include "stdafx.h"
#include <algorithm>    // std::find

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
	}
}

void nms(const std::vector<cv::Rect2d>& srcDetect, const std::vector<cv::Rect2d>& srcTrack, std::vector<cv::Rect2d>& resRects, float thresh, int neighbors = 0)
{
	resRects.clear();

	std::vector<cv::Rect2d> srcRects = srcDetect;
	for (int i = 0; i < srcTrack.size(); i++)
		srcRects.push_back(srcTrack[i]);

	const size_t size = srcRects.size();
	if (!size)
	{
		return;
	}

	// Trie des boites englobantes selon la coordonnée y inférieure droite
	std::multimap<int, size_t> idxs;
	for (size_t i = 0; i < size; ++i)
	{
		idxs.insert(std::pair<int, size_t>(srcRects[i].br().y, i)); // br = bottom right corner
	}


	vector<Rect2d> savedBoxes;

	// parcours du tableau tant qu'il n'est pas vide
	while (idxs.size() > 0)
	{
		// grab the last rectangle
		auto lastElem = --std::end(idxs);
		const cv::Rect2d& rect1 = srcRects[lastElem->second];

		int neigborsCount = 0;

		idxs.erase(lastElem);

		for (auto pos = std::begin(idxs); pos != std::end(idxs); )
		{
			// on prend le rectangle courant
			const cv::Rect2d& rect2 = srcRects[pos->second];

			double intArea = (rect1 & rect2).area();
			double unionArea = rect1.area() + rect2.area() - intArea;
			double overlap = intArea / unionArea;

			// s'il y a un chevauchement suffisant, on supprime la boite actuelle (en la sauvegardant si besoin)
			if (overlap > thresh)
			{
				if(savedBoxes.empty())
					if(find(srcTrack.begin(), srcTrack.end(), rect2) != srcTrack.end())
						savedBoxes.push_back(rect2);

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

			if (savedBoxes.size() > 0)
			{
				resRects.push_back(savedBoxes[0]);
				savedBoxes.clear();
			}
			else
				resRects.push_back(rect1);
		}
	}
}

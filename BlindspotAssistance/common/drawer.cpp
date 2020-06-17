#include <opencv2/opencv.hpp>
#include <iostream>
#include "drawer.hpp"

enum {
	STREETS = 0,
};

const cv::Scalar STREETS_COLOR = cv::Scalar(0, 0, 255);
std::vector<cv::Point> vertices;
cv::Mat img;


cv::Scalar getScnColor(RegionsOfInterest scn)
{
	cv::Scalar color = cv::Scalar(0,0,0);
	switch(scn.state) {
		case STREETS:
			color = STREETS_COLOR;
		default:
			std::cout<<"Something is broken"<<std::endl;
			break;
	}
	return color;
}

void drawVertices(RegionsOfInterest *scn)
{
	cv::Scalar color = getScnColor(*scn);
	scn->aux = scn->orig.clone();
	if (scn->vertices.size()>0) {
		// Second, or later click, draw all lines to previous vertex
		for (int i = scn->vertices.size()-1; i > 0; i--){
			cv::line(scn->aux,scn->vertices[i-1],scn->vertices[i],color,2);
		}
	}
}

void CallBDraw(int event, int x, int y, int flags, void *scn)
{
	RegionsOfInterest* scene = (RegionsOfInterest*) scn;
	RegionsOfInterest& sceneRef = *scene;

	if(event==cv::EVENT_LBUTTONDOWN){
		std::cout << "Left mouse button clicked at (" << x << ", " << y << ")" << std::endl;
		sceneRef.vertices.push_back(cv::Point(x,y));
		drawVertices(scene);
	}
}

void CallBackDraw(int event, int x, int y, int flags, void *userdata)
{
	if(event==cv::EVENT_LBUTTONDOWN){
      std::cout << "Left mouse button clicked at (" << x << ", " << y << ")" << std::endl;
      if(vertices.size()==0){
         // First click - just draw point
         img.at<cv::Vec3b>(x,y)=cv::Vec3b(255,0,0);
      } else {
         // Second, or later click, draw line to previous vertex
         line(img,cv::Point(x,y),vertices[vertices.size()-1],cv::Scalar(0,0,0));
      }
      vertices.push_back(cv::Point(x,y));
      return;
   }
}

bool closePolygon(RegionsOfInterest *scn)
{
	RegionsOfInterest* scene = (RegionsOfInterest*) scn;
	RegionsOfInterest& sceneRef = *scene;
	cv::Scalar color;
	double alpha = 0.3;

	if(sceneRef.vertices.size()<2){
		std::cout << "You need a minimum of three points!" << std::endl;
		return false;
	}

	color = getScnColor(sceneRef);
	// Close polygon
	cv::line(sceneRef.aux,sceneRef.vertices[sceneRef.vertices.size()-1],sceneRef.vertices[0],color,2);

	// Mask is black with white where our ROI is
	cv::Mat roi(cv::Size(sceneRef.orig.cols, sceneRef.orig.rows), sceneRef.orig.type(), cv::Scalar(0));
	std::vector< std::vector< cv::Point > > pts{sceneRef.vertices};
	cv::Mat roi2 = roi.clone();
	cv::fillPoly(roi2,pts,cv::Scalar(255, 255, 255));
	cv::fillPoly(roi,pts,color);
	int key = 'x';
	switch(sceneRef.state) {
		case STREETS:
			std::cout<<"Define orientation, (n, s, e, w)" << std::endl;
			while (key != 'n' && key != 's' && key != 'e' && key !='w') {
				key = cv::waitKey();
			}
			sceneRef.streets.push_back(std::make_pair(roi, key));
			sceneRef.mask_streets.push_back(std::make_pair(roi2, key));
			break;
		default:
			std::cout<<"Something is broken"<<std::endl;
			break;
	}
	cv::addWeighted(roi, alpha, sceneRef.out, 1.0, 0.0, sceneRef.out);
	sceneRef.vertices.clear();
	return true;
}

int DrawAreasOfInterest(const cv::String & winname, RegionsOfInterest *scn)
{
	bool finished = false;
	bool can_finish = true;
	RegionsOfInterest* scene = (RegionsOfInterest*) scn;
	RegionsOfInterest& sceneRef = *scene;

	sceneRef.aux = sceneRef.orig;
	std::cout<<"Draw streets (S), sidewalks(W), crosswalks (Z). To draw next area, press (N) or to finish drawing, press (F)." << std::endl;
	while(!finished){
		cv::imshow(winname, sceneRef.aux);
		switch (cv::waitKey(1)) {
			case 'S':
				if(can_finish) {
					sceneRef.state = STREETS;
					can_finish = false;
				}
				break;
			case 'N':
				std::cout<<"Draw streets (S), sidewalks(W), crosswalks (Z). To draw next area, press (N) or to finish drawing, press (F)." << std::endl;
				can_finish = closePolygon(scn);
				break;
			case 'F':
				if (can_finish) {
					finished=true;
				}
				break;
			case 8: // Del
				if (sceneRef.vertices.size() > 0) {
					sceneRef.vertices.pop_back();
				}
				drawVertices(scene);
				break;
			case 27: // Esc
				return -1;
			default:
				break;
		}
	}
	return 0;
}

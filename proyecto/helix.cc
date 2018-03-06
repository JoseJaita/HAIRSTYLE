#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

const double pi = acos(-1);


Mat skeletization(Mat& src){
   Mat img = src.clone();
   threshold(img, img, 0, 255, THRESH_BINARY_INV|THRESH_OTSU);

   //threshold(img, img, 20, 255, THRESH_BINARY_INV);
   Mat element2 = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(7, 7));
   dilate(img,img,element2);
   dilate(img,img,element2);
   erode(img,img,element2);
   erode(img,img,element2);

  
   Mat skel(img.size(), CV_8UC1, cv::Scalar(0));
   Mat temp(img.size(), CV_8UC1);

   Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));

   bool done;
   do{
      cv::morphologyEx(img, temp, cv::MORPH_OPEN, element);
      cv::bitwise_not(temp, temp);
      cv::bitwise_and(img, temp, temp);
      cv::bitwise_or(skel, temp, skel);
      cv::erode(img, img, element);
      
      double max;
      cv::minMaxLoc(img, 0, &max);
      done = (max == 0);
   } while (!done);
   return skel;
}


void extractPoints(Mat& img, vector<Point>& points){
   points.clear();
   for(int r = 0; r < img.rows; r++){
      for(int c = 0; c<img.cols;c++){
	 // find curve
	 if (img.at<unsigned char>(r,c) == 255){
	    points.push_back(Point(c, r));
	    break;
	 }
      }
   }
}

void findMaxCurvature(const vector<Point>& points, vector<Point>& max_points){
  max_points.clear();
  int dir = 0, acc = 0;
  Point last = points[0];
  Point min_p, max_p;

  int i = 0;
  for (auto p : points) {
    if (i<100){
      cout << dir<<" " <<acc <<" " <<p<<" "<<last<< "\n";
    }
    // curve O+
    if (p.x > last.x){
      cout <<"+" <<endl;
      if (dir == 1) {
	acc = 0;
	max_p = p;
      }
      if (dir == -1){
	acc++;
	if (acc>10){
	  acc = 0; dir = +1; max_points.push_back(min_p);
	}
      }
      if (dir == 0){
	acc++; if (acc>10) {dir = +1; acc = 0;}
      }
    }
    // curve O-
    if (p.x < last.x){
      cout << "-" << "\n";
      if (dir == -1) {
	acc = 0;
	min_p = p;
      }
      if (dir == 1){
	acc++;
	if (acc>10){
	  acc = 0; dir = -1;
	  max_points.push_back(max_p);
	}
      }if (dir == 0){
	acc++; if (acc>10) {dir = -1; acc = 0;}}
    }
    last = p;
  }
}


void generateParametricEqu(const vector<Point>& points,
			   const vector<Point>& max_curv,
			   vector<Point>& helix_points){
  helix_points.clear();
  
  // first point
  Point pvs1 = max_curv[0];
  Point pvs1_0 = points[0];
  auto p1 = find(points.begin(), points.end(), pvs1);
  auto p2 = find_if(p1, points.end(), [&pvs1_0](const auto p){return p.x < pvs1_0.x;});
  Point pvs1_1 = *p2;

  cout << "pvs1_0: "<< pvs1_0 << "\n";
  cout << "pvs1_1: "<< pvs1_1 << "\n";
  
  // align local coordinate frame
  pvs1 -= pvs1_0;
  cout << "pvs1: "<< pvs1 << "\n";
  
  // parametric equations
  int xs = pvs1.x;
  int ys = pvs1.y;
  int delta_y = pvs1_1.y-pvs1_0.y; // delta_y == 2*ys should be
  double b = delta_y/pi;
  double a = xs;

  // show result using parametric equations
  double delta_t = pi/(delta_y);
  for (int i = 0; i < delta_y; i++) {
    Point p (int(a*sin(delta_t*i)), i);
    helix_points.push_back(p+pvs1_0);
  }
  
}



int main(int argc, char *argv[]){
  Mat img = imread("images/export2.png", 0);
  Mat skel, res2;

  
  if (img.empty()){cout << "can't read image"<<endl; return -1;}

  
  skel = skeletization(img);
  vector<Point> points, max_points;
  
  extractPoints(skel, points);
  findMaxCurvature(points, max_points);
  for (auto & p : max_points) {
    cout << p << "\n";
  }
  // draw skel with points extraidos
  Mat skel_points_img(skel.size(), CV_8UC1, cv::Scalar(0));
  for (auto& p : points){
     skel_points_img.at<unsigned char>(p.y, p.x) = 255;
  }

  vector<Point> helix_points;
  generateParametricEqu(points,max_points,helix_points);
  for (auto&p : helix_points) {
    skel_points_img.at<unsigned char>(p.y, p.x) = 255;
  }

  


  // sow results
  namedWindow("image", 0);
  namedWindow("skel", 0);
  namedWindow("endp", 0);
  imshow("image", img);
  imshow("skel", skel);
  imshow("endp", skel_points_img);
  
  waitKey(0);
  destroyAllWindows();
  
  return 0;
}


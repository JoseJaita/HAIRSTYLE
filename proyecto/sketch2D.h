#ifndef SKETCH2D_H
#define SKETCH2D_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <string>
#include <omp.h>
#include <iostream>
#include <utility>
#include <cmath>

using namespace cv;
using namespace std;

const float pi = acos(-1);

typedef vector<pair<float,float>> waves_t;

class Sketch2D{
  Mat image_g;
  Mat skel_g;
  vector<Point> points_g;
  vector<Point> max_points_g;
  waves_t waves_g;
  
  Mat skeletization(Mat& src);
  void extractPoints(Mat& img, vector<Point>& points);
  void findMaxCurvature(const vector<Point>& points, vector<Point>& max_points);
 

 public:
  Sketch2D(string filename);
  void processing();
  waves_t getWaves()const;
  
};

#endif // SKETCH2D_H

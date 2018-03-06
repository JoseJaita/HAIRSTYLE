#include "sketch2D.h"


Sketch2D::Sketch2D(string filename){
  image_g = imread(filename,0);
  if (image_g.empty())
    cout << "can't read image" << "\n";
}

waves_t Sketch2D::getWaves()const{return waves_g;}

Mat Sketch2D::skeletization(Mat& src){
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


void Sketch2D::extractPoints(Mat& img, vector<Point>& points){
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

void Sketch2D::findMaxCurvature(const vector<Point>& points, vector<Point>& max_points){
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
			   waves_t& waves){
  //helix_points.clear();
  Point last = points[0];
  
  for (int i = 0; i < max_curv.size(); i++) {
    Point pvs1 = max_curv[i];
    // first boundary
    Point pvs1_0 = last;
    int dir = (pvs1.x-pvs1_0.x>0) ? 1 : 0;// 0 -> negative wave, 1-> positive wave
    
    
    auto p1 = find(points.begin(), points.end(), pvs1);
    auto p2 = (dir) ? find_if(p1, points.end(), [&pvs1_0](const auto p){return p.x < pvs1_0.x;}):
      find_if(p1, points.end(), [&pvs1_0](const auto p){return p.x > pvs1_0.x;});
    // verificar if is the end
    if (p2 == points.end()) p2 -= 1;
    // compute the second boundary
    Point pvs1_1 = *p2;
    
    //cout << "pvs1_0: "<< pvs1_0 << "\n";
    //cout << "pvs1_1: "<< pvs1_1 << "\n";
    
    // align local coordinate frame
    pvs1 -= pvs1_0;
    //cout << "pvs1: "<< pvs1 << "\n";
    
    // parametric equations
    int xs = pvs1.x;
    int ys = pvs1.y;
    int delta_y = pvs1_1.y-pvs1_0.y; // delta_y == 2*ys should be
    float b = delta_y/pi;
    float a = xs;
    
    waves.push_back(make_pair(a,b));

    // update the first boundarie for next wave
    last = pvs1_1;
    
    /*
  // show result using parametric equations
  double delta_t = pi/(delta_y);
  for (int i = 0; i < delta_y; i++) {
  Point p (int(a*sin(delta_t*i)), i);
    helix_points.push_back(p+pvs1_0);
    }
  */
  }
}


void Sketch2D::processing(){
  skel_g = skeletization(image_g);
  extractPoints(skel_g, points_g);
  findMaxCurvature(points_g, max_points_g);
  generateParametricEqu(points_g, max_points_g, waves_g);
  for (auto p: waves_g) {
    cout << p.first<<"      " <<p.second << "\n";
  }
  
}












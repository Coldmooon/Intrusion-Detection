#include <iostream>
#include <vector>
#include <string>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/video/video.hpp"
//#include <sstream>
#include <SFML/Audio.hpp>
#include <SFML/Audio/SoundSource.hpp>
using namespace cv;
using namespace std;

#define UNKNOWN_FLOW_THRESH 1e9

typedef struct AllSelections
{
    char type; 
    Rect selection;
    int id;
} PAllSelections;

// rStatus = 0: No sound is played.
// rStatus = 1: Now, play the sound.
// rStatus = 2: The sound is being played.
// The same to lStatus
int rStatus = 0, lStatus = 0;

int ihash[10] = {0};
int ifUsed[10] = {0};

char c = '0';
bool bSelectObject = false;
bool bTracking = false;
Point origin;
Rect selection;
vector<AllSelections> allSelection;
Mat img;

vector<Mat> allPrevgray;
vector<Point> pointSets;
vector< vector<Point> > allPointSets;
vector<Point> start;
vector<Point> endP;
vector<int> timeCount(100,0);
vector<int> timeCountForRegion(100,0);
int nArea = 0;

int getID(int array[] )
{
    for(int i = 1; i <= 9; ++i)
    {
        bool r = 1 & array[i];
        if( r == 0 )
            return i;
    }
    return 0;
}

void getPointOnTheLine(Point _start, Point _end, vector<Point> & _set)
{
    Point start = _start;
    Point endP = _end;
    Point temp;
    int aPointY;
    vector<Point> & set =  _set;
    
    float diffence = (float)(endP.y - start.y)/(float)((endP.x - start.x) + 0.0001);
    for( int i = selection.x; i <= (selection.x + selection.width - 1); ++i )
    {
        aPointY = (int)(diffence*(i - start.x )) + start.y;
        temp.x = i;
        temp.y = aPointY;
        set.push_back(temp);
    }
}

void onMouse(int event, int x, int y, int, void*)
{
    Point tmpStart, tmpEnd;
    if( bSelectObject )
    {
        
        allPrevgray.resize(nArea+1);
        start.resize(nArea+1);
        endP.resize(nArea+1);
        allSelection.resize(nArea+1);
        allPointSets.resize(nArea+1);
        
        selection.x = MIN(x, origin.x); 
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x) + 1;
        selection.height = std::abs(y - origin.y) + 1;
        selection &= Rect(0, 0, img.cols, img.rows);

        tmpStart = origin;
        tmpEnd = Point(x,y);
    }
    
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
            origin = Point(x,y); 
            selection = Rect(x,y,0,0);
            bSelectObject = true;
            break;
        case CV_EVENT_LBUTTONUP:
            bSelectObject = false;
            bTracking = true;
            start[nArea] = tmpStart;
            endP[nArea] = tmpEnd;
            allSelection[nArea].selection = selection;
            allSelection[nArea].type = c;
            pointSets.clear();
            getPointOnTheLine(start[nArea], endP[nArea], pointSets);
            allPointSets[nArea] = pointSets;
            if( (nArea+1) != allSelection.size()) { cout << "nArea != allSelection"; exit(0);}
            if(allSelection[nArea].selection.height == 1 && allSelection[nArea].selection.width == 1)
            {
                cout << "Click!" << endl;
                allSelection[nArea].id = 0;
            }
            else
            {
                int id = getID(ifUsed); 
                if (id == 0)
                {
                    allSelection[ihash[1]].type = '0';
                    allSelection[nArea].id = 1;
                    for(int j = 2; j < 10; ++j)
                    {
                        ifUsed[j] = 0;
                    }
                    ihash[1] = nArea;
                }
                else
                {
                    if(ihash[id] != 0) allSelection[ihash[id]].type = '0';
                    allSelection[nArea].id = id;
                    ihash[id] = nArea;
                    ifUsed[id] = 1;
                }
            }
            nArea++;
            break;
    }
}

#ifdef DEBUG
void makecolorwheel(vector<Scalar> &colorwheel)
{
    int RY = 15;
    int YG = 6;
    int GC = 4;
    int CB = 11;
    int BM = 13;
    int MR = 6;
    
    int i;
    
    for (i = 0; i < RY; i++) colorwheel.push_back(Scalar(255,	   255*i/RY,	 0));
    for (i = 0; i < YG; i++) colorwheel.push_back(Scalar(255-255*i/YG, 255,		 0));
    for (i = 0; i < GC; i++) colorwheel.push_back(Scalar(0,		   255,		 255*i/GC));
    for (i = 0; i < CB; i++) colorwheel.push_back(Scalar(0,		   255-255*i/CB, 255));
    for (i = 0; i < BM; i++) colorwheel.push_back(Scalar(255*i/BM,	   0,		 255));
    for (i = 0; i < MR; i++) colorwheel.push_back(Scalar(255,	   0,		 255-255*i/MR));
}
#endif

void DEBUG_motionToColor(const Mat & flow, Mat &color)
{
#ifdef DEBUG
    if (color.empty())
        color.create(flow.rows, flow.cols, CV_8UC3);
    
    static vector<Scalar> colorwheel;
    if (colorwheel.empty())
        makecolorwheel(colorwheel);
    
    float maxrad = -1;
    for (int i= 0; i < flow.rows; ++i)
    {
        for (int j = 0; j < flow.cols; ++j)
        {
            Vec2f flow_at_point = flow.at<Vec2f>(i, j);
            float fx = flow_at_point[0];
            float fy = flow_at_point[1];
            if ((fabs(fx) >  UNKNOWN_FLOW_THRESH) || (fabs(fy) >  UNKNOWN_FLOW_THRESH))
                continue;
            float rad = sqrt(fx * fx + fy * fy);
            maxrad = maxrad > rad ? maxrad : rad;
        }
    }
    
    for (int i= 0; i < flow.rows; ++i)
    {
        for (int j = 0; j < flow.cols; ++j)
        {
            uchar *data = color.data + color.step[0] * i + color.step[1] * j;
            Vec2f flow_at_point = flow.at<Vec2f>(i, j);
            
            float fx = flow_at_point[0] / maxrad;
            float fy = flow_at_point[1] / maxrad;
            if ((fabs(fx) >  UNKNOWN_FLOW_THRESH) || (fabs(fy) >  UNKNOWN_FLOW_THRESH))
            {
                data[0] = data[1] = data[2] = 0;
                continue;
            }
            float rad = sqrt(fx * fx + fy * fy);
            
            float angle = atan2(-fy, -fx) / CV_PI;
            float fk = (angle + 1.0) / 2.0 * (colorwheel.size()-1);
            int k0 = (int)fk;
            int k1 = (k0 + 1) % colorwheel.size();
            float f = fk - k0;
            
            for (int b = 0; b < 3; b++)
            {
                float col0 = colorwheel[k0][b] / 255.0;
                float col1 = colorwheel[k1][b] / 255.0;
                float col = (1 - f) * col0 + f * col1;
                if (rad <= 1)
                    col = 1 - rad * (1 - col);
                else
                    col *= .75;
                data[2 - b] = (int)(255.0 * col);
            }
        }
    }
#endif
}


void TWO_LEVEL_DEBUG_imshowPoint(const Point currentPoint, const Mat frame, const string windowName)
{
#ifdef DEBUG_LEVEL_TWO
    Mat tempFrame;
    frame.copyTo(tempFrame);
    circle(tempFrame, currentPoint, 2, Scalar(0,255,0));
    imshow(windowName, tempFrame);
    waitKey(0);
#endif
}


static void show_usage( string name )
{
    cout << "*************************************************************************" << std::endl
    << "Usage: " << name << " <option(s)> SOURCES" << std::endl
    << "Options: " << std::endl
    << "\t run without parameters to load camera or + file path to load a video." << std::endl
    << "*************************************************************************" << std::endl;
}

int main(int argc, char** argv)
{
    if( argc > 2 )
    {
        show_usage(argv[0]);
        return -1;
    }
    
    VideoCapture cap;
    
    if( argc == 2 )
    {
        cap.open(argv[1]);
        if( !cap.isOpened() )
        {
            show_usage(argv[0]);
            return -1;
        }
    }
    else if(argc == 1)
    {
        cap.open(0);
        if( !cap.isOpened() )
        {
            show_usage(argv[0]);
            return -1;
        }
    }
    else
    {
        show_usage(argv[0]);
        return -1;
    }
    
    
    Mat gray, cflow, frame;
    
    int delay = 10;
    string WIN_RESULT = "Result";
    namedWindow(WIN_RESULT, WINDOW_AUTOSIZE);
    setMouseCallback(WIN_RESULT, onMouse, 0);
    bool paused = false;
    int energyOnLine = 0;
    int energyInRegion = 0;
    string name = "./dog.ogg";
    sf::Music music;
    if (!music.openFromFile(name))
    {
        cout << "Can not load the sound file." << endl;
        return -1;
    }
    for(;;)
    {
        if(!paused)
        {
            cap >> frame;
        }
        frame.copyTo(img);
        
        if(bTracking == true)
        {
            if(!paused)
            {
                cvtColor(frame, gray, CV_BGR2GRAY);
                for(int k = 0; k < allSelection.size(); ++k)
                {
                    Mat prevgray, roi, flow, motion2color;
                    if(allSelection[k].type == '0' || (allSelection[k].selection.height == 1 && allSelection[k].selection.width == 1))
                    {
                        energyInRegion = 0;
                        energyOnLine = 0;
                        prevgray.release();
                        roi.release();
                        flow.release();
                        continue;
                    }
                    else if(allSelection[k].type == 'R' || allSelection[k].type == 'r')
                    {
                        std::ostringstream number; number << allSelection[k].id;
                        rectangle(frame, allSelection[k].selection, cv::Scalar(255,255,0), 2);
                        putText(frame, number.str(), Point(allSelection[k].selection.x, allSelection[k].selection.y-5),
                                    FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255,255,0), 1, CV_AA);
                    }
                    else if(allSelection[k].type == 'L' || allSelection[k].type == 'l')
                    {
                        std::ostringstream number; number << allSelection[k].id;
                            line(frame, start[k], endP[k], Scalar(0,255,0));
                        putText(frame, number.str(), Point(allSelection[k].selection.x, allSelection[k].selection.y-5),
                                    FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255,255,0), 1, CV_AA);
                    }
                    else
                        continue;
                    double t = (double)cvGetTickCount();
                    roi = gray(allSelection[k].selection);
                    prevgray = allPrevgray[k];
                    if( prevgray.data )
                    {
                        calcOpticalFlowFarneback(prevgray, roi, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
                        DEBUG_motionToColor(flow, motion2color);
                        if(allSelection[k].type == 'R' || allSelection[k].type == 'r')
                        {
                            for( int i = 0; i < flow.rows; ++i)
                            {
                                Point2f* data = flow.ptr<Point2f>(i);
                                for( int j = 0; j < flow.cols; ++j)
                                {
                                    energyInRegion += data[j].x*data[j].x + data[j].y*data[j].y;
                                }
                            }
                            if (energyInRegion > 5000) timeCountForRegion[k] = 15;
                            if ( timeCountForRegion[k] > 0)
                            {
                                if (rStatus == 0)
                                    rStatus = 1;
                                
                                // sound the alarm if recieve the signal 'rStatus = 1'.
                                if (rStatus == 1)
                                    music.play();
                                // stop playing when the alarm terminates.
                                if (timeCountForRegion[k] == 1 ) {
                                    rStatus = 0;
                                    music.stop();
                                }
                                putText(frame, ":Region Alarm!", Point(allSelection[k].selection.x+10, allSelection[k].selection.y-5),
                                        FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255,255,0), 1, CV_AA);
                            }
                            if (rStatus == 1)
                                rStatus = 2;
                            rectangle(frame, allSelection[k].selection, cv::Scalar(255,255,0), 2);
                            if (timeCountForRegion[k] != 0) timeCountForRegion[k]--;
                        }
                        else if(allSelection[k].type == 'L' || allSelection[k].type == 'l')
                        {
                            for( int i = 0; i < allPointSets[k].size()-1; ++i)
                            {
                                TWO_LEVEL_DEBUG_imshowPoint(allPointSets[k][i], frame, WIN_RESULT);
                                int row = allPointSets[k][i].y-allSelection[k].selection.y;
                                Point2f* fxy = flow.ptr<Point2f>(row);
                                int col = allPointSets[k][i].x-allSelection[k].selection.x;
                                energyOnLine += fxy[col].x * fxy[col].x + fxy[col].y * fxy[col].y;
                            }
                            if (energyOnLine > 100 ) timeCount[k] = 15;
                            if ( timeCount[k] > 0)
                            {
                                if (lStatus == 0)
                                    lStatus = 1;
                                
                                if (lStatus == 1)
                                    music.play();
                                if (timeCount[k] == 1 ) {
                                    lStatus = 0;
                                    music.stop();
                                }
                                putText(frame, ":Line Alarm!", Point(allSelection[k].selection.x+10, allSelection[k].selection.y-5),
                                        FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(0,255,0), 1, CV_AA);
                            }
                            if (lStatus == 1)
                                lStatus = 2;
                            line(frame, start[k], endP[k], Scalar(0,255,0));
                            if (timeCount[k] != 0) timeCount[k]--;
                        }
                        else
                        {
                            cout << "Type Error!" << endl;
                            exit(0);
                        }
                    } 
                    roi.copyTo(allPrevgray[k]); 
                    t = (double)cvGetTickCount() - t;
                    energyInRegion = 0;
                    energyOnLine = 0;
                }
            }
        }
        imshow(WIN_RESULT, frame);
        char key = waitKey(delay); 
        if (key == 'r' || key == 'R' || key == 'l' || key == 'L' )
        {
            c = key;
        }
        else if (key >= 49 && key <= 57)
        {
            int p = key - '0';
            allSelection[ihash[p]].type = 0;
            ifUsed[p] = 0;
        }
        else if( key == 27 )
            break;
        switch(key)
        {
            case 'p':
                paused = !paused;
                break;
            default:
                ;
        }
    } 
    return 0;
}
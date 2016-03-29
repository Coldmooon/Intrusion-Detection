#include <iostream>
#include <vector>
#include <string>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/video/video.hpp"

using namespace cv;
using namespace std;

#define UNKNOWN_FLOW_THRESH 1e9

// Color encoding of flow vectors from:
// http://members.shaw.ca/quadibloc/other/colint.htm
// This code is modified from:
// http://vision.middlebury.edu/flow/data/

typedef struct AllSelections // 一个鼠标选择区域
{
    char type; // 检测类型：警戒线 or 区域检测.
    Rect selection;// 存储鼠标画的框
    int id;
} PAllSelections;

int ihash[10] = {0}; // 存储每个检测区域的 id 号和 allselections 的对应关系。
int ifUsed[10] = {0};

char c = '0';  // 检测区域的类型，'0'表示其他类型，不进行检测。
bool bSelectObject = false;	// 区域选择标志
bool bTracking = false;		// 开始跟踪标志
Point origin;	// 鼠标按下时的点位置
Rect selection;// 鼠标选择的区域大小
vector<AllSelections> allSelection;// 鼠标选择区域的集合
Mat img;	// 输出的目标图像
//Mat roi, prevgray, flow, motion2color;

vector<Mat> allPrevgray;
vector<Point> pointSets;
vector< vector<Point> > allPointSets;
vector<Point> start;
vector<Point> endP;
vector<int> timeCount(100,0); // 每次运行程序，最多画100次检测区域
vector<int> timeCountForRegion(100,0); // 每次运行程序，最多画100次检测区域
int nArea = 0;

/* 判断来自键盘的输入是否为一个数字
int validInput()
{
    int x;
    std::cin >> x;
    while(std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
        std::cout << "输入错误，请输入数字: " << std::endl;
        std::cin >> x;
    }
    return x;
} */

// 得到当前监测区域的 1 - 9 编号
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

// 得到线段上的点的坐标
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
        // 注意，在使用强制类型转换的时候，必须对转换对象的整体加括号。否则，只对最近的变量进行强制类型转换。
        // 例如：(int)(diffence*(i - start.x )) 是对 diffence*(i - start.x ) 变成整数。
        // 而 (int)diffence*(i - start.x ) 是对 diffence 变成整数。
        aPointY = (int)(diffence*(i - start.x )) + start.y;
        temp.x = i;
        temp.y = aPointY;
        set.push_back(temp);
    }
}

// 鼠标按下时触发该函数一次，鼠标松开时再次触发该函数一次。
void onMouse(int event, int x, int y, int, void*)
{
    // bSelectObject 初始值是false，当鼠标按下时不触发该if条件句。当鼠标松开，再次触发该函数时，该if条件句被执行。
    // 注意，这里的“松开鼠标”是广义的“松开”。该 onMouse 函数会扑捉鼠标的移动轨迹，每扑捉一次轨迹，都算作一次“松开”
    // 并非最终抬起鼠标按键的那一刻才算“松开”。
    Point tmpStart, tmpEnd;
    if( bSelectObject )
    {
        
        allPrevgray.resize(nArea+1);
        start.resize(nArea+1);
        endP.resize(nArea+1);
        allSelection.resize(nArea+1);
        allPointSets.resize(nArea+1);
        
        selection.x = MIN(x, origin.x); // origin.x和origin.y是鼠标按下时保存的坐标，现在鼠标松开始又有了新的x,y坐标。
        selection.y = MIN(y, origin.y);
        //注意，不 +1 会报错。因为x - origin.x 只是计算的起点和落点之间的距离。没有考虑起点也占一个宽度。所以，这里必须将 +1，来补充上起点的宽度。
        selection.width = std::abs(x - origin.x) + 1;
        selection.height = std::abs(y - origin.y) + 1;
        selection &= Rect(0, 0, img.cols, img.rows);
        //        start.x = selection.x;
        //        start.y = selection.y;
        //        endP.x = selection.x + selection.width;
        //        endP.y = selection.y + selection.height;
        tmpStart = origin;
        tmpEnd = Point(x,y);
        //allSelection[nArea].selection = selection;
        //allSelection[nArea].type = '0';
        // 因为getPointOnTheLine这个函数是push_back 的形式，所以为了防止每次选择区域时，后选择的区域会被累计到pointSets中。所以，每次鼠标松开始，都要清空pointSets。
        //pointSets.clear();
        //getPointOnTheLine(start[nArea], endP[nArea], pointSets);
        //allPointSets[nArea] = pointSets;
        //if( (nArea+1) != allSelection.size()) { cout << "nArea != allSelection"; exit(0);}
    }
    
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN: // 鼠标按下
            origin = Point(x,y); // 鼠标按下时，将触发点坐标传给origin保存下来，以便用于上面的if条件句。
            selection = Rect(x,y,0,0);
            bSelectObject = true;
            //bTracking = false;
            break;
        case CV_EVENT_LBUTTONUP: // 鼠标松开
            bSelectObject = false;
            bTracking = true;
            start[nArea] = tmpStart;
            endP[nArea] = tmpEnd;
            allSelection[nArea].selection = selection;
            allSelection[nArea].type = c;
            // 因为getPointOnTheLine这个函数是push_back 的形式，所以为了防止每次选择区域时，后选择的区域会被累计到pointSets中。所以，每次鼠标松开始，都要清空pointSets。
            pointSets.clear();
            getPointOnTheLine(start[nArea], endP[nArea], pointSets);
            allPointSets[nArea] = pointSets;
            if( (nArea+1) != allSelection.size()) { cout << "nArea != allSelection"; exit(0);}
            //            if(allSelection[nArea].selection.height != 1 || allSelection[nArea].selection.width != 1)
            //            {
            //
            //                while(allSelection[nArea].type != 'L' && allSelection[nArea].type != 'l' &&
            //                      allSelection[nArea].type != 'R' && allSelection[nArea].type != 'r')
            //                {
            //                    cout << "\n*\n请给第" << nArea+1 << "个检测区选择检测类型" << "\n"
            //                         << "L: 警戒线检测；" << "R: 区域检测; 可以一次输入多个。\n*; " << endl;
            //                    allSelection[nArea].type = cin.get();
            //                }
            //            }
            if(allSelection[nArea].selection.height == 1 && allSelection[nArea].selection.width == 1)
            {
                cout << "你点了下鼠标。" << endl;
                allSelection[nArea].id = 0;
            }
            else
            {
                int id = getID(ifUsed); // 监测区域的编号, 从 1 开始
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
            nArea++; // nArea 必须放在这个位置上。如果放在了上面 if( bSelectObject ) 的语句里。则 nArea 会被错误的累计多次。
            //cout << "完成。当前有" << nArea << "个检测区\n" << endl;
            //nFrameNum = 0;
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
    
    static vector<Scalar> colorwheel; //Scalar r,g,b
    if (colorwheel.empty())
        makecolorwheel(colorwheel);
    
    // determine motion range:
    float maxrad = -1;
    //int k = flow.rows;
    //int j = flow.cols;
    // Find max flow to normalize fx and fy
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
            //f = 0; // uncomment to see original color wheel
            
            for (int b = 0; b < 3; b++)
            {
                float col0 = colorwheel[k0][b] / 255.0;
                float col1 = colorwheel[k1][b] / 255.0;
                float col = (1 - f) * col0 + f * col1;
                if (rad <= 1)
                    col = 1 - rad * (1 - col); // increase saturation with radius
                else
                    col *= .75; // out of range
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
    //namedWindow("flow", 1);
    
    int delay = 10;	// 控制播放速度
//    const char* WIN_RESULT = "Result";
    string WIN_RESULT = "Result";
//    namedWindow(WIN_RESULT, CV_WINDOW_NORMAL);
    namedWindow(WIN_RESULT, WINDOW_AUTOSIZE);
    // 鼠标响应函数
    setMouseCallback(WIN_RESULT, onMouse, 0);
    bool paused = false;
    int energyOnLine = 0;
    int energyInRegion = 0;
    for(;;)
    {
        if(!paused)
        {
            // 有时，在此处出现 incorrect checksum for freed object - object was probably modified after being freed.
            // 这种错误。这说明，该错误是在给 Mat 赋值的时候出现的。那么同时也说明，如果是光流计算那出现这种错误的话，一定是给 flow 这个
            // Mat 赋值的时候出错的。
            cap >> frame;
        }
        frame.copyTo(img);
        
        if(bTracking == true) // 如果捕获鼠标了，就开始干活
        {
            if(!paused) // 是否按下了暂停键
            {
                cvtColor(frame, gray, CV_BGR2GRAY);
                for(int k = 0; k < allSelection.size(); ++k)
                {
                    Mat prevgray, roi, flow, motion2color;
                    if(allSelection[k].type == '0' || (allSelection[k].selection.height == 1 && allSelection[k].selection.width == 1))
                    {
                        //cout << "1*1 matric!" << endl;
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
                    //Mat roi(gray, selection); // 在这声明的话，会没有实际效果。
                    //gray(allSelection[k]).copyTo(roi);
                    //allPrevgray[k].copyTo(prevgray);
                    roi = gray(allSelection[k].selection);
                    prevgray = allPrevgray[k];
                    if( prevgray.data )
                    {
                        // 如果出现 incorrect checksum for freed object - object was probably modified after being freed.
                        // 错误，则可能是多释放了一次Mat，看 http://bbs.csdn.net/topics/380093493?page=1#post-394849205
                        calcOpticalFlowFarneback(prevgray, roi, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
                        DEBUG_motionToColor(flow, motion2color);
                        //方法一：
                        //Point2f fxy;
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
                            if (energyInRegion > 5000) timeCountForRegion[k] = 50;
                            if ( timeCountForRegion[k] > 0)
                            {
                                putText(frame, ":Region Alarm!", Point(allSelection[k].selection.x+10, allSelection[k].selection.y-5),
                                        FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(255,255,0), 1, CV_AA);
                            }
                            rectangle(frame, allSelection[k].selection, cv::Scalar(255,255,0), 2);
                            if (timeCountForRegion[k] != 0) timeCountForRegion[k]--;
                        }
                        else if(allSelection[k].type == 'L' || allSelection[k].type == 'l')
                        {
                            for( int i = 0; i < allPointSets[k].size()-1; ++i)
                            {
                                TWO_LEVEL_DEBUG_imshowPoint(allPointSets[k][i], frame, WIN_RESULT);
                                
                                //方法一：方法一还需要额外打开 268 行对 fxy 的声明
                                //fxy = flow.at<Point2f>((pointSets[i].y-start.y), (pointSets[i].x-start.x));
                                //energyOnLine += fxy.x*fxy.x + fxy.y*fxy.y;
                                
                                //方法二：
                                //const Point2f& fxy = flow.at<Point2f>((pointSets[i].y-start.y), (pointSets[i].x-start.x));
                                //energyOnLine += fxy.x*fxy.x + fxy.y*fxy.y;
                                
                                //方法三
                                //int row = pointSets[i].y-start.y;
                                int row = allPointSets[k][i].y-allSelection[k].selection.y;
                                Point2f* fxy = flow.ptr<Point2f>(row); // 获取当前像素点的在flow中的行指针
                                int col = allPointSets[k][i].x-allSelection[k].selection.x;// 找到当前像素点在flow中对应的列数
                                
                                // 从左下到右上划线时，会提前报警，因为物体的影子会先于物体接触警戒线。
                                energyOnLine += fxy[col].x * fxy[col].x + fxy[col].y * fxy[col].y;
                            }
                            if (energyOnLine > 100 ) timeCount[k] = 50;
                            if ( timeCount[k] > 0)
                            {
                                putText(frame, ":Line Alarm!", Point(allSelection[k].selection.x+10, allSelection[k].selection.y-5),
                                        FONT_HERSHEY_COMPLEX_SMALL, 0.8, Scalar(0,255,0), 1, CV_AA);
                            }
                            line(frame, start[k], endP[k], Scalar(0,255,0));
                            if (timeCount[k] != 0) timeCount[k]--;
                        }
                        else
                        {
                            cout << "Type Error!" << endl;
                            exit(0);
                        }
                        // flow的大小是107*150，而pointSets的大小是 151。所以这里要对pointSets.size()-1
                        
                        //  char fileName[256];
                        //  sprintf(fileName, "overenergy%06d.jpg",numFrame);
                        //                    if(energyInRegion > 1000 || energyOnLine > 30) imwrite(fileName, motion2color);
                        //cout << "energyInRegionSum: " << energyInRegion << endl;
                        //cout << "energyOnLineSum: " << energyOnLine << endl;
                        //imshow("flow", motion2color); // 如果在 waitkey 之前， flow 被release 掉，那么就不会显示 flow 窗口
                    } // if( prevgray.data )
                    
                    //  if(waitKey(10)>=0)
                    //      break;
                    //std::swap(allPrevgray[k], roi);
                    //imshow("roi", roi );
                    roi.copyTo(allPrevgray[k]); // copyTo 这种方法也可以。
                    t = (double)cvGetTickCount() - t;
                    //cout << "cost time: " << t / ((double)cvGetTickFrequency()*1000.) << endl;
                    energyInRegion = 0;
                    energyOnLine = 0;
                    //prevgray.release();
                    //roi.release();
                    //flow.release();
                } // for(;roi;)
            } // if paused
        } // if(bTracking == true)
        
        //setWindowProperty(WIN_RESULT,CV_WND_PROP_FULLSCREEN,CV_WINDOW_FULLSCREEN);
        imshow(WIN_RESULT, frame);
        
        // 当按下一个按键后，waitKey() 是先对窗口进行操作，然后再返回按键值。
        // 所以，按键后的效果总会慢于窗口的反应。如果在同一帧内，按下了按键，又画了个框。那么按键值是不会输入给当前帧的当前框的。
        // 只能输入给后续帧的框。
        char key = waitKey(delay); // 对图像的所有操作，都是在 waitkey 处被触发。例如，画框，点击鼠标等。
        // 注意：c一旦被赋值，会在接下来的每一帧里，都 keep 这个值。为了程序不出错，最好每帧结束后，清空c。
        if (key == 'r' || key == 'R' || key == 'l' || key == 'L' )
        {
            c = key;
            
            /* 删除模式代码（未启用）
            if( key != 'd') {
                while (1) {
                    int delWin;
                    cout << "输入你想删除的检测区域编号（0 退出）: " << endl;
                    delWin = validInput();
                    if ( delWin >= allSelection.size() ) delWin = allSelection.size() - 1;
                    else if (delWin == 0) break;
                    //std::vector<AllSelections>::iterator it = allSelection.begin() + delWin;
                    //allSelection.erase(it);
                    allSelection[delWin-1].type = 0;
                }
            } */
        }
        else if (key >= 49 && key <= 57)
        {
            int p = key - '0';
            allSelection[ihash[p]].type = 0;
            ifUsed[p] = 0;
        }
        else if( key == 27 ) // 按 ESC 键退出。
            break;
        switch(key)
        {
            case 'p'://暂停键
                paused = !paused;
                break;
            default:
                ;
        }
    } // for(;;) loop the video
    return 0;
}
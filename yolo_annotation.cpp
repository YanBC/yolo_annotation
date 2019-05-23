////////////////////////////////////////////////////////////////
// Original: <opencv-master>/apps/annotation/opencv_annotation.cpp
// Modified: yanbc <imyanbc@gmail.com>
////////////////////////////////////////////////////////////////

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include <fstream>
#include <iostream>
#include <map>

using namespace std;
using namespace cv;


// Class: holds the cv::Rect and class infomations
class Info
{
public:
    Rect coordinates;
    int cls;

    Info()
    {
        coordinates = Rect(0, 0, 0, 0);
        cls = -1;        
    }

    Info(const Rect r, int c)
    {
        coordinates = r;
        cls = c;
    }
};



// Function prototypes
void on_mouse(int, int, int, int, void*);
vector<Info> get_annotations(Mat);
vector<string> split(const string&, const string&);

// Public parameters
Mat image;
int roi_x0 = 0, roi_y0 = 0, roi_x1 = 0, roi_y1 = 0, num_of_rec = 0;
bool start_draw = false, stop = false;

// Window name for visualisation purposes
string window_name = "OpenCV Based Annotation Tool";







int main( int argc, const char** argv )
{
    // Use the cmdlineparser to process input arguments
    CommandLineParser parser(argc, argv,
        "{ help h usage ?   |      | show this message }"
        "{ images i         |      | (required) path to image folder [example - /data/testimages/] }"
        "{ maxWindowHeight m|  -1  | (optional) images larger in height than this value will be scaled down }"
        "{ resizeFactor r   |  2   | (optional) factor for scaling down [default = half the size] }"
        "{ yolo             |      | (optional) specified if using yolo type of annotations}"
    );
    // Read in the input arguments
    if (parser.has("help")){
        parser.printMessage();
        cerr << "TIP: Use absolute paths to avoid any problems with the software!" << endl;
        return 0;
    }
    string image_folder(parser.get<string>("images"));
    if (image_folder.empty()){
        parser.printMessage();
        cerr << "TIP: Use absolute paths to avoid any problems with the software!" << endl;
        return -1;
    }

    int resizeFactor = parser.get<int>("resizeFactor");
    int const maxWindowHeight = parser.get<int>("maxWindowHeight") > 0 ? parser.get<int>("maxWindowHeight") : -1;

    // Start by processing the data
    // Return the image filenames inside the image folder
    map< String, vector<Info> > annotations;
    vector<String> filenames;
    String folder(image_folder);
    glob(folder, filenames);

    // Add key tips on how to use the software when running it
    cout << "* mark rectangles with the left mouse button," << endl;
    cout << "* press {'0','1','2','3','4','5','6','7','8','9'} to accept a selection," << endl;
    cout << "* press 'd' to delete the latest selection," << endl;
    cout << "* press 'n' to proceed with next image," << endl;
    cout << "* press 'esc' to stop." << endl;

    // Loop through each image stored in the images folder
    // Create and temporarily store the annotations
    // At the end write everything to the annotations file
    for (size_t i = 0; i < filenames.size(); i++){
        // Show file name in image windows
        window_name = filenames[i];

        // Read in an image
        Mat current_image = imread(filenames[i]);
        bool const resize_bool = (maxWindowHeight > 0) && (current_image.rows > maxWindowHeight);

        // Check if the image is actually read - avoid other files in the folder, because glob() takes them all
        // If not then simply skip this iteration
        if(current_image.empty()){
            continue;
        }

        if(resize_bool){
            resize(current_image, current_image, Size(current_image.cols/resizeFactor, current_image.rows/resizeFactor), 0, 0, INTER_LINEAR_EXACT);
        }

        // Perform annotations & store the result inside the vectorized structure
        // If the image was resized before, then resize the found annotations back to original dimensions
        vector<Info> current_annotations = get_annotations(current_image);
        if(resize_bool){
            for(int j =0; j < (int)current_annotations.size(); j++){
                current_annotations[j].coordinates.x = current_annotations[j].coordinates.x * resizeFactor;
                current_annotations[j].coordinates.y = current_annotations[j].coordinates.y * resizeFactor;
                current_annotations[j].coordinates.width = current_annotations[j].coordinates.width * resizeFactor;
                current_annotations[j].coordinates.height = current_annotations[j].coordinates.height * resizeFactor;
            }
        }

        // Check if the ESC key was hit, then exit earlier then expected
        if(stop){
            break;
        }

        annotations[filenames[i]] = current_annotations;
    }


    // Write annotations
    for(map<String, vector<Info> >::iterator it = annotations.begin(); it != annotations.end(); it++){
    
        // Create annotation files
        string annoName = split(it->first, ".")[0] + ".txt";
        ofstream output(annoName.c_str());
        if ( !output.is_open() ){
            cerr << "The path for the output file contains an error and could not be opened. Please check again!\nTIP: Use absolute paths to avoid any problems with the software!" << endl;
            return 0;
        }

        // Convert to yolo annotations
        // And write to file
        Mat current_image = imread(it->first);
        float imageHeight = float(current_image.rows);
        float imageWidth = float(current_image.cols);

        vector<Info> &anno = it -> second;

        for(size_t j=0; j < anno.size(); j++){
            Rect coor = anno[j].coordinates;
            int cls = anno[j].cls;

            if (parser.has("yolo"))
            {
                float x, y, w, h;
                x = (float(coor.x) + float(coor.width) /2) / imageWidth;
                y = (float(coor.y) + float(coor.height) /2) / imageHeight;
                w = float(coor.width) / imageWidth;
                h = float(coor.height) / imageHeight;
                output << char(cls) << " " << x << " " << y << " " << w << " " << h << endl;
            }
            else
            {
                output << char(cls) << " " << coor.x << " " << coor.y << " " << coor.width << " " << coor.height << endl;
            }


            
        }
    }
    return 0;
}









// FUNCTION : Mouse response for selecting objects in images
// If left button is clicked, start drawing a rectangle as long as mouse moves
// Stop drawing once a new left click is detected by the on_mouse function
void on_mouse(int event, int x, int y, int , void * )
{
    // Action when left button is clicked
    if(event == EVENT_LBUTTONDOWN)
    {
        if(!start_draw)
        {
            roi_x0 = x;
            roi_y0 = y;
            start_draw = true;
        } else {
            roi_x1 = x;
            roi_y1 = y;
            start_draw = false;
        }
    }

    // Action when mouse is moving and drawing is enabled
    if((event == EVENT_MOUSEMOVE) && start_draw)
    {
        // Redraw bounding box for annotation
        Mat current_view;
        image.copyTo(current_view);
        rectangle(current_view, Point(roi_x0,roi_y0), Point(x,y), Scalar(0,0,255));
        imshow(window_name, current_view);
    }
}

// FUNCTION : returns a vector of Info objects given an image containing positive object instances
vector<Info> get_annotations(Mat input_image)
{
    vector<Info> current_annotations;

    // Make it possible to exit the annotation process
    stop = false;

    // Init window interface and couple mouse actions
    namedWindow(window_name, WINDOW_AUTOSIZE);
    setMouseCallback(window_name, on_mouse);

    image = input_image;
    imshow(window_name, image);
    int key_pressed = 0;

    do
    {
        // Get a temporary image clone
        Mat temp_image = input_image.clone();
        Rect currentRect(0, 0, 0, 0);
        int cls = -1;

        // Keys for processing
        // You need to select one for confirming a selection and one to continue to the next image
        // Based on the universal ASCII code of the keystroke: http://www.asciitable.com/
        //      c = 99          add rectangle to current image
        //      n = 110         save added rectangles and show next image
        //      d = 100         delete the last annotation made
        //      <ESC> = 27      exit program
        key_pressed = 0xFF & waitKey(0);
        switch( key_pressed )
        {
        case 27:
                stop = true;
                break;
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
                // Draw initiated from top left corner
                if(roi_x0<roi_x1 && roi_y0<roi_y1)
                {
                    currentRect.x = roi_x0;
                    currentRect.y = roi_y0;
                    currentRect.width = roi_x1-roi_x0;
                    currentRect.height = roi_y1-roi_y0;
                }
                // Draw initiated from bottom right corner
                if(roi_x0>roi_x1 && roi_y0>roi_y1)
                {
                    currentRect.x = roi_x1;
                    currentRect.y = roi_y1;
                    currentRect.width = roi_x0-roi_x1;
                    currentRect.height = roi_y0-roi_y1;
                }
                // Draw initiated from top right corner
                if(roi_x0>roi_x1 && roi_y0<roi_y1)
                {
                    currentRect.x = roi_x1;
                    currentRect.y = roi_y0;
                    currentRect.width = roi_x0-roi_x1;
                    currentRect.height = roi_y1-roi_y0;
                }
                // Draw initiated from bottom left corner
                if(roi_x0<roi_x1 && roi_y0>roi_y1)
                {
                    currentRect.x = roi_x0;
                    currentRect.y = roi_y1;
                    currentRect.width = roi_x1-roi_x0;
                    currentRect.height = roi_y0-roi_y1;
                }
                cls = key_pressed;

                current_annotations.push_back(Info(currentRect, cls));
                break;
        case 100:
                // Remove the last annotation
                if(current_annotations.size() > 0){
                    current_annotations.pop_back();
                }
                break;
        default:
                // Default case --> do nothing at all
                // Other keystrokes can simply be ignored
                break;
        }

        // Check if escape has been pressed
        if(stop)
        {
            break;
        }

        // Draw all the current rectangles onto the top image and make sure that the global image is linked
        for(int i=0; i < (int)current_annotations.size(); i++){
            // Draw bounding box
            rectangle(temp_image, current_annotations[i].coordinates, Scalar(0,255,0), 1);

            // Draw label
            // char labelClass = char(current_annotations[i].cls);
            // string label = "Class";
            char labelClass[] = "Class \0\0";
            labelClass[6] = char(current_annotations[i].cls);
            string label = string(labelClass);
            int baseline;
            Rect labelRect;

            Size labelSize = getTextSize(label, FONT_HERSHEY_PLAIN, 1.5, 2, &baseline);
            labelRect.x = current_annotations[i].coordinates.x;
            labelRect.y = current_annotations[i].coordinates.y - 2 * baseline;
            labelRect.width = int(labelSize.width);
            labelRect.height = int(labelSize.height);

            rectangle(temp_image, labelRect, Scalar(0,0,255), FILLED);
            putText(temp_image, label.c_str(), Point(current_annotations[i].coordinates.x, current_annotations[i].coordinates.y), FONT_HERSHEY_PLAIN, 1.5, Scalar(0,0,0), 2);
        }
        image = temp_image;

        // Force an explicit redraw of the canvas --> necessary to visualize delete correctly
        imshow(window_name, image);
    }
    // Continue as long as the next image key has not been pressed
    while(key_pressed != 110);

    // Close down the window
    destroyWindow(window_name);

    // Return the data
    return current_annotations;
}


// FUNCTION: splits a string with the given seperator
vector<string> split(const string &s, const string &seperator)
{
    vector<string> result;
    typedef string::size_type string_size;
    string_size i = 0;
    
    while(i != s.size()){
        int flag = 0;
        while(i != s.size() && flag == 0){
            flag = 1;
            for(string_size x = 0; x < seperator.size(); ++x)
            if(s[i] == seperator[x]){
                ++i;
                flag = 0;
                break;
            }
        }
        
        flag = 0;
        string_size j = i;
        while(j != s.size() && flag == 0){
            for(string_size x = 0; x < seperator.size(); ++x)
            if(s[j] == seperator[x]){
                flag = 1;
                break;
            }
            if(flag == 0) 
            ++j;
        }
        if(i != j){
            result.push_back(s.substr(i, j-i));
            i = j;
        }
    }
    return result;
}
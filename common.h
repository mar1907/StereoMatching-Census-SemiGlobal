#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#ifdef __WIN32__
#include <windows.h>
#include <direct.h> /* _mkdir */
#elif __linux__
#include <sys/stat.h> /* mkdir */
#include <sys/types.h> /* mkdir */
#else
// other platform
#endif

using namespace cv;

#define PI 3.14159265
#define POS_INFINITY 1e30
#define NEG_INFINITY -1e30
#define max_(x,y) ((x) > (y) ? (x) : (y))
#define min_(x,y) ((x) < (y) ? (x) : (y))
#define isNan(x) ((x) != (x) ? 1 : 0)

#ifdef __WIN32__
class FileGetter{
	WIN32_FIND_DATAA found;	
	HANDLE hfind;
	char folder[MAX_PATH];			
	int chk;
	bool first;
	bool hasFiles;
public:
	FileGetter(char* folderin,char* ext);
	int getNextFile(char* fname);
	int getNextAbsFile(char* fname);
	char* getFoundFileName();
};


int openFileDlg(char* fname);

int openFolderDlg(char* folderName);
#elif __linux__
// linux code
#else
// other platform
#endif

void resizeImg(Mat src, Mat &dst, int maxSize, bool interpolate);

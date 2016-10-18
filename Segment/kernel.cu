#include "Stdafx.h"
#include "HFSSegment.h"

int main()
{
	// run on BSDS dataset
	DataSet bsds("C:/WkDir/BSR/BSDS500/");
	HFSSegment engine(bsds);
    engine.runDataSet(0.28f, 220);

	// segment a single image
	/*HFSSegment engine("C:/WkDir/BSR/BSDS500/Results/Segment.WS");
	Mat img3u = imread("C:/WkDir/BSR/BSDS500/JPEGImages/103078.jpg");
	Mat seg, show;
	int num_css = engine.processImage(seg, img3u, 0.28f, 220);
	engine.drawSegmentationRes(show, seg, img3u, num_css);
	imshow("Segmentation", show);
	waitKey(0);*/

	return 0;
}


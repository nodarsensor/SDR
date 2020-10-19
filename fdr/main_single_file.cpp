#include <sys/stat.h>
#include <chrono>
#include "ArgsParser.h"
#include "Evaluator.h"
#include "FastDR.h"
#include "Utilities.hpp"

#ifdef _WIN32
#include <direct.h>
#endif

/**
 * Winner-take-all operation to compute disparity
 */
cv::Mat WTAOfVolume(const cv::Mat& volume)
{
	int D = volume.size.p[0];
	int H = volume.size.p[1];
	int W = volume.size.p[2];
	cv::Mat disp(H, W, CV_32F), mask;
	cv::Mat cost(H, W, CV_32F);
	cv::Mat cost_minimum(H, W, CV_32F, cv::Scalar::all(FLT_MAX));
	cv::Mat disp_WTA(H, W, CV_32F, cv::Scalar::all(0));
	for (int d = 0; d < D; d++) {
		disp = d;
		for (int y = 0; y < H; y++) {
			auto p = volume.ptr<float>(d, y);
			for (int x = 0; x < W; x++) {
				cost.at<float>(y, x) = *(p + x);
			}
		}
		mask = cost < cost_minimum;
		cost.copyTo(cost_minimum, mask);
		disp.copyTo(disp_WTA, mask);
	}
	return disp_WTA;
}

/**
 * Filling the left (right) part of the cost volume
 */
void fillOutOfView(cv::Mat& volume)
{
	int D = volume.size.p[0];
	int H = volume.size.p[1];
	int W = volume.size.p[2];

	for (int d = 0; d < D; d++)
		for (int y = 0; y < H; y++)
		{
			auto p = volume.ptr<float>(d, y);
			auto q = p + d;
			float v = *q;
			while (p != q) {
				*p = v;
				p++;
			}
		}
}

/**
 * Loading the data of Middlebury dataset
 */
bool loadData(const std::string inputDir, cv::Mat& im0, cv::Mat& disp_WTA, cv::Mat& dispGT, cv::Mat& nonocc, Calib&calib)
{
	if (calib.ndisp <= 0)
		printf("Try to retrieve ndisp from file [calib.txt].\n");
	calib = Calib(inputDir + "calib.txt");
	if (calib.ndisp <= 0) {
		printf("ndisp is not speficied.\n");
		return false;
	}

	im0 = cv::imread(inputDir + "im0.png");
	if (im0.empty()) {
		printf("Image im0.png not found in\n");
		printf("%s\n", inputDir.c_str());
		return false;
	}

	disp_WTA = cvutils::io::read_pfm_file(inputDir + "disp_WTA.pfm");
	if (disp_WTA.empty()) {
		int sizes[] = { calib.ndisp, im0.rows, im0.cols };
		cv::Mat vol = cv::Mat_<float>(3, sizes);
		if (cvutils::io::loadMatBinary(inputDir + "im0.acrt", vol, false) == false) {
			printf("Cost volume file im0.acrt not found\n");
			return false;
		}
		fillOutOfView(vol);
		disp_WTA = WTAOfVolume(vol);
		cvutils::io::save_pfm_file(inputDir + "disp_WTA.pfm", disp_WTA);
	}

	dispGT = cvutils::io::read_pfm_file(inputDir + "disp0GT.pfm");
	if (dispGT.empty())
		dispGT = cv::Mat_<float>::zeros(im0.size());

	nonocc = cv::imread(inputDir + "mask0nocc.png", cv::IMREAD_GRAYSCALE);
	if (!nonocc.empty())
		nonocc = nonocc == 255;
	else
		nonocc = cv::Mat_<uchar>(im0.size(), 255);

	return true;
}

/**
 * Update the invalid disparity (0 value) to a random disparity from
 * (1,max_disp)
 */
void preprocess_disp(cv::Mat& disp_WTA, const int max_disp) {
	cv::RNG rng;
	for (int y = 0; y < disp_WTA.rows; y++)
		for (int x = 0; x < disp_WTA.cols; x++) {
			if (disp_WTA.at<float>(y, x) < 1)
				disp_WTA.at<float>(y, x) = rng.operator() (max_disp - 1) + 1;
		}
}

/**
 * Processing NODAR
 */
void NODAR(const std::string imagePath, const std::string disparityPath, const std::string outputPath,
    const Options& options) {
    Parameters params = options.params;

    int max_disp = options.ndisp;

    cv::Mat im0, disp_WTA_u16, disp_WTA;

    auto c0 = std::chrono::steady_clock::now();

    auto filename = cv::format("nodar_%04d.png", i);
    
    std::cout << "--------------------------------------------"
              << std::endl;
    std::cout << "processing: " + imagePath.c_str() << std::endl;

    im0 = cv::imread(imagePath);
    if (im0.empty()) {
        printf("Color reference image not found in\n");
        printf("%s\n", imagePath.c_str());
        return;
    };
    disp_WTA_u16 =
        cv::imread(disparityPath,
                   CV_LOAD_IMAGE_UNCHANGED);
    if (disp_WTA_u16.empty()) {
        printf("WTA disparity map not found in\n");
        printf("%s\n", disparityPath.c_str());
        return;
    };

    // disp_WTA_u16.convertTo(disp_WTA, CV_32FC1, 1/256.0);
    disp_WTA_u16.convertTo(disp_WTA, CV_32FC1);
    preprocess_disp(disp_WTA, max_disp);

    FastDR* fdr = new FastDR(im0, disp_WTA, params, max_disp, 0);

    cv::Mat labeling, refined_disp;
    fdr->run(labeling, refined_disp);

    refined_disp.convertTo(disp_WTA_u16, CV_16UC1, 256);

    cv::imwrite(outputPath, disp_WTA_u16);

    delete fdr;

    auto c1 = std::chrono::steady_clock::now();
    std::cout << "Total time consumed: "
              << std::chrono::duration_cast<std::chrono::microseconds>(c1 - c0) .count()
              << std::endl;
}


int main(int argc, const char** args) {
    std::cout << "----------- parameter settings -----------" << std::endl;
    ArgsParser parser(argc, args);
    Options options;

    options.loadOptionValues(parser);

    NODAR(options.targetDir + "/", options.outputDir + "/", options);

    return 0;
}

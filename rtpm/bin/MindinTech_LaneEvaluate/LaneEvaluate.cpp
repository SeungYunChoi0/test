// LaneEvaluate.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "include/counter.hpp"
#include "include/spline.hpp"
//#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>

#include <opencv2/opencv.hpp>            // C++
#include <opencv2/highgui/highgui_c.h>   // C
#include <opencv2/imgproc/imgproc_c.h>   // C
using namespace std;
using namespace cv;

void read_lane_file(const string& file_name, vector<vector<Point2f> >& lanes);
void visualize(string& full_im_name, vector<vector<Point2f> >& anno_lanes, vector<vector<Point2f> >& detect_lanes, vector<int> anno_match, int width_lane);

std::string ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = 0; //string처음부터 검사
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)  //from을 찾을 수 없을 때까지
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // 중복검사를 피하고 from.length() > to.length()인 경우를 위해서
	}
	return str;
}

int main(int argc, char* argv[])
{
	// process params
	//string anno_dir = "I:\\LaneAF\\LaneAF-main\\datasets\\custom\\test";// "/data/driving/eval_data/anno_label/";
	//string detect_dir = "I:\\LaneAF\\LaneAF-main\\datasets\\custom\\test";//"/data/driving/eval_data/predict_label/";
	//string im_dir = "I:\\LaneAF\\LaneAF-main\\experiments\\custom\\2022_04_11_19_21_infer\\outputs";
	//string list_im_file = "I:\\LaneAF\\LaneAF-main\\datasets\\custom\\test\\all.txt";

	string anno_dir = "D:\\01_Enlight\\ENLIGHT_NPU_V20_SW_Toolkit_v0.9.2_telechips_0711\\downloads\\custom_lane\\CULane";// "/data/driving/eval_data/anno_label/";
	string detect_dir = "D:\\01_Enlight\\ENLIGHT_NPU_V20_SW_Toolkit_v0.9.2_telechips_0711\\working\\lane_result_640x384_small\\txt";//"/data/driving/eval_data/predict_label/";
	string im_dir = "D:\\01_Enlight\\ENLIGHT_NPU_V20_SW_Toolkit_v0.9.2_telechips_0711\\downloads\\custom_lane\\CULane";
	string list_im_file = "D:\\01_Enlight\\ENLIGHT_NPU_V20_SW_Toolkit_v0.9.2_telechips_0711\\downloads\\custom_lane\\CULane\\all.txt";

	string output_file = "./output.txt";
	int width_lane = 30;
	double iou_threshold = 0.5;
	//int im_width = 1920;
	//int im_height = 1280;
	int im_width = 1640;
	int im_height = 590;
	int oc;
	bool show = false;
	int frame = 1;

	if (argc == 8)
	{
		anno_dir = argv[1];
		detect_dir = argv[2];
		im_dir = argv[3];
		list_im_file = argv[4];
		im_width = atoi(argv[5]);
		im_height = atoi(argv[6]);
		output_file = argv[7];

	}
	else
	{
		cout << "Invalid Arguments" << endl;
		return 0;
	}


	//while ((oc = getopt(argc, argv, "ha:d:i:l:w:t:c:r:sf:o:")) != -1)
	//{
	//	switch (oc)
	//	{
	//	case 'h':
	//		//help()；
	//		return 0;
	//	case 'a':
	//		anno_dir = optarg;
	//		break;
	//	case 'd':
	//		detect_dir = optarg;
	//		break;
	//	case 'i':
	//		im_dir = optarg;
	//		break;
	//	case 'l':
	//		list_im_file = optarg;
	//		break;
	//	case 'w':
	//		width_lane = atoi(optarg);
	//		break;
	//	case 't':
	//		iou_threshold = atof(optarg);
	//		break;
	//	case 'c':
	//		im_width = atoi(optarg);
	//		break;
	//	case 'r':
	//		im_height = atoi(optarg);
	//		break;
	//	case 's':
	//		show = true;
	//		break;
	//	case 'f':
	//		frame = atoi(optarg);
	//		break;
	//	case 'o':
	//		output_file = optarg;
	//		break;
	//	}
	//}


	cout << "------------Configuration---------" << endl;
	cout << "anno_dir: " << anno_dir << endl;
	cout << "detect_dir: " << detect_dir << endl;
	cout << "im_dir: " << im_dir << endl;
	cout << "list_im_file: " << list_im_file << endl;
	cout << "width_lane: " << width_lane << endl;
	cout << "iou_threshold: " << iou_threshold << endl;
	cout << "im_width: " << im_width << endl;
	cout << "im_height: " << im_height << endl;
	cout << "output_file: " << output_file << endl;
	cout << "-----------------------------------" << endl;
	cout << "Evaluating the results..." << endl;
	// this is the max_width and max_height

	if (width_lane < 1)
	{
		cerr << "width_lane must be positive" << endl;
		//help();
		return 1;
	}


	ifstream ifs_im_list(list_im_file, ios::in);
	if (ifs_im_list.fail())
	{
		cerr << "Error: file " << list_im_file << " not exist!" << endl;
		return 1;
	}


	Counter counter(im_width, im_height, iou_threshold, width_lane);

	vector<int> anno_match;
	string sub_im_name;
	// pre-load filelist
	vector<string> filelists;
	while (getline(ifs_im_list, sub_im_name)) {
		filelists.push_back(sub_im_name);
	}
	ifs_im_list.close();

	vector<tuple<vector<int>, long, long, long, long>> tuple_lists;
	tuple_lists.resize(filelists.size());

#pragma omp parallel for
	for (size_t i = 0; i < filelists.size(); i++)
	{
		auto sub_im_name = filelists[i];
		string full_im_name = im_dir + sub_im_name;
		full_im_name = ReplaceAll(full_im_name, string("/"), string("\\"));

		string sub_txt_name = sub_im_name.substr(0, sub_im_name.find_last_of(".")) + ".lines.txt";
		string sub_detect_txt_name = sub_im_name.substr(0, sub_im_name.find_last_of(".")) + ".txt";
		string anno_file_name = anno_dir + sub_txt_name;
		anno_file_name = ReplaceAll(anno_file_name, string("/"), string("\\"));
		string detect_file_name = detect_dir + sub_detect_txt_name;
		detect_file_name = ReplaceAll(detect_file_name, string("/"), string("\\"));
		vector<vector<Point2f> > anno_lanes;
		vector<vector<Point2f> > detect_lanes;
		//read_lane_file("D:\\62da2315ff696b0031f1e53a-MITC_TEST_CAM0001-0000000100.txt", anno_lanes);
		read_lane_file(anno_file_name, anno_lanes);
		read_lane_file(detect_file_name, detect_lanes);
		//cerr<<count<<": "<<full_im_name<<endl;
		tuple_lists[i] = counter.count_im_pair(anno_lanes, detect_lanes);
		if (show)
		{
			auto anno_match = get<0>(tuple_lists[i]);
			visualize(full_im_name, anno_lanes, detect_lanes, anno_match, width_lane);
			waitKey(0);
		}
	}

	long tp = 0, fp = 0, tn = 0, fn = 0;
	for (auto result : tuple_lists) {
		tp += get<1>(result);
		fp += get<2>(result);
		// tn = get<3>(result);
		fn += get<4>(result);
	}
	counter.setTP(tp);
	counter.setFP(fp);
	counter.setFN(fn);

	double precision = counter.get_precision();
	double recall = counter.get_recall();
	double F = 2 * precision * recall / (precision + recall);
	cerr << "finished process file" << endl;
	cout << "precision: " << precision << endl;
	cout << "recall: " << recall << endl;
	cout << "Fmeasure: " << F << endl;
	cout << "----------------------------------" << endl;

	ofstream ofs_out_file;
	ofs_out_file.open(output_file, ios::out);
	ofs_out_file << "tp,fp,fn,precision,recall,f1-score" << endl;
	ofs_out_file << counter.getTP() << "," << counter.getFP() << "," << counter.getFN() << "," << precision << "," << recall << "," << F << endl;
	//ofs_out_file << "file: " << output_file << endl;
	//ofs_out_file << "tp: " << counter.getTP() << " fp: " << counter.getFP() << " fn: " << counter.getFN() << endl;
	//ofs_out_file << "precision: " << precision << endl;
	//ofs_out_file << "recall: " << recall << endl;
	//ofs_out_file << "Fmeasure: " << F << endl << endl;
	ofs_out_file.close();
	return 0;
}

void read_lane_file(const string& file_name, vector<vector<Point2f> >& lanes)
{
	lanes.clear();
	//ifstream ifs_lane(file_name, ios::in);
	std::ifstream ifs_lane(file_name);
	if (ifs_lane.fail())
	{
		return;
	}

	string str_line;
	while (getline(ifs_lane, str_line))
	{
		vector<Point2f> curr_lane;
		stringstream ss;
		ss << str_line;
		double x, y;
		while (ss >> x >> y)
		{
			curr_lane.push_back(Point2f(x, y));
		}
		lanes.push_back(curr_lane);
	}

	ifs_lane.close();
}

void visualize(string& full_im_name, vector<vector<Point2f> >& anno_lanes, vector<vector<Point2f> >& detect_lanes, vector<int> anno_match, int width_lane)
{
	Mat img = imread(full_im_name, 1);
	Mat img2 = imread(full_im_name, 1);
	vector<Point2f> curr_lane;
	vector<Point2f> p_interp;
	Spline splineSolver;
	Scalar color_B = Scalar(255, 0, 0);
	Scalar color_G = Scalar(0, 255, 0);
	Scalar color_R = Scalar(0, 0, 255);
	Scalar color_P = Scalar(255, 0, 255);
	Scalar color;
	for (int i = 0; i < anno_lanes.size(); i++)
	{
		curr_lane = anno_lanes[i];
		if (curr_lane.size() == 2)
		{
			p_interp = curr_lane;
		}
		else
		{
			p_interp = splineSolver.splineInterpTimes(curr_lane, 50);
		}
		if (anno_match[i] >= 0)
		{
			color = color_G;
		}
		else
		{
			color = color_G;
		}
		for (int n = 0; n < p_interp.size() - 1; n++)
		{
			cv::Point2i i1 = p_interp[n];
			cv::Point2i i2 = p_interp[n + 1];
			cv::line(img, i1, i2, color, width_lane);
			cv::line(img2, i1, i2, color, 2);
		}
	}
	bool detected;
	for (int i = 0; i < detect_lanes.size(); i++)
	{
		detected = false;
		curr_lane = detect_lanes[i];
		if (curr_lane.size() == 2)
		{
			p_interp = curr_lane;
		}
		else
		{
			p_interp = splineSolver.splineInterpTimes(curr_lane, 50);
		}
		for (int n = 0; n < anno_lanes.size(); n++)
		{
			if (anno_match[n] == i)
			{
				detected = true;
				break;
			}
		}
		if (detected == true)
		{
			color = color_B;
		}
		else
		{
			color = color_R;
		}
		for (int n = 0; n < p_interp.size() - 1; n++)
		{
			cv::Point2i i1 = p_interp[n];
			cv::Point2i i2 = p_interp[n + 1];
			cv::line(img, i1, i2, color, width_lane);
			cv::line(img2, i1, i2, color, 2);
		}
	}
	namedWindow("visualize", 1);
	imshow("visualize", img);
	namedWindow("visualize2", 1);
	imshow("visualize2", img2);
}
// 프로그램 실행: <Ctrl+F5> 또는 [디버그] > [디버깅하지 않고 시작] 메뉴
// 프로그램 디버그: <F5> 키 또는 [디버그] > [디버깅 시작] 메뉴

// 시작을 위한 팁: 
//   1. [솔루션 탐색기] 창을 사용하여 파일을 추가/관리합니다.
//   2. [팀 탐색기] 창을 사용하여 소스 제어에 연결합니다.
//   3. [출력] 창을 사용하여 빌드 출력 및 기타 메시지를 확인합니다.
//   4. [오류 목록] 창을 사용하여 오류를 봅니다.
//   5. [프로젝트] > [새 항목 추가]로 이동하여 새 코드 파일을 만들거나, [프로젝트] > [기존 항목 추가]로 이동하여 기존 코드 파일을 프로젝트에 추가합니다.
//   6. 나중에 이 프로젝트를 다시 열려면 [파일] > [열기] > [프로젝트]로 이동하고 .sln 파일을 선택합니다.

#include<iostream>
#include<cmath>
#include<opencv2/opencv.hpp>


//带灯条匹配的识别函数并在视频流函数中调用
static void building(cv::Mat image) {
	using namespace std;
	using namespace cv;
	Mat image_ = image.clone();//视频帧副本
	Mat drawcontours = image_.clone();//展示帧
	Mat contours_show = image.clone();//展示轮廓

	//形态学闭运算
	Mat kernel_close = getStructuringElement(MORPH_RECT, Size(7, 7));
	morphologyEx(image_, image_, MORPH_CLOSE, kernel_close, Point(-1, -1), 1);
	GaussianBlur(image_, image_, Size(5, 5), 0, 0);

	//颜色空间转换
	Mat image_hsv = image_.clone();
	cvtColor(image_, image_hsv, COLOR_BGR2HSV);

	//对HSV的亮蓝色阈值化
	Mat mask_H = Mat::zeros(image.size(), image.type());//颜色掩码
	Mat mask_V = Mat::zeros(image.size(), image.type());//亮度掩码
	inRange(image_hsv, Scalar(100, 43, 130), Scalar(124, 255, 255), mask_H);
	//inRange(image_hsv, Scalar(0, 0, 235), Scalar(180, 255, 255), mask_V);
	Mat mask_hsv = mask_H /*| mask_V*/;

	//根据目标颜色的掩码提取轮廓
	std::vector<std::vector<Point>> contours_all_0;
	std::vector<Vec4i> contours_all_1;
	std::vector<std::vector<Point>> contours;
	findContours(mask_hsv, contours_all_0, contours_all_1, RETR_CCOMP, CHAIN_APPROX_NONE);//所有层全部提取，不作优化

	//提取内轮廓
	if (contours_all_1.size() != 0) {
		for (int i = 0; i < contours_all_1.size(); i++) {
			double area = contourArea(contours_all_0[i], true);//若面积为非零正数则轮廓内像素值为0
			if ((contours_all_1[i][3] != -1) && (area > 0)) {
				contours.push_back(contours_all_0[i]);
			}
		}
	}
	//drawContours(drawcontours, contours, -1, Scalar(0, 255, 0), 1, LINE_4);

	//轮廓大小和长宽比筛选
	if (contours.size() != 0) {
		auto contour = contours.begin();
		std::vector<std::vector<Point>> contours_ustd;//不合格轮廓，展示用
		while (contour != contours.end()) {
			float ratio = 0;
			float ratio_w = 0;
			Rect rect_bounding_0 = boundingRect(*contour);
			RotatedRect rect_bounding = minAreaRect(*contour);
			float contours_area = contourArea(*contour);//轮廓面积

			if ((rect_bounding.size.height >= rect_bounding.size.width) && (rect_bounding.size.width != 0)) {
				ratio = rect_bounding.size.height / rect_bounding.size.width;//使用外接轴对称矩形检查长宽比
			}
			if ((rect_bounding.size.height < rect_bounding.size.width) && (rect_bounding.size.height != 0)) {
				ratio = rect_bounding.size.width / rect_bounding.size.height;
			}
			ratio_w = rect_bounding_0.height / rect_bounding_0.width;//使用旋转矩形的长宽比检查是否竖直

			//合格则忽略
			if ((ratio > 3) && (ratio < 40.0) && (rect_bounding_0.area() > 90) && (rect_bounding_0.area() < 5000)
				&& (ratio_w > 1) && (ratio_w < 25)) {
				contour++;
			}
			else {//不合格则展示差异
				/*printf("ratio = %lf\n", ratio);
				printf("rect_bounding_0.area() = %d\n", rect_bounding_0.area());
				printf("ratio_w = %lf\n", ratio_w);
				printf("-----------\n");*/
				if ((ratio_w > 1) && (ratio_w < 25)) {
					contours_ustd.push_back(*contour);
				}
				contour = contours.erase(contour);
			}
		}
		drawContours(drawcontours, contours/*_ustd*/, -1, Scalar(0, 100, 255), 4, LINE_4);
	}

	//遍历轮廓集并配对灯条
	std::vector<std::vector<Point>> contours_paired;//成对灯条集
	if (contours.size() >= 2) {
		auto contour_ele = contours.begin();//迭代器起点

		while (contour_ele != contours.end()) {

			RotatedRect rollrect_0 = minAreaRect(*contour_ele);//给迭代器对应的轮廓创建旋转矩阵
			Rect rect_ele_bounding = boundingRect(*contour_ele);
			Point rollrect_0_center = rollrect_0.center;//转化成整形点
			auto contour_ele_inside = contour_ele + 1;//剩下轮廓的begin

			//创建一个待配对灯条的差异度
			double rating = 1000000;
			std::vector<Point>contour_std;//暂存符合条件的轮廓
			auto contour_std_0 = contour_ele_inside;//暂存符合指向条件轮廓的迭代器
			while (contour_ele_inside != contours.end()) {
				RotatedRect rollrect_1 = minAreaRect(*contour_ele_inside);
				Rect rect_ele_inside_bounding = boundingRect(*contour_ele_inside);

				if ((*contour_ele_inside).empty() && (rect_ele_bounding.width == 0) && (rect_ele_inside_bounding.width == 0)) {
					contour_std = *contour_ele_inside;//初始化暂存器
					contour_ele_inside++;
					continue;
				}

				//长宽比差异
				float rect_ele_bounding_ratio = rect_ele_bounding.height / rect_ele_bounding.width;
				float rect_ele_inside_bounding_ratio = rect_ele_inside_bounding.height / rect_ele_inside_bounding.width;
				float dratio = fabs(rect_ele_bounding_ratio - rect_ele_inside_bounding_ratio);

				//距离
				Point rect_ele_inside_point(-1, -1);
				rect_ele_inside_point.x = (rect_ele_inside_bounding.x + (rect_ele_inside_bounding.width * 0.5));
				rect_ele_inside_point.y = (rect_ele_inside_bounding.y + (rect_ele_inside_bounding.height * 0.5));
				double dis = norm(rect_ele_inside_point - rollrect_0_center);

				//面积差异
				double area_0 = contourArea(*contour_ele);
				double area_1 = contourArea(*contour_ele_inside);
				double darea = fabs(area_0 - area_1);

				//长度差异
				double dlength = fabs(rollrect_0.size.height - rollrect_1.size.height);

				//高度差异
				double dheight = fabs(rollrect_0.center.y - rollrect_1.center.y);

				//差异度
				double rating_0 = dis + darea * 10 + dratio * 1000 + dlength * 1000 + dheight * 1000;
				if (rating_0 < rating) {//遍历各个轮廓的与外层循环轮廓的差异值
					rating = rating_0;
					contour_std = *contour_ele_inside;
					contour_std_0 = contour_ele_inside;
				}
				contour_ele_inside++;
			}

			//如果有合格轮廓
			if (!contour_std.empty()) {
				contours_paired.push_back(contour_std);
				contours_paired.push_back(*contour_ele);
				contours.erase(contour_std_0);
				contour_ele = contours.erase(contour_ele);//迭代器更新！
			}
			else contour_ele++;//迭代器更新！
		}
	}


	//如果有成对灯条
	if ((contours_paired.size() != 0) && ((contours_paired.size() % 2) == 0)) {

		//对每一对灯条进行识别
		for (int i = 0; i < contours_paired.size(); i += 2) {
			float ratio = 0;
			//把成对的灯条塞进待处理集
			std::vector<std::vector<Point>>to_be_processed;
			to_be_processed.push_back(contours_paired[i]);
			to_be_processed.push_back(contours_paired[i + 1]);
			Rect rect_0 = boundingRect(to_be_processed[0]);  //生成第一个轮廓的外接矩形用于判断
			ratio = rect_0.height / rect_0.width;//计算长宽比

			//判断灯条是否为空
			if ((!to_be_processed[i].empty()) && (!to_be_processed[i + 1].empty())) {

				std::vector<Point> boundingrect_points;//外接矩形坐标集
				std::vector<Point> contour_high_points;//上顶点集
				std::vector<Point> contour_low_points;//下顶点集

				//找出两个灯条的四个顶点
				for (const auto& contour : to_be_processed) {
					//遍历轮廓上的点找出顶点
					Point y_high(0, 480);
					Point y_low(0, 0);
					//对单个轮廓上的点的遍历
					for (const auto& point : contour) {
						if (point.y < y_high.y) {
							y_high.x = point.x;
							y_high.y = point.y;
						}
						if (point.y > y_low.y) {
							y_low.x = point.x;
							y_low.y = point.y;
						}
					}
					contour_high_points.push_back(y_high);
					contour_low_points.push_back(y_low);
				}

				//连接四个顶点
				if ((contour_high_points.size() == 2) && (contour_low_points.size() == 2)) {
					line(drawcontours, contour_high_points[0], contour_high_points[1], Scalar(0, 255, 255), 1, LINE_AA);
					line(drawcontours, contour_high_points[1], contour_low_points[1], Scalar(0, 255, 255), 1, LINE_AA);
					line(drawcontours, contour_low_points[1], contour_low_points[0], Scalar(0, 255, 255), 1, LINE_AA);
					line(drawcontours, contour_low_points[0], contour_high_points[0], Scalar(0, 255, 255), 1, LINE_AA);
				}
			}
			else continue;
		}
	}

	//框选未成对轮廓
	if (contours.size() != 0) {
		for (const auto& contour : contours) {
			Rect rect = boundingRect(contour);
			rectangle(drawcontours, rect, Scalar(0, 255, 0), 2, LINE_4);
		}
	}


	namedWindow("show", WINDOW_AUTOSIZE);
	namedWindow("binary", WINDOW_NORMAL);
	imshow("show", drawcontours);
	imshow("binary_hsv", mask_hsv);
}


//读帧函数
static bool frameRead(cv::VideoCapture& cap, cv::Mat& frame) { //输入一个cap视频和一个frame图像
	using namespace std;
	using namespace cv;
	return cap.read(frame);//将从cap读取的一帧储存进frame
}


//打开视频
static void video_demo(std::string str) {
	using namespace std;
	using namespace cv;
	std::string video_path = str;//以字符串类型储存视频路径
	cv::VideoCapture cap(video_path);//将路径的视频文件储存进cpa

	if (!cap.isOpened()) {//条件无法读取文件
		std::cerr << "cannot load";
	}

	cv::Mat frame;

	while (frameRead(cap, frame)) {//如果有帧被读取
		building(frame);//使用函数处理识别当前帧

		//视频流操作
		char key = cv::waitKey(33);//等待键盘输入
		if (key == 's' || key == 'S') {
			waitKey(0);
		}
		if (key == 'c' || key == 'C') {
			continue;
		}
		if (key == 's' || key == 'S') {
			waitKey(5000);
		}
	}

	cap.release();//释放内存
	frame.release();
	cv::destroyAllWindows();
}


int main() {
	video_demo("D:/rec/b.mp4");
}
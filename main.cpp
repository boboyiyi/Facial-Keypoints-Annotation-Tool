#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "./src/common/cvui.h"

static int
read_pts_2d(const char *pts_path, std::vector<cv::Point> &pts_2d) {
    std::ifstream fi(pts_path);
    std::string line;
    int nb_pts = -1;
    std::getline(fi, line);
    fi >> line >> nb_pts;
    fi >> line;
    pts_2d.resize(nb_pts);
    float x, y;
    int cnt = 0;
    for (int i = 0; i < nb_pts; ++i) {
        cv::Point tmp;
        fi >> pts_2d[i].x >> pts_2d[i].y;
    }
    fi.close(); fi.clear();
    return nb_pts;
}

static void
draw (cv::Mat &img, std::vector<cv::Point> &pts_2d, int radius) {
    for (int i = 0; i < pts_2d.size(); ++i) {
        cv::circle(img, pts_2d[i], radius, cv::Scalar(0, 0, 255), -1);
    }
}

static void
reset() {
}

static void
fill_pts(const char *pts_file_name, std::vector<std::vector<cv::Point> > &pts, int &nb_frames, int &nb_pts) {
    std::ifstream fi(pts_file_name);
    fi >> nb_frames >> nb_pts;
    pts.resize(nb_frames);
    std::string tmp;
    float x, y;
    for (int i = 0; i < nb_frames; ++i) {
        pts[i].resize(nb_pts);
        fi >> tmp;
        for (int j = 0; j < nb_pts; ++j) {
            fi >> x >> y;
            pts[i][j].x = x; pts[i][j].y = y;
        }
    }
    fi.close(); fi.clear();
}

static void
save_pts(const char *out_file_name, std::vector<std::vector<cv::Point> > &out_pts) {
    std::ofstream fo(out_file_name);
    fo << out_pts.size() << " " << out_pts[0].size() << std::endl;
    for (int i = 0; i < out_pts.size(); ++i) {
        fo << i << std::endl;
        for (int j = 0; j < out_pts[i].size(); ++j) {
            fo << out_pts[i][j].x << " " << out_pts[i][j].y << std::endl;
        }
    }
    fo.close(); fo.clear();
}

int main(int argc, char **argv) {
    cv::VideoCapture cap("xpg.mp4");
    if(!cap.isOpened()){
        std::cout << "Error opening video stream or file" << std::endl;
    }

    std::vector<cv::Point> pts_2d;
    int nb_pts = read_pts_2d("test.pts", pts_2d);

    std::vector<cv::Mat> frames;
    while (1) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            break;
        }
        frames.push_back(frame);
    }
    std::vector<std::vector<cv::Point> > pts;
    int nb_pts_frames = 0;
    fill_pts("out.txt", pts, nb_pts_frames, nb_pts);
    
    if (nb_pts_frames != frames.size()) {
        std::cout << "pts frames: " << nb_pts_frames << std::endl;
        std::cout << "video frames: " << frames.size() << std::endl;
        std::cout << "Number of pts frames is not equal to video frames, pls check!" << std::endl;
        std:exit(-1);
    }

    std::string window_name = "FKAT";

    cvui::init(window_name, 100);
    int config_region_width = 260;
    cv::Mat frame = cv::Mat(cv::Size(config_region_width + frames[0].cols, frames[0].rows), CV_8UC3);
    int cnt = 0;
    bool play = false, draw_pts = true, move_one = false, move_all = false, rotate = false, select = false;
    bool zoom_in = false, zoom_out = false, reset = false;
    bool move_one_working = false, move_all_working = false, rotate_working = false, select_working = false;
    bool zoom_in_working = false, zoom_out_working = false, reset_working = false;
    int move_one_id = 0;
    float rotate_step = 0.01;
    int steps = 1;
    float ratio = 1.0f;
    cv::Point anchor;
    cv::Rect rect;
    cv::Mat img_to_draw = cv::Mat(cv::Size(frames[0].cols, frames[0].rows), CV_8UC3);
    std::vector<cv::Point> pts_2d_for_rotate = pts[0];
    std::vector<std::vector<cv::Point> > out_pts = pts;

    
    while(true) {
        frame = cv::Scalar(100, 100, 100);
        
        cvui::trackbar(frame, 10, 10, 240, &cnt, 0, int(frames.size() - 1), 1, "%.0Lf");

        if (cvui::button(frame, 22, 70, 100, 25, "Play")) {
            move_all = false;
            rotate = false;
            move_one = false;
            select = false;
            play = true;
            zoom_in = false;
            zoom_out = false;
		}

        if (cvui::button(frame, 135, 70, 100, 25, "Stop")) {
            move_all = false;
            rotate = false;
            move_one = false;
            select = false;
            play = false;
            zoom_in = false;
            zoom_out = false;
		}

        if (cvui::button(frame, 22, 100, 45, 25, "First")) {
            cnt = 0;
        }

        if (cvui::button(frame, 77, 100, 45, 25, "&--1")) {
            if (cnt != 0) {
                --cnt;
            }
		}

        if (cvui::button(frame, 135, 100, 45, 25, "&=+1")) {
            if (cnt != frames.size() - 1) {
                ++cnt;
            }
		}

        if (cvui::button(frame, 189, 100, 45, 25, "Last")) {
            cnt = frames.size() - 1;
        }

        if (cvui::button(frame, 22, 140, 100, 25, "&1: Move One") && !play) {
            move_all = false;
            rotate = false;
            select = false;
            zoom_in = false;
            zoom_out = false;
            move_one = true;
		}

        if (cvui::button(frame, 135, 140, 100, 25, "&2: Move All") && !play) {
            move_one = false;
            rotate = false;
            select = false;
            zoom_in = false;
            zoom_out = false;
            move_all = true;
		}
        
        if (cvui::button(frame, 22, 170, 100, 25, "&3: Select") && !play) {
            move_one = false;
            move_all = false;
            rotate = false;
            zoom_in = false;
            zoom_out = false;
            select = true;
		}

        if (cvui::button(frame, 135, 170, 100, 25, "&4: Move Sel") && !play) {
            move_one = false;
            move_all = false;
            rotate = false;
            select = false;
            zoom_in = false;
            zoom_out = false;
		}

        if (cvui::button(frame, 22, 200, 100, 25, "&5: Rotate") && !play) {
            move_one = false;
            move_all = false;
            select = false;
            zoom_in = false;
            zoom_out = false;
            rotate = true;
		}

        if (cvui::button(frame, 135, 200, 100, 25, "&6: Reset") && !play) {
            move_one = false;
            move_all = false;
            select = false;
            zoom_in = false;
            zoom_out = false;
            rotate = false;
            out_pts[cnt] = pts[cnt];
		}

        if (cvui::button(frame, 22, 240, 100, 25, "&7: Zoom in")) {
            zoom_out = false;
            zoom_in = true;
        }

        if (cvui::button(frame, 135, 240, 100, 25, "&8: Zoom out")) {
            zoom_in = false;
            zoom_out = true;
        }

        if (cvui::button(frame, 22, 280, 100, 25, "&0: delete") && !play) {
            move_one = false;
            move_all = false;
            select = false;
            zoom_in = false;
            zoom_out = false;
            rotate = false;
            for (int i = 0; i < nb_pts; ++i) {
                out_pts[cnt][i].x = 0;
                out_pts[cnt][i].y = 0;
            }
        }

        if (cvui::button(frame, 135, 280, 100, 25, "&x: quit")) {
            break;
        }

        if (cvui::button(frame, 22, 330, 212, 25, "&Save")) {
            save_pts("revised.txt", out_pts);
        }

        if (move_one) {
            // if button down, select most close point to move
            if (cvui::mouse(cvui::DOWN) && cvui::mouse().x >= config_region_width) {
                anchor.x = cvui::mouse().x;
                anchor.y = cvui::mouse().y;
                std::cout << anchor.x << " " << anchor.y << std::endl;
                for (int i = 0; i < nb_pts; ++i) {
                    if (std::abs(anchor.x - config_region_width - out_pts[cnt][i].x) <= 2 && std::abs(anchor.y - out_pts[cnt][i].y) <=2) {
                        move_one_id = i;
                        move_one_working = true;
                        std::cout << "move one mode" << std::endl;
                        break;
                    }
                }
                std::cout << move_one_id << std::endl;
            }
        }

        if (move_one_working) {
            if (cvui::mouse(cvui::IS_DOWN)) {
                out_pts[cnt][move_one_id].x = cvui::mouse().x - config_region_width;
                out_pts[cnt][move_one_id].y = cvui::mouse().y;
            }
        }

        if (move_all) {
            if (cvui::mouse(cvui::DOWN) && cvui::mouse().x >= config_region_width) {
                anchor.x = cvui::mouse().x;
                anchor.y = cvui::mouse().y;
                move_all_working = true;
            }
        }

        if (move_all_working) {
            if (cvui::mouse(cvui::IS_DOWN)) {
                for (int i = 0; i < nb_pts; ++i) {
                    out_pts[cnt][i].x += (cvui::mouse().x - anchor.x);
                    out_pts[cnt][i].y += (cvui::mouse().y - anchor.y);
                }
                anchor.x = cvui::mouse().x;
                anchor.y = cvui::mouse().y;
            }
        }

        if (rotate) {
            if (cvui::mouse(cvui::DOWN) && cvui::mouse().x >= config_region_width) {
                anchor.x = cvui::mouse().x;
                anchor.y = cvui::mouse().y;
                rotate_working = true;
                // backup the pts_2d
                pts_2d_for_rotate = out_pts[cnt];
            }
        }

        if (rotate_working) {
            if (cvui::mouse(cvui::LEFT_BUTTON, cvui::IS_DOWN)) {
                for (int i = 0; i < nb_pts; ++i) {
                    out_pts[cnt][i].x = std::ceil(std::cos(-rotate_step * steps) * float(pts_2d_for_rotate[i].x - (anchor.x - config_region_width)) - std::sin(-rotate_step * steps) * float(pts_2d_for_rotate[i].y - anchor.y)) + (anchor.x - config_region_width);
                    out_pts[cnt][i].y = std::ceil(std::sin(-rotate_step * steps) * float(pts_2d_for_rotate[i].x - (anchor.x - config_region_width)) + std::cos(-rotate_step * steps) * float(pts_2d_for_rotate[i].y - anchor.y)) + anchor.y;
                }
            }
            if (cvui::mouse(cvui::RIGHT_BUTTON, cvui::IS_DOWN)) {
                for (int i = 0; i < nb_pts; ++i) {
                    out_pts[cnt][i].x = std::ceil(std::cos(rotate_step * steps) * float(pts_2d_for_rotate[i].x - (anchor.x - config_region_width)) - std::sin(rotate_step * steps) * float(pts_2d_for_rotate[i].y - anchor.y)) + (anchor.x - config_region_width);
                    out_pts[cnt][i].y = std::ceil(std::sin(rotate_step * steps) * float(pts_2d_for_rotate[i].x - (anchor.x - config_region_width)) + std::cos(rotate_step * steps) * float(pts_2d_for_rotate[i].y - anchor.y)) + anchor.y;
                }
            }
            ++steps;
        }

        if (zoom_in) {
            if (cvui::mouse(cvui::DOWN) && cvui::mouse().x >= config_region_width) {
                anchor.x = cvui::mouse().x;
                anchor.y = cvui::mouse().y;
                zoom_in_working = true;
                std::cout << "zoom_in_working" << std::endl;
                std::cout << anchor.x << " " << anchor.y << std::endl;
            }
        }

        if (zoom_in_working) {
            if (cvui::mouse(cvui::IS_DOWN)) {
                int width = cvui::mouse().x - anchor.x;
                int height = cvui::mouse().y - anchor.y;
                rect.x = width < 0 ? anchor.x + width : anchor.x;
                rect.y = height < 0 ? anchor.y + height : anchor.y;
                rect.width = std::abs(width);
                rect.height = std::abs(height);
                rect.x = rect.x - config_region_width < 0 ? config_region_width : rect.x;
                rect.y = rect.y < 0 ? 0 : rect.y;
                rect.width = rect.x + rect.width > frames[0].cols + config_region_width ? frames[0].cols + config_region_width - rect.x : rect.width;
                rect.height = rect.y + rect.height > frames[0].rows ? rect.height + frames[0].rows - (rect.y + rect.height) : rect.height;
                ratio = std::max(float(frames[0].rows) / float(rect.height), float(frames[0].cols) / float(rect.width));
            }
        }

        if (cvui::mouse(cvui::UP)) {
            rotate_working = false;
			move_one_working = false;
            move_all_working = false;
            select_working = false;
            zoom_in_working = false;
            steps = 1;
		}
        
        frames[cnt].copyTo(img_to_draw);
        if (zoom_in_working) {
            cvui::rect(img_to_draw, rect.x - config_region_width, rect.y, rect.width, rect.height, 0xff0000);
        }
        draw(img_to_draw, out_pts[cnt], 2);
        img_to_draw.copyTo(frame(cv::Rect(config_region_width, 0, frames[0].cols, frames[0].rows)));
        cvui::imshow(window_name, frame);

        if (rect.area() > 0 && !zoom_in_working) {
            cv::Rect roi = rect;
            roi.x = rect.x - config_region_width;
            cv::Mat img_roi;
            cv::resize(img_to_draw(roi), img_roi, cv::Size(roi.width * 2, roi.height * 2));
            cv::imshow("ROI", img_roi);
        }

        if (play) {
            if (cnt != frames.size() - 1) {
                ++cnt;
            }
            else {
                play = false;
            }
        }
        // if (cv::waitKey(20) == 27) {
		// 	break;
		// }
    }
    return -1;
}
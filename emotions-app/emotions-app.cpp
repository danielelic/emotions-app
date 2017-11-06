// emotions-app
//
// Copyright (C) 2017 Daniele Liciotti
//
// Authors: Daniele Liciotti <danielelic@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see: http://www.gnu.org/licenses/gpl-3.0.txt

#include <iostream>
#include <memory>
#include <chrono>
#include <fstream>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "Frame.h"
#include "Face.h"
#include "FrameDetector.h"

#include "AFaceListener.hpp"
#include "PlottingImageListener.hpp"
#include "StatusListener.hpp"

using namespace std;
using namespace affdex;

namespace configuration {
    //---------------------------------------------------------------------------
    // The configuration::data is a simple map string (key, value) pairs.
    // The file is stored as a simple listing of those pairs, one per line.
    // The key is separated from the value by an equal sign '='.
    // Commentary begins with the first non-space character on the line a hash or
    // semi-colon ('#' or ';').
    struct data : std::map<std::string, std::string> {
        // Here is a little convenience method...
        bool iskey(const std::string &s) const {
            return count(s) != 0;
        }
    };

    //---------------------------------------------------------------------------
    // The extraction operator reads configuration::data until EOF.
    // Invalid data is ignored.
    //
    std::istream &operator>>(std::istream &ins, data &d) {
        std::string s, key, value;

        // For each (key, value) pair in the file
        while (std::getline(ins, s)) {
            std::string::size_type begin = s.find_first_not_of(" \f\t\v");

            // Skip blank lines
            if (begin == std::string::npos) continue;

            // Skip commentary
            if (std::string("#;").find(s[begin]) != std::string::npos) continue;

            // Extract the key value
            std::string::size_type end = s.find('=', begin);
            key = s.substr(begin, end - begin);

            // (No leading or trailing whitespace allowed)
            key.erase(key.find_last_not_of(" \f\t\v") + 1);

            // No blank keys allowed
            if (key.empty()) continue;

            // Extract the value (no leading or trailing whitespace allowed)
            begin = s.find_first_not_of(" \f\n\r\t\v", end + 1);
            end = s.find_last_not_of(" \f\n\r\t\v") + 1;

            value = s.substr(begin, end - begin);

            // Insert the properly extracted (key, value) pair into the map
            d[key] = value;
        }

        return ins;
    }

    //---------------------------------------------------------------------------
    // The insertion operator writes all configuration::data to stream.
    //
    std::ostream &operator<<(std::ostream &outs, const data &d) {
        data::const_iterator iter;
        for (iter = d.begin(); iter != d.end(); iter++)
            outs << iter->first << " = " << iter->second << endl;
        return outs;
    }

} // namespace configuration


int main(int argsc, char **argsv) {
    namespace po = boost::program_options; // abbreviate namespace

    shared_ptr<FrameDetector> frameDetector;

    try {

        const std::vector<int> DEFAULT_RESOLUTION{1920, 1080};

        affdex::path DATA_FOLDER;

        std::vector<int> resolution;
        int process_framerate = 30;
        int camera_framerate = 15;
        int buffer_length = 2;
        int camera_id = 0;
        unsigned int nFaces = 1;
        bool draw_display = true;
        int faceDetectorMode = (int) FaceDetectorMode::LARGE_FACES;

        float last_timestamp = -1.0f;
        float capture_fps = -1.0f;

        const int precision = 2;
        std::cerr.precision(precision);
        std::cout.precision(precision);

        po::options_description description(
                "Emotion Project");
        description.add_options()
                ("help,h", po::bool_switch()->default_value(false), "Display this help message.")
#ifdef _WIN32
                ("data,d", po::wvalue< affdex::path >(&DATA_FOLDER)->default_value(affdex::path(L"data"), std::string("data")), "Path to the data folder")
#else //  _WIN32
                ("data,d",
                 po::value<affdex::path>(&DATA_FOLDER)->default_value(affdex::path("data"), std::string("data")),
                 "Path to the data folder")
#endif // _WIN32
                ("resolution,r", po::value<std::vector<int> >(&resolution)->default_value(DEFAULT_RESOLUTION,
                                                                                          "1920 1080")->multitoken(),
                 "Resolution in pixels (2-values): width height")
                ("pfps", po::value<int>(&process_framerate)->default_value(30), "Processing framerate.")
                ("cfps", po::value<int>(&camera_framerate)->default_value(30), "Camera capture framerate.")
                ("bufferLen", po::value<int>(&buffer_length)->default_value(30), "process buffer size.")
                ("cid", po::value<int>(&camera_id)->default_value(0), "Camera ID.")
                ("faceMode", po::value<int>(&faceDetectorMode)->default_value((int) FaceDetectorMode::LARGE_FACES),
                 "Face detector mode (large faces vs small faces).")
                ("numFaces", po::value<unsigned int>(&nFaces)->default_value(1), "Number of faces to be tracked.")
                ("draw", po::value<bool>(&draw_display)->default_value(false), "Draw metrics on screen.");
        po::variables_map args;
        try {
            po::store(po::command_line_parser(argsc, argsv).options(description).run(), args);
            if (args["help"].as<bool>()) {
                std::cout << description << std::endl;
                return 0;
            }
            po::notify(args);
        }
        catch (po::error &e) {
            std::cerr << "ERROR\t" << e.what() << std::endl << std::endl;
            std::cerr << "INFO\tFor help, use the -h option." << std::endl << std::endl;
            return 1;
        }

        if (!boost::filesystem::exists(DATA_FOLDER)) {
            std::cerr << "ERROR\tFolder doesn't exist: " << std::string(DATA_FOLDER.begin(), DATA_FOLDER.end())
                    << std::endl
                    << std::endl;;
            std::cerr << "ERROR\tTry specifying the folder through the command line" << std::endl;
            std::cerr << description << std::endl;
            return 1;
        }
        if (resolution.size() != 2) {
            std::cerr << "ERROR\tOnly two numbers must be specified for resolution." << std::endl;
            return 1;
        } else if (resolution[0] <= 0 || resolution[1] <= 0) {
            std::cerr << "ERROR\tResolutions must be positive number." << std::endl;
            return 1;
        }

        std::cerr << "INFO\tInitializing Affdex FrameDetector" << endl;
        shared_ptr<FaceListener> faceListenPtr(new AFaceListener());
        shared_ptr<PlottingImageListener> listenPtr(
                new PlottingImageListener(draw_display));    // Instanciate the ImageListener class
        shared_ptr<StatusListener> videoListenPtr(new StatusListener());
        frameDetector = make_shared<FrameDetector>(buffer_length, process_framerate, nFaces,
                                                   (affdex::FaceDetectorMode) faceDetectorMode);        // Init the FrameDetector Class

        //Initialize detectors
        frameDetector->setDetectAllEmotions(true);
        frameDetector->setDetectAllExpressions(true);
        frameDetector->setDetectAllEmojis(true);
        frameDetector->setDetectAllAppearances(true);
        frameDetector->setClassifierPath(DATA_FOLDER);
        frameDetector->setImageListener(listenPtr.get());
        frameDetector->setFaceListener(faceListenPtr.get());
        frameDetector->setProcessStatusListener(videoListenPtr.get());

        cv::VideoCapture webcam(camera_id);    //Connect to the first webcam
        webcam.set(CV_CAP_PROP_FPS, camera_framerate);    //Set webcam framerate.
        webcam.set(CV_CAP_PROP_FRAME_WIDTH, resolution[0]);
        webcam.set(CV_CAP_PROP_FRAME_HEIGHT, resolution[1]);
        std::cerr << "INFO\tSetting the webcam frame rate to: " << camera_framerate << std::endl;
        auto start_time = std::chrono::system_clock::now();
        if (!webcam.isOpened()) {
            std::cerr << "ERROR\tError opening webcam!" << std::endl;
            return 1;
        }

        std::cout << "INFO\tMax num of faces set to: " << frameDetector->getMaxNumberFaces() << std::endl;
        std::string mode;
        switch (frameDetector->getFaceDetectorMode()) {
            case FaceDetectorMode::LARGE_FACES:
                mode = "LARGE_FACES";
                break;
            case FaceDetectorMode::SMALL_FACES:
                mode = "SMALL_FACES";
                break;
            default:
                break;
        }
        std::cout << "INFO\tFace detector mode set to: " << mode << std::endl;

        //Start the frame detector thread.
        frameDetector->start();

        do {
            cv::Mat img;
            if (!webcam.read(img))    //Capture an image from the camera
            {
                std::cerr << "ERROR\tFailed to read frame from webcam! " << std::endl;
                break;
            }

            //Calculate the Image timestamp and the capture frame rate;
            const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - start_time);
            const double seconds = milliseconds.count() / 1000.f;

            // Create a frame
            Frame f(img.size().width, img.size().height, img.data, Frame::COLOR_FORMAT::BGR, seconds);
            capture_fps = 1.0f / (seconds - last_timestamp);
            last_timestamp = seconds;
            frameDetector->process(f);  //Pass the frame to detector

            // For each frame processed
            if (listenPtr->getDataSize() > 0) {

                std::pair<Frame, std::map<FaceId, Face> > dataPoint = listenPtr->getData();
                Frame frame = dataPoint.first;
                std::map<FaceId, Face> faces = dataPoint.second;

                // Draw metrics to the GUI
                if (draw_display) {
                    listenPtr->draw(faces, frame);
                }

                // Output metrics to the db server
                // Read data from configuration file

                std::string urlBase;
                bool debugMode;

                configuration::data myconfigdata;
                std::ifstream f(std::string(getenv("HOME")) + "/.emoj/emoj.conf");
                f >> myconfigdata;
                f.close();

                urlBase = myconfigdata["URLBASE"];

                listenPtr->outputToServer(faces, frame.getTimestamp(), urlBase);

            }
        }

#ifdef _WIN32
            while (!GetAsyncKeyState(VK_ESCAPE) && videoListenPtr->isRunning());
#else //  _WIN32
        while (videoListenPtr->isRunning());
#endif
        std::cerr << "INFO\tStopping FrameDetector Thread" << endl;
        frameDetector->stop();    //Stop frame detector thread
    }
    catch (AffdexException
           ex) {
        std::cerr << "ERROR\tEncountered an AffdexException " << ex.what();
        return 1;
    }
    catch (std::runtime_error
           err) {
        std::cerr << "ERROR\tEncountered a runtime error " << err.what();
        return 1;
    }
    catch (std::exception
           ex) {
        std::cerr << "ERROR\tEncountered an exception " << ex.what();
        return 1;
    }
    catch (...) {
        std::cerr << "ERROR\tEncountered an unhandled exception ";
        return 1;
    }
    return 0;
}

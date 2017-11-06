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


#pragma once

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <boost/filesystem.hpp>
#include <boost/timer/timer.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include "ProcessStatusListener.h"

using namespace affdex;

class StatusListener : public ProcessStatusListener
{
public:

    StatusListener(): mIsRunning(true) {};

    void onProcessingException(AffdexException ex) {
        std::cerr << "Encountered an exception while processing: " << ex.what() << std::endl;
        m.lock();
        mIsRunning = false;
        m.unlock();
    };

    void onProcessingFinished() {
        std::cerr << "Processing finished successfully" << std::endl;
        m.lock();
        mIsRunning = false;
        m.unlock();

    };

    bool isRunning() {
        bool ret = true;
        m.lock();
        ret = mIsRunning;
        m.unlock();
        return ret;
    };

private:
    std::mutex m;
    bool mIsRunning;

};

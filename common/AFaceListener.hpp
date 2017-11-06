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

#include "FaceListener.h"

using namespace affdex;

class AFaceListener : public FaceListener {
    void onFaceFound(float timestamp, FaceId faceId) {
        std::cout << "INFO\tFace ID:\t" << faceId << "\tfound at timestamp:\t" << timestamp << std::endl;
    }

    void onFaceLost(float timestamp, FaceId faceId) {
        std::cout << "INFO\tFace ID:\t" << faceId << "\tlost at timestamp:\t" << timestamp << std::endl;
    }
};

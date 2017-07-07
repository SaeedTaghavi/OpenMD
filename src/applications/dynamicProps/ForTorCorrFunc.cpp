/*
 * Copyright (c) 2005 The University of Notre Dame. All Rights Reserved.
 *
 * The University of Notre Dame grants you ("Licensee") a
 * non-exclusive, royalty free, license to use, modify and
 * redistribute this software in source and binary code form, provided
 * that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 * This software is provided "AS IS," without a warranty of any
 * kind. All express or implied conditions, representations and
 * warranties, including any implied warranty of merchantability,
 * fitness for a particular purpose or non-infringement, are hereby
 * excluded.  The University of Notre Dame and its licensors shall not
 * be liable for any damages suffered by licensee as a result of
 * using, modifying or distributing the software or its
 * derivatives. In no event will the University of Notre Dame or its
 * licensors be liable for any lost revenue, profit or data, or for
 * direct, indirect, special, consequential, incidental or punitive
 * damages, however caused and regardless of the theory of liability,
 * arising out of the use of or inability to use software, even if the
 * University of Notre Dame has been advised of the possibility of
 * such damages.
 *
 * SUPPORT OPEN SCIENCE!  If you use OpenMD or its source code in your
 * research, please cite the appropriate papers when you publish your
 * work.  Good starting points are:
 *
 * [1]  Meineke, et al., J. Comp. Chem. 26, 252-271 (2005).
 * [2]  Fennell & Gezelter, J. Chem. Phys. 124, 234104 (2006).
 * [3]  Sun, Lin & Gezelter, J. Chem. Phys. 128, 234107 (2008).
 * [4]  Kuang & Gezelter,  J. Chem. Phys. 133, 164101 (2010).
 * [5]  Vardeman, Stocker & Gezelter, J. Chem. Theory Comput. 7, 834 (2011).
 */

#include "applications/dynamicProps/ForTorCorrFunc.hpp"
#include "math/SquareMatrix3.hpp"//may not be necessary

namespace OpenMD {
  ForTorCorrFunc::ForTorCorrFunc(SimInfo* info,
                                 const std::string& filename,
                                 const std::string& sele1,
                                 const std::string& sele2)
    : templatedCrossCorrFunc<Mat3x3d>(info, filename, sele1, sele2,
                          DataStorage::dslForce | DataStorage::dslAmat |
                          DataStorage::dslTorque){

    setCorrFuncType("Force - Torque Correlation Function");
    setOutputName(getPrefix(dumpFilename_) + ".ftcorr");

    forces_.resize(nFrames_);
    torques_.resize(nFrames_);

    sumForces_ = V3Zero;
    sumTorques_ = V3Zero;

    forcesCount_ = 0;
    torquesCount_ = 0;

    propertyTemp = V3Zero;
  }

  int ForTorCorrFunc::computeProperty1(int frame, StuntDouble* sd) {
    propertyTemp = sd->getA() * sd->getFrc();
    forces_[frame].push_back(propertyTemp);
    sumForces_ += propertyTemp;
    forcesCount_++;
    return forces_[frame].size() - 1;
  }

  int ForTorCorrFunc::computeProperty2(int frame, StuntDouble* sd) {
    propertyTemp = sd->getA() * sd->getTrq();
    torques_[frame].push_back(propertyTemp);
    sumTorques_ += propertyTemp;
    torquesCount_++;
    return torques_[frame].size() - 1;
  }

  Mat3x3d ForTorCorrFunc::calcCorrVal(int frame1, int frame2,
                                      int id1, int id2) {
    return outProduct( forces_[frame1][id1] , torques_[frame2][id2] );
  }

  void ForTorCorrFunc::postCorrelate() {
    sumForces_ /= RealType(forcesCount_); //gets the average of the forces_
    sumTorques_ /= RealType(torquesCount_);
    Mat3x3d correlationOfAverages_ = outProduct(sumForces_, sumTorques_);
    for (int i =0 ; i < nTimeBins_; ++i) {
      if (count_[i] > 0) {
	       histogram_[i] /= RealType(count_[i]);//divides matrix by count_[i]
         histogram_[i] -= correlationOfAverages_;//the outerProduct correlation of the averages is subtracted from the correlation value
      } else {
        histogram_[i] = M3Zero;
      }
    }
  }

}

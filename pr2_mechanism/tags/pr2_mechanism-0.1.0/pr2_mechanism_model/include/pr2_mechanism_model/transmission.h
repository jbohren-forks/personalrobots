/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2008, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Willow Garage nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/
/*
 * Author: Stuart Glaser
 */
#ifndef TRANSMISSION_H
#define TRANSMISSION_H

#include <tinyxml/tinyxml.h>
#include "pr2_mechanism_model/joint.h"
#include "pr2_hardware_interface/hardware_interface.h"

namespace pr2_mechanism {

class Robot;

class Transmission
{
public:
  Transmission() {}
  virtual ~Transmission() {}

  // Initializes the transmission from XML data
  virtual bool initXml(TiXmlElement *config, Robot *robot) = 0;

  // Uses encoder data to fill out joint position and velocities
  virtual void propagatePosition(std::vector<Actuator*>&, std::vector<JointState*>&) = 0;

  // Uses the joint position to fill out the actuator's encoder.
  virtual void propagatePositionBackwards(std::vector<JointState*>&, std::vector<Actuator*>&) = 0;

  // Uses commanded joint efforts to fill out commanded motor currents
  virtual void propagateEffort(std::vector<JointState*>&, std::vector<Actuator*>&) = 0;

  // Uses the actuator's commanded effort to fill out the torque on
  // the joint.
  virtual void propagateEffortBackwards(std::vector<Actuator*>&, std::vector<JointState*>&) = 0;

  std::string name_;

  // Specifies the names of the actuators and joints that this
  // transmission uses.  In the propagate* methods, the order of
  // actuators and joints in the parameters matches the order in
  // actuator_names_ and in joint_names_.
  std::vector<std::string> actuator_names_;
  std::vector<std::string> joint_names_;
};

} // namespace pr2_mechanism

#endif

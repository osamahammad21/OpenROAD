/* Authors: Osama */
/*
 * Copyright (c) 2022, The Regents of the University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include <boost/serialization/base_object.hpp>
#include <string>

#include "dst/JobMessage.h"
namespace boost::serialization {
class access;
}
namespace fr {

class FrankensteinJobDescription : public dst::JobDescription
{
 public:
  FrankensteinJobDescription()
      : gr_seed_(-1), drt_run_time_(0), grt_run_time_(0), drvs_(-1), reply_port_(0)
  {
  }
  // Setter
  void setDesignPath(const std::string& value) { design_path_ = value; }
  void setGrSeed(const int& value) { gr_seed_ = value; }
  void setDrtRunTime(const int& value) { drt_run_time_ = value; }
  void setGrtRunTime(const int& value) { grt_run_time_ = value; }
  void setDrvs(const int& value) { drvs_ = value; }
  void setReplyHost(const std::string& value) { reply_host_ = value; }
  void setReplyPort(const ushort& value) { reply_port_ = value; }
  // Getters
  std::string getDesignPath() const { return design_path_; }
  int getGrSeed() const { return gr_seed_; }
  int getDrtRunTime() const { return drt_run_time_; }
  int getGrtRunTime() const { return grt_run_time_; }
  int getDrvs() const { return drvs_; }
  std::string getReplyHost() const { return reply_host_; }
  ushort getReplyPort() const { return reply_port_; }

 private:
  std::string design_path_;
  int gr_seed_;
  int drt_run_time_;
  int grt_run_time_;
  int drvs_;
  std::string reply_host_;
  ushort reply_port_;

  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    (ar) & boost::serialization::base_object<dst::JobDescription>(*this);
    (ar) & design_path_;
    (ar) & gr_seed_;
    (ar) & drt_run_time_;
    (ar) & grt_run_time_;
    (ar) & drvs_;
    (ar) & reply_host_;
    (ar) & reply_port_;
  }
  friend class boost::serialization::access;
};
}  // namespace fr

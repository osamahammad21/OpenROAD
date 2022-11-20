/* Authors: Osama */
/*
 * Copyright (c) 2021, The Regents of the University of California
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
#include "dr/FlexDR.h"
namespace boost::serialization {
class access;
}
namespace fr {

class StubbornRoutingJobDescription : public dst::JobDescription
{
 public:
  StubbornRoutingJobDescription() : worker_id_(0), reply_port_(0) {}
  void setWorker(const std::string& value) { worker_ = value; }
  void setWorkerId(const int& value) { worker_id_ = value; }
  void setArgs(const std::vector<SearchRepairArgs>& value) {args_ = value; }
  void setReplyPort(ushort value) { reply_port_ = value; }
  void setReplyHost(const std::string& value) { reply_host_ = value; }
  const std::string& getWorker() const { return worker_; }
  int getWorkerId() const { return worker_id_; }
  const std::vector<SearchRepairArgs>& getArgs() const { return args_; }
  ushort getReplyPort() const { return reply_port_; }
  std::string getReplyHost() const { return reply_host_; }

 private:
  // std::vector<std::tuple<int, std::string, SearchRepairArgs>> workers_;
  std::string worker_;
  int worker_id_;
  std::vector<SearchRepairArgs> args_;
  ushort reply_port_;
  std::string reply_host_;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    (ar) & boost::serialization::base_object<dst::JobDescription>(*this);
    (ar) & worker_;
    (ar) & worker_id_;
    (ar) & args_;
    (ar) & reply_port_;
    (ar) & reply_host_;
  }
  friend class boost::serialization::access;
};
}  // namespace fr

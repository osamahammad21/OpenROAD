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

#include "WorkerResult.h"
#include "dst/JobMessage.h"
#include "dr/FlexDR.h"
namespace boost::serialization {
class access;
}
namespace fr {

class MLJobDescription : public dst::JobDescription
{
 public:
  MLJobDescription() {}
  void setWorkers(const std::vector<std::pair<int, std::string>>& workers)
  {
    workers_ = workers;
  }
  void setResult(const WorkerResult& result)
  {
    result_ = result;
  }
  void setReplyPort(ushort value) { reply_port_ = value; }
  void setReplyHost(const std::string& value) { reply_host_ = value; }
  const std::vector<std::pair<int, std::string>>& getWorkers() const
  {
    return workers_;
  }
  WorkerResult getResult() const
  {
    return result_;
  }
  ushort getReplyPort() const { return reply_port_; }
  std::string getReplyHost() const { return reply_host_; }
  std::string odb_path_;
  std::string init_globals_path_;
  std::string updates_path_;
  std::string via_data_path_;
  std::string worker_path_;
  std::string worker_globals_path_;
  vector<SearchRepairArgs> strategies_;

 private:
  std::vector<std::pair<int, std::string>> workers_;
  WorkerResult result_;
  ushort reply_port_;
  std::string reply_host_;
  
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version)
  {
    (ar) & boost::serialization::base_object<dst::JobDescription>(*this);
    (ar) & workers_;
    (ar) & result_;
    (ar) & reply_port_;
    (ar) & reply_host_;
    (ar) & odb_path_;
    (ar) & init_globals_path_;
    (ar) & updates_path_;
    (ar) & via_data_path_;
    (ar) & worker_path_;
    (ar) & worker_globals_path_;
    (ar) & strategies_;
  }
  friend class boost::serialization::access;
};
}  // namespace fr

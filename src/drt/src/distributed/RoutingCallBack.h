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
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#include <boost/asio/post.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/bind/bind.hpp>
#include <iostream>
#include <mutex>

#include "db/infra/frTime.h"
#include "distributed/RoutingJobDescription.h"
#include "distributed/MLJobDescription.h"
#include "distributed/frArchive.h"
#include "dr/FlexDR.h"
#include "dst/Distributed.h"
#include "dst/JobCallBack.h"
#include "dst/JobMessage.h"
#include "global.h"
#include "ord/OpenRoad.hh"
#include "triton_route/TritonRoute.h"
#include "utl/Logger.h"
#include <chrono>
using namespace std::chrono;
namespace asio = boost::asio;
namespace odb {
class dbDatabase;
}
namespace fr {

class RoutingCallBack : public dst::JobCallBack
{
 public:
  RoutingCallBack(triton_route::TritonRoute* router,
                  dst::Distributed* dist,
                  utl::Logger* logger)
      : router_(router), dist_(dist), logger_(logger), init_(true)
  {
  }
  void onRoutingJobReceived(dst::JobMessage& msg, dst::socket& sock) override
  {
    if (msg.getJobType() == dst::JobMessage::ROUTING_INITIAL)
      handleInitialRoutingJob(msg, sock);
    else if (msg.getJobType() == dst::JobMessage::ROUTING_STUBBORN)
      handleStubbornTilesJob(msg, sock);
  }
  void onRoutingStubbornResultReceived(dst::JobMessage& msg, dst::socket& sock) override
  {
    MLJobDescription* desc = static_cast<MLJobDescription*>(msg.getJobDescription());
    router_->addWorkerResults({desc->getResult()});
    {
      dst::JobMessage result(dst::JobMessage::NONE);
      dist_->sendResult(result, sock);
      sock.close();
    }
  }

  void handleInitialRoutingJob(dst::JobMessage& msg, dst::socket& sock)
  {
    RoutingJobDescription* desc
        = static_cast<RoutingJobDescription*>(msg.getJobDescription());
    omp_set_num_threads(ord::OpenRoad::openRoad()->getThreadCount());
    auto workers = desc->getWorkers();
    int size = workers.size();
    std::vector<std::pair<int, std::string>> results;
    asio::thread_pool reply_pool(1);
    int prev_perc = 0;
    int cnt = 0;
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < workers.size(); i++) {
      std::pair<int, std::string> result
          = {workers.at(i).first,
             router_->runDRWorker(workers.at(i).second, &via_data_)};
      #pragma omp critical
      {
        results.push_back(result);
        ++cnt;
        if (cnt * 1.0 / size >= prev_perc / 100.0 + 0.1 && prev_perc < 90) {
          prev_perc += 10;
          if (prev_perc % desc->getSendEvery() == 0) {
            asio::post(reply_pool,
                       boost::bind(&RoutingCallBack::sendResult,
                                   this,
                                   results,
                                   boost::ref(sock),
                                   false,
                                   cnt));
            results.clear();
          }
        }
      }
    }
    reply_pool.join();
    sendResult(results, sock, true, cnt);
  }

  void handleStubbornTilesJob(dst::JobMessage& msg, dst::socket& sock)
  {
    MLJobDescription* desc
        = static_cast<MLJobDescription*>(msg.getJobDescription());
    auto remote_ip = sock.remote_endpoint().address().to_string();
    {
      dst::JobMessage result(dst::JobMessage::NONE);
      dist_->sendResult(result, sock);
      sock.close();
    }
    omp_set_num_threads(ord::OpenRoad::openRoad()->getThreadCount());
    auto workers = desc->getWorkers();
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < workers.size(); i++) {
      high_resolution_clock::time_point t0 = high_resolution_clock::now();
      std::unique_ptr<FlexDRWorker> uWorker;
      router_->runDRWorker(uWorker, workers.at(i).second, &via_data_);
      high_resolution_clock::time_point t1 = high_resolution_clock::now();
      seconds time_span = duration_cast<seconds>(t1 - t0);
      WorkerResult result;
      result.id = workers.at(i).first;
      result.numOfViolations = uWorker->getBestNumMarkers();
      result.runTime = time_span.count();
      std::unique_ptr<MLJobDescription> resultDesc = std::make_unique<MLJobDescription>();
      resultDesc->setResult(result);
      dst::JobMessage resultMsg(dst::JobMessage::ROUTING_STUBBORN_RESULT);
      resultMsg.setJobDescription(std::move(resultDesc));
      dst::JobMessage dummy;
      dist_->sendJob(resultMsg, remote_ip.c_str(), desc->getReplyPort(), dummy);
      #pragma omp critical
      debugPrint(logger_,
                 utl::DRT,
                 "autotuner",
                 1,
                 "Number of markers {} elapsed time {:02}:{:02}:{:02}", 
                 result.numOfViolations,
                 frTime::getHours(result.runTime),
                 frTime::getMinutes(result.runTime),
                 frTime::getSeconds(result.runTime));
    }
    logger_->report("########Done########");
  }

  void onFrDesignUpdated(dst::JobMessage& msg, dst::socket& sock) override
  {
    if (msg.getJobType() != dst::JobMessage::UPDATE_DESIGN)
      return;
    dst::JobMessage result(dst::JobMessage::UPDATE_DESIGN);
    RoutingJobDescription* desc
        = static_cast<RoutingJobDescription*>(msg.getJobDescription());
    if (desc->getGlobalsPath() != "") {
      if (globals_path_ != desc->getGlobalsPath()) {
        globals_path_ = desc->getGlobalsPath();
        router_->setSharedVolume(desc->getSharedDir());
        router_->updateGlobals(desc->getGlobalsPath().c_str());
      }
    }
    if ((desc->isDesignUpdate() && !desc->getUpdates().empty())
        || desc->getDesignPath() != "") {
      frTime t;
      logger_->report("Design Update");
      if (desc->isDesignUpdate())
        if(desc->getUpdates().size() == 1)
          router_->updateDesign(desc->getUpdates()[0]);
        else
          router_->updateDesign(desc->getUpdates());
      else
        router_->resetDb(desc->getDesignPath().c_str());
      t.print(logger_);
    }
    if (!desc->getViaData().empty()) {
      std::stringstream stream(
          desc->getViaData(),
          std::ios_base::binary | std::ios_base::in | std::ios_base::out);
      frIArchive ar(stream);
      ar >> via_data_;
    }
    dist_->sendResult(result, sock);
    sock.close();
  }

 private:
  void sendResult(std::vector<std::pair<int, std::string>> results,
                  dst::socket& sock,
                  bool finish,
                  int cnt)
  {
    dst::JobMessage result;
    if (finish)
      result.setJobType(dst::JobMessage::SUCCESS);
    else
      result.setJobType(dst::JobMessage::NONE);
    auto uResultDesc = std::make_unique<RoutingJobDescription>();
    auto resultDesc = static_cast<RoutingJobDescription*>(uResultDesc.get());
    resultDesc->setWorkers(results);
    result.setJobDescription(std::move(uResultDesc));
    dist_->sendResult(result, sock);
    if (finish)
      sock.close();
  }
  
  triton_route::TritonRoute* router_;
  dst::Distributed* dist_;
  utl::Logger* logger_;
  std::string design_path_;
  std::string globals_path_;
  bool init_;
  FlexDRViaData via_data_;
};

}  // namespace fr

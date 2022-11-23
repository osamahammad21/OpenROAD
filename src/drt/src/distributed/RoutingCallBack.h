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
#include "distributed/StubbornRoutingJobDescription.h"
#include "distributed/RoutingResultDescription.h"
#include "distributed/MLJobDescription.h"
#include "distributed/TimeOutDescription.h"
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
      : router_(router), dist_(dist), logger_(logger), init_(true), busy_(false)
  {
    routing_pool_ = std::make_unique<asio::thread_pool>(ord::OpenRoad::openRoad()->getThreadCount());
  }

  void onTimeOut(dst::JobMessage& msg, dst::socket& sock) override
  {
    TimeOutDescription* desc = static_cast<TimeOutDescription*>(msg.getJobDescription());
    if(MAX_OPS != -2)
      MAX_OPS = desc->getMaxOps();
    if(desc->getBannedId() != -1)
      router_->banWorker(desc->getBannedId());
    if(MAX_OPS == -1)
      router_->clearBannerWorkers();
    dst::JobMessage result(dst::JobMessage::SUCCESS);
    dist_->sendResult(result, sock);
    sock.close();
  }

  void onRoutingResultRequested(dst::JobMessage& msg, dst::socket& sock) override
  {
    RoutingResultDescription* desc = static_cast<RoutingResultDescription*>(msg.getJobDescription());
    {
      dst::JobMessage result(dst::JobMessage::NONE);
      dist_->sendResult(result, sock);
      sock.close();
    }
    auto requestedResult = desc->getResult();
    if(keep_results_.find(requestedResult.id) != keep_results_.end())
    {
      auto keep_result = keep_results_[requestedResult.id];
      if(keep_result.args == requestedResult.args)
      {
        dst::JobMessage result(dst::JobMessage::ROUTING_STUBBORN_RESULT), dummy;
        auto resultDesc = std::make_unique<RoutingResultDescription>();
        resultDesc->setResult(keep_result);
        result.setJobDescription(std::move(resultDesc));
        dist_->sendJob(result, desc->getReplyHost().c_str(), desc->getReplyPort(), dummy);
        return;
      }
    }
  }

  void onRoutingJobReceived(dst::JobMessage& msg, dst::socket& sock) override
  {
    busy_ = true;
    if (msg.getJobType() == dst::JobMessage::ROUTING_INITIAL)
      handleInitialRoutingJob(msg, sock);
    else if (msg.getJobType() == dst::JobMessage::ROUTING_STUBBORN)
      handleStubbornTilesJob(msg, sock);
  }

  void onRoutingResultReceived(dst::JobMessage& msg, dst::socket& sock) override
  {
    RoutingResultDescription* desc = static_cast<RoutingResultDescription*>(msg.getJobDescription());
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
    busy_ = false;
  }
  
  void handleStubbornTilesJobHelper2(MLJobDescription desc)
  {
    router_->updateGlobals(desc.init_globals_path_.c_str());
    router_->resetDb(desc.odb_path_.c_str());
    router_->updateDesign(desc.updates_path_.c_str());
    router_->updateGlobals(desc.worker_globals_path_.c_str());
    std::ifstream viaDataFile(desc.via_data_path_,
                              std::ios::binary);
    frIArchive ar(viaDataFile);
    ar >> via_data_;
    std::ifstream workerFile(desc.worker_path_,
                           std::ios::binary);
    std::string workerStr((std::istreambuf_iterator<char>(workerFile)),
                          std::istreambuf_iterator<char>());
    workerFile.close();

    omp_set_num_threads(ord::OpenRoad::openRoad()->getThreadCount());
    auto strategies = desc.strategies_;
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < strategies.size(); i++) {
      auto args = strategies.at(i);
      auto worker = FlexDRWorker::load(workerStr, logger_, router_->getDesign(), nullptr); 
      worker->setMazeEndIter(args.mazeEndIter);
      worker->setMarkerCost(args.workerMarkerCost);
      worker->setDrcCost(args.workerDRCCost);
      worker->setRipupMode(args.ripupMode);
      worker->setFollowGuide((args.followGuide));
      worker->setViaData(&via_data_);

      high_resolution_clock::time_point t0 = high_resolution_clock::now();
      worker->reloadedMain();
      high_resolution_clock::time_point t1 = high_resolution_clock::now();
      seconds time_span = duration_cast<seconds>(t1 - t0);
      WorkerResult result;
      result.id = args.offset;
      if(MAX_OPS != -1 && worker->getHeapOps() > MAX_OPS)
        result.numOfViolations = -1;
      else
        result.numOfViolations = worker->getBestNumMarkers();
      result.runTime = time_span.count();
      result.heapOps = worker->getHeapOps();
      #pragma omp critical
      debugPrint(logger_,
                 utl::DRT,
                 "distributed",
                 1,
                 "Number of markers {} elapsed time {:02}:{:02}:{:02}", 
                 result.numOfViolations,
                 frTime::getHours(result.runTime),
                 frTime::getMinutes(result.runTime),
                 frTime::getSeconds(result.runTime));
      std::unique_ptr<RoutingResultDescription> resultDesc = std::make_unique<RoutingResultDescription>();
      resultDesc->setResult(result);
      dst::JobMessage resultMsg(dst::JobMessage::ROUTING_STUBBORN_RESULT);
      resultMsg.setJobDescription(std::move(resultDesc));
      dst::JobMessage dummy;
      dist_->sendJob(resultMsg, desc.getReplyHost().c_str(), desc.getReplyPort(), dummy);
    }
    logger_->report("########Done########");
    busy_ = false;
  }
  void handleStubbornTilesJobHelper(const std::string workerStr, 
                                    const int workerId, 
                                    const std::string replyHost, 
                                    const ushort replyPort,
                                    const SearchRepairArgs args)
  {
    auto worker = FlexDRWorker::load(workerStr, logger_, router_->getDesign(), nullptr); 
    worker->setViaData(&via_data_);
    worker->setMazeEndIter(args.mazeEndIter);
    worker->setMarkerCost(args.workerMarkerCost);
    worker->setDrcCost(args.workerDRCCost);
    worker->setRipupMode(args.ripupMode);
    worker->setFollowGuide(args.followGuide);
    worker->setWorkerId(fmt::format("{}_{}_{}_{}", args.mazeEndIter, args.workerDRCCost, args.workerMarkerCost, args.followGuide));
    worker->setRouter(router_);
    high_resolution_clock::time_point t0 = high_resolution_clock::now();
    worker->reloadedMain();
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    seconds time_span = duration_cast<seconds>(t1 - t0);
    WorkerResult result;
    result.id = workerId;
    if((MAX_OPS != -1 && worker->getHeapOps() > MAX_OPS) || router_->isWorkerBanned(workerId))
      result.numOfViolations = -1;
    else
      result.numOfViolations = worker->getBestNumMarkers();
    result.runTime = time_span.count();
    result.heapOps = worker->getHeapOps();
    result.args = args;
    result.workerStr = worker->getSerializedWorker();
    debugPrint(logger_,
                utl::DRT,
                "distributed",
                1,
                "Number of markers {} elapsed time {:02}:{:02}:{:02}", 
                result.numOfViolations,
                frTime::getHours(result.runTime),
                frTime::getMinutes(result.runTime),
                frTime::getSeconds(result.runTime));
    std::unique_ptr<RoutingResultDescription> resultDesc = std::make_unique<RoutingResultDescription>();
    resultDesc->setResult(result);
    dst::JobMessage resultMsg(dst::JobMessage::ROUTING_STUBBORN_RESULT);
    resultMsg.setJobDescription(std::move(resultDesc));
    dst::JobMessage dummy;
    dist_->sendJob(resultMsg, replyHost.c_str(), replyPort, dummy);
  }
  void handleStubbornTilesJob(dst::JobMessage& msg, dst::socket& sock)
  {
    {
      dst::JobMessage result(dst::JobMessage::NONE);
      dist_->sendResult(result, sock);
      sock.close();
    }
    keep_results_.clear();
    StubbornRoutingJobDescription* desc
      = static_cast<StubbornRoutingJobDescription*>(msg.getJobDescription());
    auto workerStr = desc->getWorker();
    auto workerId = desc->getWorkerId();
    auto replyHost = desc->getReplyHost();
    auto replyPort = desc->getReplyPort();
    for(auto args : desc->getArgs())
    {
      asio::post(*routing_pool_.get(), boost::bind(&RoutingCallBack::handleStubbornTilesJobHelper, this, workerStr, workerId, replyHost, replyPort, args));
    }
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
      logger_->report("Design Update {}", desc->isDesignUpdate());
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
    // sock.close();
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
  std::unique_ptr<asio::thread_pool> routing_pool_;
  std::map<int, WorkerResult> keep_results_;
  bool busy_;
};

}  // namespace fr

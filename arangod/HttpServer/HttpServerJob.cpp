////////////////////////////////////////////////////////////////////////////////
/// @brief general server job
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014-2015 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Achim Brandt
/// @author Copyright 2014-2015, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2009-2014, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "HttpServerJob.h"

#include "Basics/WorkMonitor.h"
#include "Basics/logging.h"
#include "Dispatcher/DispatcherQueue.h"
#include "HttpServer/AsyncJobManager.h"
#include "HttpServer/HttpCommTask.h"
#include "HttpServer/HttpHandler.h"
#include "HttpServer/HttpServer.h"
#include "Scheduler/Scheduler.h"

using namespace arangodb;
using namespace triagens::rest;
using namespace std;

// -----------------------------------------------------------------------------
// --SECTION--                                               class HttpServerJob
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief constructs a new server job
////////////////////////////////////////////////////////////////////////////////

HttpServerJob::HttpServerJob(HttpServer *server,
                             WorkItem::uptr<HttpHandler> &handler)
    : Job("HttpServerJob"), _server(server) {
  _handler.swap(handler);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destructs a server job
////////////////////////////////////////////////////////////////////////////////

HttpServerJob::~HttpServerJob () {
  WorkMonitor::releaseHandler(_handler);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       Job methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

size_t HttpServerJob::queue () const {
  return _handler->queue();
}

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

Job::status_t HttpServerJob::work() {
  TRI_ASSERT(_handler != nullptr);

  LOG_TRACE("beginning job %p", (void*)this);

  // start working with handler
  HttpHandler::status_t status;

  {
    HandlerWorkStack work(_handler, false);
    this->RequestStatisticsAgent::transfer(
        _handler.get());  // FMH TODO(fc) XXX MOVE to WorkMonitor?

    status = _handler->executeFull();
  }

  LOG_TRACE("finished job %p with status %d", (void*)this, (int)status.status);

  /* TODO(fc) XXXX
  if (!isDetached()) {
  */
    std::unique_ptr<TaskData> data(new TaskData());
    data->_taskId = _handler->taskId();
    data->_loop = _handler->eventLoop();
    data->_type = HttpCommTask::TASK_DATA_RESPONSE; // TODO(fc) XXX this should be TaskData::
    data->_response = _handler->stealResponse();

    WorkMonitor::releaseHandler(_handler);
    _handler = nullptr;

    Scheduler::SCHEDULER->signalTask(data);
    //  }

  return status.jobStatus();
}

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

bool HttpServerJob::cancel () {
  return _handler->cancel();
}

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

void HttpServerJob::cleanup (DispatcherQueue* queue) {
  /* TODO(fc) XXXX
  if (isDetached()) {
    _server->jobManager()->finishAsyncJob(this);
  }
  */

  queue->removeJob(this);

  /* ZODO(fc) XXXX
  if (--_refCount == 0) {
    delete this;
  }
  */
}

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

void HttpServerJob::beginShutdown () {
  LOG_TRACE("shutdown job %p", (void*) this);

  /* TODO(fc) XXXXX
  if (--_refCount == 0) {
    delete this;
  }
  */
}

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

void HttpServerJob::handleError (triagens::basics::Exception const& ex) {
  _handler->handleError(ex);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

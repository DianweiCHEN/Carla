// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once

#include <chrono>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>
#include <thread>

#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>
#include <cstring>

#ifdef _WIN32
// TODO: Test on Windows
// #define WIN32_LEAN_AND_MEAN
// #include <windows.h>
#else
#include <unistd.h>
#endif


#define TRACE_ENABLE 1
#define NUM_TRACER_EVENTS 10000000

namespace carla {
namespace profiler {
namespace detail {

class Tracer {

  struct MetadataEvent {
    char _name[32];
    char _category[32];
    uint32_t _tid = 0; // thread ID
  };

  struct Event : MetadataEvent {
    uint64_t _timestamp = 0; // microseconds
    uint64_t _duration = 0; // microseconds
    char _event_type = '\0';

    void WriteToFile(std::ofstream& file, uint32_t pid) const {

      // I assume that file is open
      file << "{";
      file << "\"name\": \"" << _name << "\", ";
      file << "\"cat\": \"" << _category << "\", ";
      file << "\"ph\": \"" << _event_type << "\", ";
      file << "\"ts\": " << _timestamp << ", ";
      file << "\"dur\": " << _duration << ", ";
      file << "\"pid\": " << pid << ", ";
      file << "\"tid\": " << _tid << ", ";
      // TODO: add args
      file << "\"args\": {}";
      file << "}";

    }
  };

public:

  static Tracer& GetInstance() {
    static Tracer* instance = new Tracer();
    return *instance;
  }

  ~Tracer() {
    // Flush();
  }

  void Flush() {
    std::lock_guard<std::mutex> lock(_mutex);

    std::cout << "Events = " << _events.size() << std::endl;

    std::ofstream file(_output_file, std::ios_base::out);
    // Header
    file << "{\"traceEvents\":[" << std::endl;

    // Metadata
    for(MetadataEvent& metadata_event : _metadata_events) {
      file << "{";
      file << "\"name\": \"" << metadata_event._category << "\", ";
      file << "\"ph\": \"M\", ";
      file << "\"pid\": " << _pid << ", ";
      file << "\"tid\": " << metadata_event._tid << ", ";
      // TODO: add args
      file << "\"args\": {\"name\" : \"" << metadata_event._name << "\"}";
      file << "}," << std::endl;;
    }

    // Events
    size_t num_events = (_events.size() > 0) ? (_events.size() - 1) : 0;
    for(size_t i = 0; i < num_events; i++) {
      const Event& event = _events[i];
      event.WriteToFile(file, _pid);
      file << "," << std::endl;
    }
    if(num_events > 0) {
      // Last case can't have the comma (,) so I prefer to avoid the if condition in the loop
      _events[_events.size() - 1].WriteToFile(file, _pid);
    }

    // Close
    file << std::endl << "]}" << std::endl;
    file.close();
  }

  void AddEvent(const char *name, const char *category, char event_type) {
    std::lock_guard<std::mutex> lock(_mutex);

    Event event;

    std::strncpy(event._name, name, 32);
    std::strncpy(event._category, category, 32);
    event._timestamp = GetTimeStamp();
    event._tid = GetThreadID();
    event._event_type = event_type;

    _events.push_back(event);
  }

  void AddScopedEvent(const char *name, const char *category, uint64_t timestamp, uint64_t duration)  {
    if(duration < 1) return;
    std::lock_guard<std::mutex> lock(_mutex);

    Event event;

    std::strncpy(event._name, name, 32);
    std::strncpy(event._category, category, 32);
    event._timestamp = timestamp;
    event._duration = duration;
    event._tid = GetThreadID();
    event._event_type = 'X';

    _events.push_back(event);

  }

  void AddMetaDataEvent(const char *name, const char *value)  {
    std::lock_guard<std::mutex> lock(_mutex);

    MetadataEvent metadata_event;

    std::strncpy(metadata_event._name, name, 32);
    std::strncpy(metadata_event._category, value, 32);
    metadata_event._tid = GetThreadID();

    _metadata_events.push_back(metadata_event);
  }

  uint64_t GetTimeStamp() const {
    Chrono::duration d = Chrono::now() - _initial_timestamp;
    int64_t diff = std::chrono::duration_cast<std::chrono::milliseconds>(d).count();
    return static_cast<uint64_t>(diff);
  }

private:

  using Chrono = std::chrono::high_resolution_clock;

  Tracer() {
    _events.reserve(NUM_TRACER_EVENTS);
    _initial_timestamp = Chrono::now();
    _pid = GetProcessID();
  }

  uint32_t GetProcessID() {
#ifdef _WIN32
    return static_cast<uint32_t>(GetCurrentProcessId());
#else
    return static_cast<uint32_t>(getpid());
#endif
  }

  uint32_t GetThreadID() {
    std::thread::id tid = std::this_thread::get_id();
    auto tid_it = _tids.find(tid);

    if (tid_it!=_tids.end())
    {
      return tid_it->second;
    }

    // Since the conversion from thread::id to integer is not trivial and I only need
    // an id to identify the thread, I assign one myself
    uint32_t result = static_cast<uint32_t>(_tids.size());
    _tids.insert({tid, result});

    return result;
  }

  std::unordered_map<std::thread::id, uint32_t> _tids;

  std::vector<Event> _events;
  std::vector<MetadataEvent> _metadata_events;

  std::mutex _mutex;

  std::string _output_file = "trace.json";

  Chrono::time_point _initial_timestamp;

  uint32_t _pid;
};

struct ScopedEvent {
  ScopedEvent(const char *name, const char *category)
    : _timestamp(Tracer::GetInstance().GetTimeStamp()) {
    std::strncpy(_name, name, 32);
    std::strncpy(_category, category, 32);
  }

  ~ScopedEvent() {
    uint64_t duration = Tracer::GetInstance().GetTimeStamp() - _timestamp;
    Tracer::GetInstance().AddScopedEvent(_name, _category, _timestamp, duration);
  }

  char _name[32];
  char _category[32];
  uint64_t _timestamp;
};

} // namespace detail
} // namespace profiler
} // namespace carla


#if TRACE_ENABLE

#define TRACE_INIT() carla::profiler::detail::Tracer::GetInstance(); // carla::profiler::detail::InitTracer();// carla::profiler::detail::Tracer::GetInstance();
#define TRACE_SHUTDOWN() carla::profiler::detail::Tracer::GetInstance().Flush();
#define TRACE_PROCESS_NAME(name) carla::profiler::detail::Tracer::GetInstance().AddMetaDataEvent(name, "process_name");
#define TRACE_THREAD_NAME(name) carla::profiler::detail::Tracer::GetInstance().AddMetaDataEvent(name, "thread_name");
#define TRACE_BEGIN(name, category) carla::profiler::detail::Tracer::GetInstance().AddEvent(name, category, 'B');
#define TRACE_END(name, category) carla::profiler::detail::Tracer::GetInstance().AddEvent(name, category, 'E');
#define TRACE_SCOPE(name, category) carla::profiler::detail::ScopedEvent ___scoped_event___(name, category);
#define TRACE_SCOPE_FUNCTION(category) TRACE_SCOPE(__FUNCTION__, category)
#else
#define TRACE_INIT()
#define TRACE_SHUTDOWN()
#define TRACE_PROCESS_NAME(name)
#define TRACE_THREAD_NAME(name)
#define TRACE_BEGIN(name, category)
#define TRACE_END(name, category)
#define TRACE_SCOPE(name, category)
#endif // TRACE_ENABLE
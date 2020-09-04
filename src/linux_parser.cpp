#include <experimental/filesystem>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <dirent.h>
#include <unistd.h>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::map;

namespace fs = std::experimental::filesystem;

//HELPER FUNCTIONS

//Parse file keys for generic values
template<typename Type> Type ParseValueByKey(const string path, const string mykey) {
  string line, key;
  Type value;

  std::ifstream stream(path);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while(linestream >> key >> value) {
        if (key == mykey) {
          stream.close();
          return value;
        }
      }
    }
  }
  return value;
}

float KbToMb(float kb) {
  return kb / 1024;
}

//Split string by whitespace into a vector
vector<string> LinuxParser::SplitLine(const string line) {
  std::istringstream buffer(line);
  std::istream_iterator<string> beg(buffer), end;
  vector<string> tokens(beg, end);

  return tokens;
}

//SYSTEM
string LinuxParser::OperatingSystem() {
  string line, key, value;
  std::ifstream stream(kOSPath); 
    if (stream.is_open()) {
      while (std::getline(stream, line)) {
        std::replace(line.begin(), line.end(), ' ', '_');
        std::replace(line.begin(), line.end(), '=', ' ');
        std::replace(line.begin(), line.end(), '"', ' ');
        std::istringstream linestream(line);
        while (linestream >> key >> value) {
          if (key == "PRETTY_NAME") {
            std::replace(value.begin(), value.end(), '_', ' ');
            stream.close();
            return value;
        }
      }
    }
  }
  stream.close();
  return value;
}

string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  stream.close();
  return kernel;
}

//Store pids with std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  for (const auto &entry : fs::directory_iterator(kProcDirectory)) {
    //Is this a directory?
    if (fs::is_directory(entry)) {
      string directory_name = fs::path(entry.path()).filename();
      //Is every character of the directory name a digit?
      if (std::all_of(directory_name.begin(), directory_name.end(), isdigit)) {
        int pid = stoi(directory_name);
        pids.push_back(pid);
      }
    }
  }
  return pids;
}

float LinuxParser::MemoryUtilization() { 
  string line, key, value, kb;
  float memtotal, memfree, memutil;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while(std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (memtotal && memfree) {
          memutil = (memtotal - memfree) / memtotal;
          stream.close();
          return memutil;
        }
        if(key == "MemTotal") {
          memtotal = stof(value);
        }
        if(key == "MemFree") {
          memfree = stof(value);
        }
      }
    }
  }
  stream.close();
  return memutil; 
}

long LinuxParser::UpTime() { 
  long uptime, idletime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
    stream.close();
  }
  return uptime;
 }

//Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

//Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

//Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

//Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0 ; }

//Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  return {}; 
}

int LinuxParser::TotalProcesses() {
  int total_processes = ParseValueByKey<int>(kProcDirectory + kStatFilename, "processes");
  return total_processes;
}

int LinuxParser::RunningProcesses() {
  int running_processes = ParseValueByKey<int>(kProcDirectory + kStatFilename, "procs_running");
  return running_processes;
}

//PROCESSES 

string LinuxParser::Command(const int pid) { 
  string line;
  std::ifstream stream(kProcProcessDirectory + to_string(pid) + kCmdlineFilename); 
  if (stream.is_open()) {
    std::getline(stream, line);
    stream.close();
  }
  return line;
}

float LinuxParser::Ram(const int pid) {
  float ram_kb = ParseValueByKey<float>(kProcProcessDirectory + to_string(pid) + kStatusFilename, "VmSize");
  float ram_mb = KbToMb(ram_kb);
  return ram_mb;
 }

string LinuxParser::Uid(const int pid) { 
  const string uid = ParseValueByKey<string>(kProcProcessDirectory + to_string(pid) + kStatusFilename, "Uid");
  return uid;
 }

string LinuxParser::User(const int pid) {
  string line, username, x, uid;
  const string myuid = Uid(pid);
  std::ifstream stream(kPasswordPath);

  if (stream.is_open()) {
    while(std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username >> x >> uid) {
        if (myuid == uid) { 
          stream.close();
          return username; 
        }
      }
    }
  }
  stream.close();
  return username;
}

long int LinuxParser::UpTime(const int pid) {
  string line;
  std::ifstream stream(kProcProcessDirectory + to_string(pid) + kStatFilename);
  
  if(stream.is_open()) {
    std::getline(stream, line);
    stream.close();
  }

  vector<string> proc_stat_values = SplitLine(line);

  long unsigned int start_time = stol(proc_stat_values[21]);

  //Convert from clock ticks to seconds
  long unsigned int uptime = LinuxParser::UpTime() - (start_time / sysconf(_SC_CLK_TCK));
  return uptime;
}
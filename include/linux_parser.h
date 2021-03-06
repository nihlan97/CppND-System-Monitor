#ifndef SYSTEM_PARSER_H
#define SYSTEM_PARSER_H

#include <fstream>
#include <regex>
#include <string>

namespace LinuxParser {
// Paths
const std::string kProcDirectory{"/proc"};
const std::string kProcProcessDirectory("/proc/");
const std::string kCmdlineFilename{"/cmdline"};
const std::string kCpuinfoFilename{"/cpuinfo"};
const std::string kStatusFilename{"/status"};
const std::string kStatFilename{"/stat"};
const std::string kUptimeFilename{"/uptime"};
const std::string kMeminfoFilename{"/meminfo"};
const std::string kVersionFilename{"/version"};
const std::string kOSPath{"/etc/os-release"};
const std::string kPasswordPath{"/etc/passwd"};

// File tokens
const std::string filterProcesses("processes");
const std::string filterProcsRunning("procs_running");
const std::string filterVmData("VmData");
const std::string filterUid("Uid");
const std::string filterOS("PRETTY_NAME");
const std::string filterMemTotal("MemTotal");
const std::string filterMemFree("MemFree");

// System
float MemoryUtilization();
long UpTime();
std::vector<int> Pids();
int TotalProcesses();
int RunningProcesses();
std::string OperatingSystem();
std::string Kernel();

//Helper functions
std::vector<std::string> ParsedTokens(std::string line);

// Processes
std::string Command(int pid);
float Ram(int pid);
std::string Uid(int pid);
std::string User(int pid);
long int UpTime(int pid);
};

#endif
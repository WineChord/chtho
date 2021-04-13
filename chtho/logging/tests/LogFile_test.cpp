// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/logging/Logger.h"
#include "chtho/logging/LogFile.h"

#include <memory>
#include <unistd.h> 

std::unique_ptr<chtho::LogFile> logfile;

void outputFunc(const char* msg, int len) { logfile->append(msg, len); }
void flushFunc() { logfile->flush(); }

int main(int argc, char* argv[]) {
  // roll size is 200 KB 
  logfile.reset(new chtho::LogFile(::basename(argv[0]), 200*1000));
  chtho::Logger::setOutput(outputFunc);
  chtho::Logger::setFlush(flushFunc);
  std::string line = "qwertyuioplkjhgfdsazxcvbnm0987654321";
  for(int i = 0; i < 10000; ++i)
  {
    LOG_INFO << line << i;
    usleep(1000); // sleep 1ms
  }
}
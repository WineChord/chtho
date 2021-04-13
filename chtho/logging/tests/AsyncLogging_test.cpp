// Copyright (c) 2021 Qizhou Guo
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "chtho/logging/Logger.h"
#include "chtho/logging/AsyncLogging.h"

#include <sys/resource.h>

using namespace chtho;

AsyncLogging* alog = NULL;

void output(const char* msg, int len) { alog->append(msg, len); }

void bench(bool longlog)
{
  Logger::setOutput(output);
  int cnt = 0;
  const int batch = 1000;
  std::string empty = " ";
  std::string longstr(3000, 'X');
  longstr += empty;
  for(int t = 0; t < 30; ++t)
  {
    Timestamp start = Timestamp::now();
    for(int i = 0; i < batch; ++i)
    {
      LOG_INFO << "haha qwertyuioplkjhgfdsazxcvbnm0987654321 "
        << (longlog ? longstr : empty) << cnt;
      ++cnt;
    }
    Timestamp end = Timestamp::now();
    // us per batch 
    printf("%f\n", Timestamp::diffInSec(end, start)*1000000/batch);
    struct timespec ts = { 0, 500*1000*1000 };
    nanosleep(&ts, NULL); // sleep 0.5 s 
  }
}

off_t rollsz = 500*1000*1000; // 500MB 

int main(int argc, char const *argv[])
{
  AsyncLogging log(::basename(argv[0]), rollsz);
  log.start();
  alog = &log; 
  bool longlog = argc > 1;
  bench(longlog);
  return 0;
}

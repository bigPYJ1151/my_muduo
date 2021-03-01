#include <muduo/net/EventLoop.h>
#include <muduo/base/Thread.h>
#include <muduo/base/Logging.h>

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

EventLoop* g_loop;

void callback()
{
  printf("callback(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  // EventLoop anotherLoop;
}

void threadFunc(EventLoop* p_loop)
{
  printf("threadFunc(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());
  LOG_INFO << "Waitting...";
  sleep(1);
  LOG_INFO << "Awake p_loop";
  p_loop->runInLoop(callback);
  // assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
  // EventLoop loop;
  // assert(EventLoop::getEventLoopOfCurrentThread() == &loop);
  // loop.runAfter(1.0, callback);
  // loop.loop();

}

int main()
{
  Logger::setLogLevel(Logger::TRACE);
  printf("main(): pid = %d, tid = %d\n", getpid(), CurrentThread::tid());

  assert(EventLoop::getEventLoopOfCurrentThread() == NULL);
  EventLoop loop;
  assert(EventLoop::getEventLoopOfCurrentThread() == &loop);

  Thread thread(std::bind(threadFunc, &loop));
  thread.start();

  loop.loop();
}

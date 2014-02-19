#include <node.h>
#include <v8.h>
#include <v8-profiler.h>

#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

using namespace v8;

void catch_alarm(int sig_num);
void iterateProfileNodes(const CpuProfileNode* node, int depth=0);
void print_node(const CpuProfileNode* node, int depth);

const int profile_time = 10;
Local<String> profile_name = String::New("test");

void catch_int(int sig_num)
{
  sigset_t mask_set;  /* used to set a signal masking set. */
  sigset_t old_set;   /* used to store the old mask set.   */

    /* re-set the signal handler again to catch_int, for next time */
  signal(SIGINT, catch_int);
    /* mask any further signals while we're inside the handler. */
  sigfillset(&mask_set);
  sigprocmask(SIG_SETMASK, &mask_set, &old_set);

  printf("\nStarting profiling, %d seconds\n", profile_time);
  fflush(stdout);

  CpuProfiler::StartProfiling(profile_name);

  signal(SIGALRM, catch_alarm);
  alarm(profile_time);

  /* restore the old signal mask */
  sigprocmask(SIG_SETMASK, &old_set, NULL);
}

void catch_alarm(int sig_num)
{
  sigset_t mask_set;  /* used to set a signal masking set. */
  sigset_t old_set;   /* used to store the old mask set.   */

  signal(SIGALRM, catch_alarm);
    /* mask any further signals while we're inside the handler. */
  sigfillset(&mask_set);
  sigprocmask(SIG_SETMASK, &mask_set, &old_set);

  printf("Stoping profiling...\n");
  fflush(stdout);

  const CpuProfile* profile = CpuProfiler::StopProfiling(profile_name);
  const CpuProfileNode* node = profile->GetBottomUpRoot();
  iterateProfileNodes(node);
  exit(0); // TODO: remove

  /* restore the old signal mask */
  sigprocmask(SIG_SETMASK, &old_set, NULL);
  
}

void iterateProfileNodes(const CpuProfileNode* node, int depth)
{
  int child_count = node->GetChildrenCount();

  print_node(node, depth);

  for(int i=0;i<child_count;++i) {
    iterateProfileNodes(node->GetChild(i), depth+1);
  }
  return;
}

void print_node(const CpuProfileNode* node, int depth)
{
  const String::Utf8Value& utf8_value = String::Utf8Value(node->GetFunctionName());
  const char* node_name = *utf8_value;

  const String::Utf8Value& utf8_value2 = String::Utf8Value(node->GetScriptResourceName());
  const char* node_file = *utf8_value2;

  printf("|-");
  for (int i = 1; i < depth; ++i) {
    printf("-");
  }
  
  printf("%s - %s, %d\n", node_name, node_file, node->GetLineNumber());
  fflush(stdout);
}

Handle<Value> RegProfiler(const Arguments& args) {
  signal(SIGINT, catch_int);
  return Undefined();
}

Handle<Value> UnregProfiler(const Arguments& args) {
  signal(SIGINT, 0);
  // TODO: what if we have a runing timer this time?
  return Undefined();
}

void init(Handle<Object> target) {
  NODE_SET_METHOD(target, "regProfiler", RegProfiler);
  NODE_SET_METHOD(target, "unregProfiler", UnregProfiler);
}

NODE_MODULE(binding, init);


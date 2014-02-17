#include <node.h>
#include <v8.h>
#include <v8-profiler.h>

#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */
#include <stdlib.h>
#include <stdio.h>

using namespace v8;

void catch_alarm(int sig_num);
void iterateProfileNodes(const CpuProfileNode* node, int depth=0);
void print_node(const CpuProfileNode* node, int depth);

const int profile_time = 10;
Local<String> profile_name = String::New("test");

/* the Ctrl-C signal handler */
void catch_int(int sig_num)
{
  sigset_t mask_set;  /* used to set a signal masking set. */
  sigset_t old_set;   /* used to store the old mask set.   */

    /* re-set the signal handler again to catch_int, for next time */
  signal(SIGINT, catch_int);
    /* mask any further signals while we're inside the handler. */
  sigfillset(&mask_set);
  sigprocmask(SIG_SETMASK, &mask_set, &old_set);

    /* restore the old signal mask */
  sigprocmask(SIG_SETMASK, &old_set, NULL);
  printf("\nStarting profiling, %d seconds\n", profile_time);
  fflush(stdout);

  CpuProfiler::StartProfiling(profile_name);

  signal(SIGALRM, catch_alarm);
  alarm(profile_time);
}

void catch_alarm(int sig_num)
{
  sigset_t mask_set;  /* used to set a signal masking set. */
  sigset_t old_set;   /* used to store the old mask set.   */

  signal(SIGALRM, catch_alarm);
    /* mask any further signals while we're inside the handler. */
  sigfillset(&mask_set);
  sigprocmask(SIG_SETMASK, &mask_set, &old_set);

    /* restore the old signal mask */
  sigprocmask(SIG_SETMASK, &old_set, NULL);
  printf("Stoping profiling...\n");
  fflush(stdout);

  const CpuProfile* profile = CpuProfiler::StopProfiling(profile_name);
  const CpuProfileNode* node = profile->GetBottomUpRoot();
  iterateProfileNodes(node);
  exit(0);
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

Handle<Value> Method(const Arguments& args) {
  HandleScope scope;
  return scope.Close(String::New("world"));
}

Handle<Value> TryProfile(const Arguments& args) {
  HandleScope scope;
  return scope.Close(Number::New(CpuProfiler::GetProfilesCount()));
}

void init(Handle<Object> target) {
  signal(SIGINT, catch_int);

  NODE_SET_METHOD(target, "hello", Method);
  NODE_SET_METHOD(target, "tryProfile", TryProfile);
}

NODE_MODULE(binding, init);


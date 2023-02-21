#include <exe/executors/thread_pool.hpp>
#include <exe/executors/strand.hpp>
#include <exe/executors/execute.hpp>

#include <twist/test/with/wheels/stress.hpp>

#include <wheels/test/framework.hpp>
#include <twist/test/budget.hpp>

#include <deque>

using namespace exe::executors;
using namespace std::chrono_literals;

/////////////////////////////////////////////////////////////////////

class Robot {
 public:
  explicit Robot(IExecutor& e)
  : strand_(e) {
  }

  void Cmd() {
    Execute(strand_, [this]() {
      Step();
    });
  }

  size_t Steps() const {
    return steps_;
  }

 private:
  void Step() {
    ++steps_;
  }

 private:
  Strand strand_;
  size_t steps_{0};
};

void Robots(size_t strands, size_t load) {
  ThreadPool pool{4};

  std::deque<Robot> robots;
  for (size_t i = 0; i < strands; ++i) {
    robots.emplace_back(pool);
  }

  ThreadPool clients{strands};

  size_t iters = 0;

  while (twist::test::KeepRunning()) {
    ++iters;

    for (auto& robot : robots) {
      Execute(clients, [&robot, load]() {
        for (size_t j = 0; j < load; ++j) {
          robot.Cmd();
        }
      });
    }

    clients.WaitIdle();
    pool.WaitIdle();
  }

  for (auto& robot : robots) {
    ASSERT_EQ(robot.Steps(), iters * load);
  }

  pool.Stop();
  clients.Stop();
}

//////////////////////////////////////////////////////////////////////

void MissingTasks() {
  ThreadPool pool{4};

  size_t iter = 0;

  while (twist::test::KeepRunning()) {
    Strand strand(pool);

    size_t todo = 2 + (iter++) % 5;

    size_t done = 0;

    for (size_t i = 0; i < todo; ++i) {
      Execute(strand, [&done]() {
        ++done;
      });
    }

    pool.WaitIdle();

    ASSERT_EQ(done, todo);
  }

  pool.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(Strand) {
  TWIST_TEST(Robots_1, 5s) {
    Robots(30, 30);
  }

  TWIST_TEST(Robots_2, 5s) {
    Robots(50, 20);
  }

  TWIST_TEST(MissingTasks, 5s) {
    MissingTasks();
  }
}

RUN_ALL_TESTS()

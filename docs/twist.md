# Twist

Для тестирования всех домашних заданий используется фреймворк [_Twist_](https://gitlab.com/Lipovsky/twist)

Цель _Twist_ – повысить вероятность возникновения багов, которые возникают из-за конкуренции потоков, при исполнении стресс-тестов.

_Twist_ использует две техники:

- Fault injection: в исполнение стресс-тестов внедряются сбои (переключения или паузы потоков)
- Недетерминированные многопоточные исполнения моделируются с помощью кооперативных файберов, что позволяет детерминированно воспроизводить недетерминированные баги + повысить число итераций теста в единицу времени.

## Правила использования

Чтобы _Twist_ мог работать, пользователь должен соблюдать следующие правила:

### Примитивы

Вместо `std::thread`, `std::atomic`, `std::mutex`, `std::condition_variable` и других примитивов из [Thread support library](https://en.cppreference.com/w/cpp/thread) необходимо использовать одноименные примитивы из пространства имен `twist::stdlike`.

Примитивы из _Twist_ повторяют интерфейсы примитивов из стандартной библиотеки, так что вы можете пользоваться документацией на https://en.cppreference.com/w/ и не думать про _Twist_.

Заголовочные файлы меняются по следующему принципу: `#include <atomic>` заменяется на `#include <twist/stdlike/atomic.hpp>`

Доступные заголовки: https://gitlab.com/Lipovsky/twist/-/tree/master/twist/stdlike

При этом можно использовать `std::lock_guard` и `std::unique_lock` (но только в связке с `twist::stdlike::mutex`), это не примитивы синхронизации, а RAII для более безопасного использования мьютекса.

Использование примитивов из `std` приведет к неопределенному поведению в тестах, будьте внимательны!

### Yield

Вместо `std::this_thread::yield` необходимо использовать `twist::stdlike::this_thread::yield` из заголовочного файла `<twist/stdlike/thread.hpp>`.

А еще лучше использовать `twist::stdlike::SpinWait` из заголовочного файла `<twist/util/spin_wait.hpp>`.

### `thread_local`

Вместо `thread_local` нужно использовать `twist::util::ThreadLocalPtr<T>` из заголовочного файла `<twist/util/thread_local.hpp>`.

## Пример

Так будет выглядеть простейший спинлок, написанный с учетом _Twist_:

```cpp
// Вместо #include <atomic>
#include <twist/stdlike/atomic.hpp>

class SpinLock {
 public:
  void Lock() {
    // "Твистовый" атомик повторяет API атомика из std:
    while (locked_.exchange(true)) {
      ;
    }
  }
  
  void Unlock() {
    locked_.store(false);
  }
 private:
  // Вместо std::atomic<bool>
  twist::stdlike::atomic<bool> locked_{false};
};
```

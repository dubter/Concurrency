# Twist

Для тестирования задач курса используется фреймворк [_Twist_](https://gitlab.com/Lipovsky/twist).

Цель _Twist_ – спровоцировать в стресс-тестах баги, связанные с неаккуратной синхронизацией потоков.

Две главные техники, которые использует _Twist_:

- Fault injection: в исполнение стресс-тестов внедряются непредсказуемые переключения потоков
- Недетерминированные многопоточные исполнения моделируются с помощью кооперативных файберов, что позволяет детерминированно воспроизводить недетерминированные по своей природе баги + повысить число итераций теста в единицу времени.

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
    // "Твистовый" атомик повторяет API атомика из std, 
    // так что можно пользоваться стандартной документацией:
    // https://en.cppreference.com/w/cpp/atomic/atomic
    while (locked_.exchange(true)) {
      ;
    }
  }
  
  void Unlock() {
    locked_.store(false);  // <-- Здесь работает fault injection:
                           // любая операция над примитивом синхронизации может привести к
                           // переключению исполнения на другой поток
  }
 private:
  // Вместо std::atomic<bool>
  twist::stdlike::atomic<bool> locked_{false};
};
```

### Futex

В `twist` метод `wait` у `atomic` доступен только для `T` = `uint32_t`.

Кроме того, `atomic` в `twist` дополнительно имеет методы `FutexWait` / `FutexWake`.

```cpp
// https://eel.is/c++draft/atomics.types.generic#lib:atomic,wait
void wait(T old) {
  while (load() == old) {
    FutexWait(old);
  }   
}
```
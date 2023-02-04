# Twist

Для тестирования задач курса используется фреймворк [_Twist_](https://gitlab.com/Lipovsky/twist).

Цель _Twist_ – спровоцировать в стресс-тестах баги, связанные с неаккуратной синхронизацией потоков.

## Техники

Две главные техники, которые использует _Twist_:

- внедрение сбоев (_fault injection_) и
- детерминированная симуляция.

### Fault injection

В точках обращения потоков к примитивам синхронизации внедряются "сбои" (переключения контекста),
что позволяет увеличить покрытие графа состояний теста.

### Детерминированная симуляция

Недетерминированные многопоточные исполнения моделируются с помощью однопоточных кооперативных файберов, что позволяет
- Детерминированно воспроизводить недетерминированные по своей природе баги
- Рандомизировать очередь планировщика и очереди ожидания в мьютексах и кондварах 
- Вместить больше симуляций в ограниченный временной бюджет стресс-теста

## Навигация

Клиентская часть библиотеки – директория `twist/ed/`, пространство имен `twist::ed`

- `stdlike/` – замена стандартным примитивам синхронизации
- `lang/` – тред-локальные переменные
- `wait/` – механизмы ожидания
  - `spin.hpp` – активное ожидание для спинлоков
  - `sys.hpp` – блокирующее ожидание для мьютексов

Среда исполнения – директория `twist/rt/`, пространство имен `twist::rt`

## Правила использования

Чтобы _Twist_ мог работать, пользователь должен соблюдать следующие правила:

### Примитивы

Вместо `std::thread`, `std::atomic`, `std::mutex`, `std::condition_variable` и других примитивов из [Thread support library](https://en.cppreference.com/w/cpp/thread) необходимо использовать одноименные примитивы из пространства имен `twist::ed::stdlike`.

Примитивы из _Twist_ повторяют интерфейсы примитивов из стандартной библиотеки, так что вы можете пользоваться документацией на https://en.cppreference.com/w/ и не думать про _Twist_.

Заголовочные файлы меняются по следующему принципу: `#include <atomic>` заменяется на `#include <twist/ed/stdlike/atomic.hpp>`

Доступные заголовки: https://gitlab.com/Lipovsky/twist/-/tree/master/twist/ed/stdlike

При этом можно использовать `std::lock_guard` и `std::unique_lock` (но только в связке с `twist::ed::stdlike::mutex`), это не примитивы синхронизации, а RAII для более безопасного использования мьютекса.

Использование примитивов из `std` приведет к неопределенному поведению в тестах, будьте внимательны!

### Планировщик

Заголовочный файл: `twist/ed/stdlike/thread.hpp`

#### `yield`

Вместо `std::this_thread::yield` нужно использовать `twist::ed::stdlike::this_thread::yield`.

А еще лучше использовать `twist::ed::SpinWait` из заголовочного файла `<twist/ed/wait/spin.hpp>`.

#### `sleep_for`

Вместо `std::this_thread::sleep_for` нужно использовать `twist::ed::stdlike::this_thread::sleep_for`.

### Ожидание

#### `SpinWait`

Заголовочный файл: `<twist/ed/wait/spin.hpp>`

Предназначен для активного ожидания в спинлоках.

```cpp
SpinLock::Lock() {
  // Одноразовый!
  // Для каждого нового цикла ожидания в каждом потоке 
  // нужно заводить отдельный экземпляр SpinWait
  twist::ed::SpinWait spin_wait;
  while (locked_.exchange(1) == 1) {
    // У SpinWait определен operator()
    spin_wait();  // <- backoff
  }
}
```

#### `Wait`

Заголовочный файл: `twist/ed/wait/sys.hpp`

Для блокирующего ожидания вместо методов `wait` / `notify` у `std::atomic` нужно использовать
свободные функции `twist::ed::Wait` / `twist::ed::Wake{One,All}`

```cpp
class Event {
 public:
  void Wait() {
    twist::ed::Wait(&fired_, 0);
  }

  void Fire() {
    twist::ed::WakeAll(&fired_);
  }

 private:
  // Функции `Wait/Wake` требуют типа uint32_t
  twist::ed::stdlike::atomic<uint32_t> fired_{0};
};
```


### `thread_local`

Заголовочный файл: `<twist/ed/lang/thread_local.hpp>`

Вместо `thread_local` нужно использовать `twist::ed::ThreadLocalPtr<T>`:

```cpp
// ThreadLocalPtr<T> - замена thread_local T*
// - Для каждого потока хранит собственное значение указателя
// - Повторяет интерфейс указателя
// - Инициализируется nullptr-ом
twist::ed::ThreadLocalPtr<Fiber> this_fiber;
```

## Пример

Так будет выглядеть простейший спинлок, написанный с учетом _Twist_:

```cpp
// Вместо #include <atomic>
#include <twist/ed/stdlike/atomic.hpp>
#include <twist/ed/wait/spin.hpp>

class SpinLock {
 public:
  void Lock() {
    twist::ed::SpinWait spin_wait;
    while (locked_.exchange(true)) {
      spin_wait();
    }
  }
  
  void Unlock() {
    locked_.store(false);  // <-- Здесь работает fault injection:
                           // любая операция над примитивом синхронизации может привести к
                           // переключению исполнения на другой поток
  }
 private:
  // Вместо std::atomic<bool>
  // "Твистовый" атомик повторяет API атомика из std, 
  // так что можно пользоваться стандартной документацией:
  // https://en.cppreference.com/w/cpp/atomic/atomic
  
  twist::ed::stdlike::atomic<bool> locked_{false};
};
```

## Запуск кода

```cpp
#include <twist/ed/stdlike/thread.hpp>

#include <twist/rt/run.hpp>

int main() {
  twist::rt::Run([] {
    twist::ed::thread thread t([] {
      for (size_t i = 0; i < 7; ++i) {
        twist::ed::this_thread::yield();
      }
    });
  
    t.join();
  });
  
  return 0;
}
```
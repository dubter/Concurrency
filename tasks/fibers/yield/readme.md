# Fibers

## Пререквизиты

- [tasks/thread-pool](/tasks/tasks/thread-pool)
- [fibers/coro](/tasks/fibers/coro)

---

## От корутин к файберам

В этой задаче вы должны написать многопоточные файберы.

Планировщик для файберов вами [уже реализован](tasks/tasks/thread-pool), это пул потоков.

Механизм остановки / возобновления исполнения – [тоже](tasks/fibers/coro), это stackful корутина.

Остается лишь скомбинировать их!

## Файберы

Так будет выглядеть результат:

```cpp
void FibersExample() {
  // В этой задаче мы начнем писать свой фреймворк для concurrency
  // Он будет называться `exe` (от execution)
  using namespace exe;

  // Планировщиком для файберов будет служить пул потоков
  tp::ThreadPool scheduler{/*threads=*/4};
  scheduler.Start();

  for (size_t i = 0; i < 256; ++i) {
    // Запускаем файбер в пуле-планировщике
    fibers::Go(scheduler, [] {
      for (size_t j = 0; j < 3; ++j) {
        // Уступаем поток планировщика другому файберу
        fibers::Yield();
      }  
    });
  }
  
  // Файберы исполняются _параллельно_ на разных потоках пула-планировщика

  // Дожидаемся завершения файберов
  scheduler.WaitIdle();
  // Выключаем планировщик
  scheduler.Stop();
}  
```

## Декомпозиция

Реализация многопоточных файберов свелась к комбинированию двух ортогональных компонентов:

- Пул потоков
- Stackful корутина

Пул потоков ничего не знает про природу задач, которые он исполняет.

Корутины ничего не знают про пулы потоков и кооперативную многозадачность, это лишь механизм передачи управления.

### `Task`, `Coroutine`, `Fiber`

Чтобы лучше понять предложенную декомпозицию, подумайте как в реализации файберов будут распределены обязанности между сущностями
- _задача_ (`Task`), 
- _корутина_ (`Coroutine`) и 
- _файбер_ (`Fiber`).

### Фреймворк

Декомпозиция отражена в организации фреймворка `exe`, который мы начнем писать в этой задаче:
- [`coro`](exe/coro) – корутины 
- [`tp`](exe/tp) – пул потоков (планировщик)
- [`fibers`](exe/fibers) – файберы, зависят от `tp` и `coro`

## Задание

1) Перенесите в [exe/tp](exe/tp/thread_pool.hpp) реализацию пула потоков из задачи [tasks/thread-pool](/tasks/tasks/thread-pool)
2) Перенесите в [exe/coro](exe/coro/) реализацию корутины из задачи [fibers/coro](/tasks/fibers/coro) 
3) Через `ThreadPool` и `Coroutine` выразите [файберы](exe/fibers/)

## Замечания по реализации

### Время жизни

Аллоцируйте файберы на куче.

Не используйте умные указатели для контроля времени жизни файбера. Такой подход не будет работать
с очередями ожидания в примитивах синхронизации, которые появятся в будущих задачах.

[Is it legal (and moral) for a member function to say `delete this`?](https://isocpp.org/wiki/faq/freestore-mgmt#delete-this)

### `Submit`

Будем считать, что все пользователи, запускающие в пуле потоков лямбды, пользуются не методом 
 `Submit`, а свободной функцией `tp::Submit`:

```cpp
void SubmitExample() {
  using namespace exe;
  
  tp::ThreadPool pool{4};
  pool.Start();
  
  tp::Submit(pool, [] {
    std::println("Hello");
  });
  
  pool.WaitIdle();
  pool.Stop();
}
```

В тестах задачи лямбды в пул потоков планируются _только_ с помощью `tp::Submit`.

Так что сигнатуру метода `ThreadPool::Submit` можно менять. Благодаря этому в файберах можно избавиться от динамических аллокаций при планировании.

### `ThreadPool::Current`

Не используйте `ThreadPool::Current`, вместо этого храните указатель на пул потоков прямо в полях файбера.

В будущем мы обобщим пул потоков до абстрактного планировщика, и тогда аналогичный метод написать / использовать уже не получится.

### Std-like

По желанию вы можете сохранить в своей реализации самодельные мьютекс и кондвар.

### Misc

Не используйте `ThreadLocalPtr` в `CoroutineImpl`.

Класс `Fiber` не должен быть виден пользователю через публичное API.

## References

- [Project Loom: Fibers and Continuations for the Java Virtual Machine](https://cr.openjdk.java.net/~rpressler/loom/Loom-Proposal.html), [State of Loom](https://cr.openjdk.java.net/~rpressler/loom/loom/sol1_part1.html)
- [Fibers, Oh My!](https://graphitemaster.github.io/fibers/)  
- [Ron Pressler and Alan Bateman – Project Loom](https://www.youtube.com/watch?v=J31o0ZMQEnI)

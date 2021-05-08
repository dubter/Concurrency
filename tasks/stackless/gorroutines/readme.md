# Gor-routines

Однажды с помощью stackful корутин мы реализовали кооперативную многозадачность в виде файберов (легковесных потоков).

Теперь мы сделаем то же самое для stackless корутин из С++20. 

То, что у нас получится, мы (не всерьез) назовем _гор-рутинами_ (_gor-routines_) в честь одного из авторов Coroutines TS в С++ – [Гора Нишанова](https://www.youtube.com/watch?v=_fu0gx-xseY). Не путайте их с настоящими горутинами (goroutines) из языка Go!

## Gor-routines in Action

```cpp
gorr::StaticThreadPool scheduler{/*threads=*/4};
gorr::Mutex mutex;

gorr::JoinHandle GorRoutine() {
  // Перепланируемся в пул потоков
  co_await scheduler.Schedule();

  for (size_t j = 0; j < 100; ++j) {
    if (j % 7 == 0) {
      // Уступаем поток пула другой горрутине
      co_await gorr::Yield();
    }

    {
      auto guard = co_await mutex.Lock();
      // Мы в критической секции!
    }
  }
  
  co_return;
}

int main() {
  for (size_t i = 0; i < 100; ++i) {
    // Запускаем горрутину
    GorRoutine();
  }
  
  // Дожидаемся завершения всех горрутин
  scheduler.Join();
  
  return 0;
}
```

## Runtime

Асинхронное программирование – это одно из самых очевидных и полезных применений stackless корутин в С++.

Но в стандартной библиотеке никакой поддержки для асинхронного программирования на корутинах нет. Корутины в С++20 – это исключительно language feature.

Так что нам придется написать написать все самим!

### Планировщик

В отличие от обычного пула потоков, [`gorr::StaticThreadPool`](gorr/runtime/thread_pool.hpp) планирует не задачи, а корутины: 

```cpp
// Телепортируемся в пул потоков
co_await pool.Schedule();
```

К пулу (планировщику) прилагается функция `gorr::Yield`:

```cpp
// Перепланировать текущую горрутину
// в конец очереди планировщика
co_await gorr::Yield();
```

#### Реализация

За основу возьмите уже написанный ранее пул потоков.

Пул не должен выполнять динамические аллокации при планировании корутин! 

Используйте интрузивную очередь, узлы для нее аллоцируйте на фреймах корутин (через awaiter-ые, которые пул возвращает в вызове `Schedule`).

### `Mutex`

Операция `Lock` у [`gorr::Mutex`](gorr/runtime/mutex.hpp) – асинхронная:

```cpp
{
  auto guard = co_await mutex.Lock();
  // Критическая секция
}
```

#### Реализация

Реализация мьютекса не должна выполнять динамические аллокации.

В реализации мьютекса вы больше не сможете написать цикл `while`.

Вам потребуется вариант `await_suspend`, который возвращает `bool`, а не `void`:
- `false` означает, что корутина передумала останавливаться, она сразу выполнит `await_resume` и продолжит исполнение
- `true` – что awaiter запланировал возобновление корутины, и она должна остановиться

В методе `await_ready` awaiter-а мьютекса пробуйте захватить мьютекс без ожидания. 

В качестве задачи под звездочкой подумайте над реализацией, которая не будет использовать взаимное исключение на уровне _потоков_ (т.е. в ней не будет спинлока).

### `JoinHandle`

Горрутина при запуске дает пользователю [`gorr::JoinHandle`](gorr/runtime/join_handle.hpp), с помощью которого тот может дождаться завершения горрутины:   

```cpp

gorr::StaticThreadPool scheduler{4};

gorr::JoinHandle GorRoutine() {
  co_await scheduler.Schedule();
  
  // Тут горрутина делает что-то полезное
  co_return;
}

int main() {
  // Стартуем горрутину
  auto h = GorRoutine();

  // Синхронно дожидаемся завершения горрутины в пуле
  gorr::Join(h);

  // ...
}
```

#### Реализация 

В задаче дана заглушка, [опционально] можно заменить ее на решение задачи `Task`.

## Задача

Реализуйте

1) [`StaticThreadPool`](gorr/runtime/thread_pool.hpp)
2) [`Yield`](gorr/runtime/yield.hpp)
3) [`Mutex`](gorr/runtime/mutex.hpp)
4) [Опционально] [`JoinHandle`](gorr/runtime/join_handle.hpp) и метод [`Join`](gorr/runtime/join.hpp).


## References

- [Asymmetric Transfer](https://lewissbaker.github.io/)
- [cppreference / Coroutines](https://en.cppreference.com/w/cpp/language/coroutines)
- [dcl.fct.def.coroutine](https://eel.is/c++draft/dcl.fct.def.coroutine), [expr.await](https://eel.is/c++draft/expr.await#:co_await)

# Gor-routines

Однажды с помощью _stackful_ корутин мы реализовали кооперативную многозадачность в виде файберов (легковесных потоков).

В этой задаче мы сделаем то же самое, но на этот раз с помощью _stackless_ [корутин С++20](https://en.cppreference.com/w/cpp/language/coroutines).

То, что у нас получится, мы (не всерьез) назовем _гор-рутинами_ (_gor-routines_) в честь одного из авторов Coroutines TS в С++ – [Гора Нишанова](https://www.youtube.com/watch?v=_fu0gx-xseY). Не путайте их с [настоящими горутинами](https://gobyexample.com/goroutines) (goroutines) из языка Go!

## Gor-routines in Action

```cpp
gorr::StaticThreadPool scheduler{/*threads=*/4};
gorr::Mutex mutex;

gorr::JoinHandle GorRoutine() {
  // Перепланируемся в пул потоков
  co_await scheduler.Schedule();

  // Теперь мы исполняемся в потоке пула `scheduler`

  for (size_t j = 0; j < 128; ++j) {
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
    gorr::Detach(GorRoutine());
  }
  
  // Дожидаемся завершения всех горрутин
  scheduler.Join();
  
  return 0;
}
```

## Runtime

Асинхронное программирование – одно из самых очевидных и полезных применений stackless корутин в С++.

Но в стандартной библиотеке никакой поддержки для асинхронного программирования на корутинах нет. Корутины в С++20 – это исключительно language feature.

Так что нам придется написать все самим!

### Планировщик

В отличие от обычного пула потоков, [`gorr::StaticThreadPool`](gorr/runtime/thread_pool.hpp) планирует не задачи, а корутины: 

```cpp
// Телепортируемся в пул потоков
co_await pool.Schedule();
```

К пулу (планировщику) прилагается функция `gorr::Yield`:

```cpp
// Перепланируем текущую горрутину
// в конец очереди планировщика
co_await gorr::Yield();
```

#### Реализация

За основу возьмите уже написанный ранее пул потоков.

Пул не должен выполнять динамических аллокаций памяти при планировании. Используйте интрузивную очередь, узлы для нее аллоцируйте на фреймах корутин (через awaiter-ы, которые пул возвращает в вызове `Schedule`).

### `Mutex`

Операция `Lock` у [`gorr::Mutex`](gorr/runtime/mutex.hpp) – асинхронная:

```cpp
{
  auto guard = co_await mutex.Lock();
  // Критическая секция
}
```

#### Реализация

Мьютекс не должен выполнять динамических аллокаций памяти.

В реализации мьютекса вы больше не сможете написать цикл `while`.

Вам потребуется вариант `await_suspend`, который возвращает `bool`, а не `void`:
- `false` означает, что корутина передумала останавливаться, она сразу выполнит `await_resume` и продолжит исполнение
- `true` – awaiter запланировал возобновление корутины, сейчас корутина должна остановиться

В методе `await_ready` awaiter-а мьютекса пробуйте захватить мьютекс без ожидания.

Для повышения пропускной способности исполняйте критические секции сериями. 

Задача со звездочкой: придумайте реализацию, которая не будет использовать взаимное исключение на уровне _потоков_ (т.е. без спинлоков).

### `JoinHandle`

Горрутина при запуске дает пользователю [`gorr::JoinHandle`](gorr/runtime/join_handle.hpp), с помощью которого тот может дождаться ее завершения:   

```cpp
gorr::StaticThreadPool scheduler{/*threads=*/4};

gorr::JoinHandle GorRoutine() {
  co_await scheduler.Schedule();
  
  // Тут горрутина делает что-то полезное
  co_return;
}

int main() {
  // Стартуем горрутину
  auto h = GorRoutine();

  // Синхронно дожидаемся завершения горрутины в пуле
  gorr::Join(std::move(h));
  // Альтернативный вариант - отвязываемся от
  // запущенной горрутины с помощью
  // gorr::Detach(std::move(h))

  // ...
}
```

#### Реализация 

В задаче дана заглушка, [по желанию] можно интегрировать вместо нее решение задачи [stackless/task](/tasks/stackless/task).

## Задача

Реализуйте

1) [`StaticThreadPool`](gorr/runtime/thread_pool.hpp)
2) [`Yield`](gorr/runtime/yield.hpp)
3) [`Mutex`](gorr/runtime/mutex.hpp)
4) [Опционально] [`JoinHandle`](gorr/runtime/join_handle.hpp) и функцию [`Join`](gorr/runtime/join.hpp)


## References

- [Asymmetric Transfer](https://lewissbaker.github.io/)
- [cppreference / Coroutines](https://en.cppreference.com/w/cpp/language/coroutines)
- [dcl.fct.def.coroutine](https://eel.is/c++draft/dcl.fct.def.coroutine), [expr.await](https://eel.is/c++draft/expr.await#:co_await)

# `Task` (ленивая `Future`)

Рассмотрим пример:

```cpp
std::future<int> Coro(StaticThreadPool& pool) {
  // Перепланируемся в пул потоков
  co_await pool;
  // Асинхронно вычисляем ответ
  co_return 42;
}

// Похоже на `AsyncVia`?
std::future<int> f = Coro();
std::cout << "Async value = " << f.get() << std::endl;
```

В точке вызова (точнее, старта) корутина `Coro` не может сразу вернуть значение типа `int` (оно будет вычислено асинхронно в пуле потоков), поэтому вызов `Coro()` возвращает представление этого будущего значения – `std::future<int>`:

```cpp
// Примерно такой код сгенерирует компилятор:
auto Coro(StaticThreadPool& pool) {
  // Аллоцируем coroutine state
  // Он скрыт от пользователя корутин
  auto* coro = new CoroutineState{pool};
  // С помощью promise type 
  // (который является полем coroutine state-а) 
  // строим мгновенный результат - std::future<int>
  auto ret_object = coro->GetReturnObject();
  // Делаем первый шаг корутины
  coro->Start();
  return ret_object;
}
```

См. [Hand-crafted stackless coroutine](https://gitlab.com/Lipovsky/stackless_examples/-/blob/master/state_machine_4/main.cpp)

Но фьюча приносит с собой дополнительные накладные расходы:
- Динамическая аллокация shared state
- Атомарный подсчет ссылок на shared state
- Синхронизация на shared state из-за гонки между продьюсером и консьюмером

Наша задача – избежать этих накладных расходов, заменив в корутинах фьючу на специально изготовленный `Task<T>`: [Add coroutine task type](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1056r0.html)

## `Task<T>`

Вместо фьючи корутина будет возвращать `Task<T>`:

```cpp
// Task<int> – представление будущего результата
Task<int> Coro(StaticThreadPool& pool) {
// Перепланируемся в пул потоков
  co_await pool;
  // Теперь мы исполняемся в потоке пула
  // Асинхронно (относительно caller-а корутины)
  // вычисляем ответ
  co_return 42;
}
```

При вызове `Coro()` корутина должна остановиться _перед_ исполнением пользовательского кода, в служебной точке `co_await promise.initial_suspend()`:

```cpp
Task<int> task = Coro();
// <- В этой точке вычисление еще не запланировано
// в пул потоков
```

После вызова корутины уже аллоцирован coroutine state, caller получил представление будущего результата в виде объекта `Task`, но сама асинхронная операция еще не стартовала.

### Распаковка

Асинхронная операция запускается caller-ом корутины явно в точке "распаковки" `Task`. Сделать это можно двумя способами:

#### 1. `co_await`

Первый способ "распаковать" `Task` – самому стать корутиной:
```cpp
// 1) Подписываемся на возобновление себя,
// 2) стартуем корутину, которая построила `task`,
// 3) останавливаемся.
int value = co_await task;
```

#### 2. `Await`

Альтернативный способ – провести границу между [красными и синими вызовами](https://journal.stuffwithstuff.com/2015/02/01/what-color-is-your-function/) с помощью блокирующего вызова `Await` и тем самым остановить синтаксическое заражение:
```cpp
// Функция main не может быть корутиной,
// использовать co_await в ней нельзя.
int main() {
  Task<int> task = Coro();
  // Стартуем корутину и блокируем поток до ее завершения.
  int value = Await(std::move(task));
  std::cout << "Value = " << value << std::endl;
  return 0;
}
```

## Profit

### Аллокация

Нам не нужна отдельная динамическая аллокация для shared state, у нас уже есть аллоцированный на куче coroutine state (причем компилятор может стереть эту аллокацию, если будет уверен, что она не нужна).

### Подсчет ссылок

Нам не нужен подсчет ссылок для контроля времени жизни shared state / coroutine state: `co_await task` означает, что время жизни корутины-caller-а (которая играет роль консьюмера) покрывает время жизни корутины-callee (которая является продьюсером).

### Синхронизация

Старт асинхронной операции отложенный, им управляет caller, а значит он может подписать продолжение (continuation) на этот результат без гонки, т.е. без синхронизации.

## Задача

1) Реализуйте [`JoinHandle`](task/join_handle.hpp) – синоним для `Task<void>`
2) Реализуйте [`Task<T>`](task/task.hpp)
3) Поддержите `Task<void>` и выразите `JoinHandle` через него.

### Требования

- Реализация не должна явно аллоцировать память на куче (помимо скрытых аллокаций coroutine state-а, которые выполняет сам компилятор)
- [Опционально] Реализация не должна требовать синхронизации caller-а и callee

## Замечания по реализации

Вам потребуется реализовать:

- `Task<T>` – представление будущего результата
- promise type для корутины, которая возвращает `Task<T>`
- awaiter для `co_await task`

Используйте готовый awaiter [`suspend_always`](https://en.cppreference.com/w/cpp/coroutine/suspend_always) для `initial_suspend`.

Получить [`coroutine_handle`](https://en.cppreference.com/w/cpp/coroutine/coroutine_handle) для текущей корутины из `promise_type` можно с помощью метода [`from_promise`](https://en.cppreference.com/w/cpp/coroutine/coroutine_handle/from_promise)

Используйте `wheels::Result<T>` для передачи значения / исключения между caller / callee.

`Task` / `JoinHandle` не должны зависеть от пула потоков. Использование `Task` не требует исполнения корутины в пуле.

## References

- [Asymmetric Transfer](https://lewissbaker.github.io/)
- [cppreference / Coroutines](https://en.cppreference.com/w/cpp/language/coroutines)
- [dcl.fct.def.coroutine](https://eel.is/c++draft/dcl.fct.def.coroutine), [expr.await](https://eel.is/c++draft/expr.await#:co_await)

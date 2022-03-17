# TinyEcho

Пока _TinyFibers_ умеют лишь запускаться, перепланироваться и синхронизироваться. В этой задаче мы научим их взаимодействовать с внешним миром.

Для этого мы воспользуемся библиотекой [`asio`](https://think-async.com/Asio/asio-1.22.1/doc/asio/overview/rationale.html), которая предоставляет event loop и асинхронные интерфейсы для работы с таймерами и сетью.

## Задание

1) Реализуйте функцию [`SleepFor`](tinyfibers/core/api.hpp) для файберов
2) Реализуйте [сокеты](tinyfibers/net/socket.hpp) для файберов
3) Реализуйте [эхо-сервер](echo/server.cpp)

## [А]синхронность и поток управления

### Синхронный I/O

Самый простой способ писать сетевой код – пользоваться _синхронным_ I/O.

Пример: [синхронный эхо-сервер](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp14/echo/blocking_tcp_echo_server.cpp).

В этом примере вызов `sock.read_some(asio::buffer(data), error)` блокирует вызывающий поток до момента завершения чтения.

Синхронный I/O требует блокировки потока операционной системы, так что для конкурентного обслуживания клиентов на каждого из них придется завести отдельный поток.

Такое решение не масштабируется – клиентов может быть гораздо больше, чем число потоков, которые может эффективно обслуживать операционная система.

### Асинхронный I/O

Альтернативный подход – использовать цикл событий (_event loop_) и _асинхронный_ I/O.

Пример: [асинхронный эхо-сервер](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp14/echo/async_tcp_echo_server.cpp).

При вызове на сокете асинхронной операции `async_read_some` мы не блокируемся до момента появления данных, а _подписываемся_ на результат чтения – передаем в вызов _обработчик_ (_handler_) или _коллбэк_ (_callback_), после чего вызов `async_read_some` завершается _без ожидания_.

Коллбэк (обработчик) будет вызван на какой-то будущей итерации цикла событий, когда в `epoll` соответствующий сокет станет доступным для чтения, и из него будут вычитаны данные.

В асинхронном решении для каждого клиентского соединения порождается цепочка неблокирующихся задач. Все эти цепочки "упаковываются" в один поток операционной системы.

[Диаграмма](https://disk.yandex.ru/d/LEfNBVCtfo5N2A)

Правда при этом поток управления (control flow) выворачивается наизнанку: теперь он следует не вашей логике (как в случае с потоками), а подчиняется циклу запуска коллбэков внутри `io_context`-а, фактически – циклу `epoll`-а.

Ваш код разрывается в точках выполнения I/O на фрагменты-коллбэки, вы не можете писать циклы, использовать исключения, снимать стектрейсы. Такую цену вы платите за масштабируемость сетевого кода.

### Файберы

Файберы разрешают дилемму между простотой синхронного кода и масштабируемостью асинхронного, они сочетают преимущества _обоих_ подходов.

С помощью механизма переключения контекста можно склеить точки старта и завершения асинхронной операции и дать пользователю файберов видимость синхронного вызова. При этом под капотом будет крутиться тот же цикл событий с коллбэками.

## Трансформация async → sync

Файбер стартует асинхронную операцию (например, чтение из сокета), планирует свое возобновление в коллбеке, после чего уступает поток планировщика другому файберу.

## Кооперативность и I/O

Вспомним о кооперативной природе файберов – они могут уступать поток планировщика только добровольно.

I/O – естественные точки для кооперативного переключения.

---

## Обработка ошибок

Сокеты – низкий уровень абстракции, и ошибки на этом уровне не исключительны, а наоборот, ожидаемы.

API сокетов построено на классах [`Result<T>`](https://gitlab.com/Lipovsky/wheels/-/blob/master/wheels/result/result.hpp) и `Status` (синоним для `Result<void>`).

Экземпляр `Result<T>` гарантированно содержит _либо_ значение типа `T`, _либо_ код ошибки.
Пустой `Result` сконструировать невозможно.

`Result` не навязывает конкретный способ обработки ошибок, вы можете использовать как коды ошибок, так и исключения.

### `[[nodiscard]]`

Главное правило работы с ошибками – ошибки нельзя игнорировать!

Тип `Result` аннотирован как [`[[nodiscard]]`](https://en.cppreference.com/w/cpp/language/attributes/nodiscard). Если вы проигнорируете проверку `Result`-а, который получили из вызова метода или функции, то компилятор сгенерирует предупреждение. А с флагом компиляции `-Werror` это предупреждение превратится в ошибку.

### Примеры использования

#### Исключения

```cpp
// Здесь срабатывает неявная конвертация из `Result<Socket> &&`
// с проверкой `ThrowIfError`
Socket client_socket = acceptor.Accept();

// Теперь распакуем `Result` явно с помощью `ValueOrThrow`
size_t bytes_read = client_socket.Read(asio::buffer(read_buf, kBufSize)).ValueOrThrow();

// Write возвращает `Status` - синоним `Result<void>`
// Проверка результата и выбрасывание исключения
// в случае ошибки происходит в вызове ThrowIfError()
// Проигнорировать проверку нельзя: `Result` аннотирован как nodiscard
client_socket.Write(asio::buffer(read_buf, bytes_read)).ThrowIfError();

```

#### Коды ошибок

```cpp
// Здесь за auto скрывается `Result<Socket>`
auto client_socket = acceptor.Accept();

// Вместо `IsOk` можно использовать противоположный
// по смыслу метод `HasError`
if (!client_socket.IsOk()) {
  // Пробросим ошибку выше
  return PropagateError(client_socket);
}

// Теперь мы уверены, что ошибки нет
// Операторы -> и * не выполняют проверок!
// (аналогично std::optional)
auto bytes_read = client_socket->ReadSome(asio::buffer(read_buf, kBufSize));
if (bytes_read.HasError()) {
  return PropagateError(bytes_read);
}
 
// Метод `Write` возвращает `Status`, он же `Result<void>`
Status ok = client_socket->Write(asio::buffer(read_buf, *bytes_read));
```

#### Конструкторы

Для конструирования результатов используйте свободные функции из пространства имен `make_result`, они возьмут на себя вывод шаблонного типа:

##### `Ok` / `Fail`
```cpp
Result<size_t> Socket::ReadSome(MutableBuffer buffer) {
  // Выполняем чтение
  // ...
    
  // Здесь error – std::error_code, полученный от asio
  if (error) {
    // Немного шаблонной магии,
    // не требуется явно указывать шаблонный тип Result-а
    return Fail(error);
  }
  
  return Ok(bytes_read);
}
```

##### `ToStatus`

```cpp
Status Socket::ShutdownWrite() {
  // ...

  // Здесь error - std::error_code, полученный от shutdown из asio
  return ToStatus(error);
}
```

###### `PropagateError`

```cpp
auto result = socket.ReadSome(buffer);
if (result.HasError()) {
  // Текущая функция может возвращать произвольный `Result<U>`,
  // не обязательно тот же `Result<size_t>`, что и `ReadSome`
  return PropagateError(result);
}
```

### Подходы к обработке ошибок

- [Joe Duffy's Blog – The Error Model](http://joeduffyblog.com/2016/02/07/the-error-model/)
- Go: [Error handling and Go](https://blog.golang.org/error-handling-and-go), [Error Handling — Problem Overview](https://go.googlesource.com/proposal/+/master/design/go2draft-error-handling-overview.md), [Error Handling — Draft Design](https://go.googlesource.com/proposal/+/master/design/go2draft-error-handling.md)
- Rust: [Recoverable Errors with `Result`](https://doc.rust-lang.org/book/ch09-02-recoverable-errors-with-result.html)
- C++: [Boost.Outcome](https://www.boost.org/doc/libs/1_72_0/libs/outcome/doc/html/index.html), [`ErrorOr<T>`](https://github.com/llvm-mirror/llvm/blob/master/include/llvm/Support/ErrorOr.h) в LLVM, [`Try<T>`](https://github.com/facebook/folly/blob/master/folly/Try.h) в Folly, [`expected`](https://github.com/TartanLlama/expected)

---

## `SleepFor`

Интеграцию файберов и `asio` мы начнем со вспомогательной задачи: реализации функции `SleepFor`.

Текущая реализация `SleepFor` выглядит так:

```cpp
void Scheduler::SleepFor(Duration delay) {
  StopWatch stop_watch;
  do {
    Yield();
  } while (stop_watch.Elapsed() < delay);
}
```

Как видно, файбер в вызове `SleepFor` вовсе не спит, а постоянно перепланируется, вхолостую растрачивая процессорное время.

Вы должны реализовать `SleepFor` эффективнее: спящий файбер не должен постоянно получать управление и перепланироваться, планировщик не должен тратить на него процессорное время.

Если в планировщике нет файберов, которые готовы исполняться, но при этом есть спящие файберы, то планировщик должен блокировать _поток_, в котором он (планировщик) запущен, до пробуждения первого файбера.

### Эхо-сервер

После реализации `SleepFor` и сокетов напишите [эхо-сервер](echo/server.cpp).

Он должен
- выглядеть так же просто, как и [многопоточная реализация](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp14/echo/blocking_tcp_echo_server.cpp),
- при этом исполняться так же эффективно, как и [асинхронная однопоточная реализация](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp14/echo/async_tcp_echo_server.cpp).


## Указания по реализации

### Циклы

Для использования `asio` в файберах вам потребуется сочетать итерации двух циклов:

1) Цикл запуска файберов, который находится в методе `RunLoop` планировщика
2) Цикл обработки событий / запуска обработчиков `asio`: `io_context::run`

Каждый из этих циклов разбирает собственную очередь (задач и файберов соответственно).

Подумайте, как совместить два этих цикла. Реализация метода `RunLoop` должна упроститься.

Обратите внимания на функцию `asio::post`:

```cpp
// Планируем обработчик на исполнение
asio::post(io_context, []() {
  // Будет исполнен на очередной итерации io_context.run();
});
```

### Таймеры

Прочтите tutorial по таймерам:  [Using a timer asynchronously](http://think-async.com/Asio/asio-1.22.1/doc/asio/tutorial/tuttimer2.html).

Используйте специализацию `WaitableTimer`, определенную в [timer.hpp](tinyfibers/core/timer.hpp).

Запрещается:
- использовать функции [std::this_thread::sleep_for](https://en.cppreference.com/w/cpp/thread/sleep_for) и [sleep](http://man7.org/linux/man-pages/man3/sleep.3.html), т.е. явно ставить поток на паузу,
- писать свои собственные очереди спящих потоков.

### Сокеты

Используйте [`asio::ip::tcp::socket`](http://think-async.com/Asio/asio-1.22.1/doc/asio/reference/ip__tcp/socket.html) и [`asio::ip::tcp::acceptor`](http://think-async.com/Asio/asio-1.22.1/doc/asio/reference/ip__tcp/acceptor.html) в реализациях `Socket` и `Acceptor`.

Методы `ReadSome` и `Write` у сокета реализуйте через [`async_read_some`](http://think-async.com/Asio/asio-1.22.1/doc/asio/reference/basic_stream_socket/async_read_some.html) и [`async_write`](http://think-async.com/Asio/asio-1.22.1/doc/asio/reference/async_write/overload1.html) соответственно.

Метод `Read` реализуйте через `ReadSome`.

Не делайте публичных конструкторов у `Socket`. Пользователи конструируют сокеты только с помощью
- статических конструкторов `ConnectTo` / `ConnectToLocal`
- `acceptor.Accept()`

Метод `BindToAvailablePort` у `Acceptor` реализуйте через `BindTo(/*port=*/0)`, операционная система сама подберет свободный порт.

Заголовочные файлы `socket.hpp` и `acceptor.hpp` – часть публичного API файберов, в нем не должно быть зависимостей от планировщика и т.п.

#### Установка соединения

Сначала вам потребуется транслировать `host` в IP-адреса, для этого используйте класс [`asio::ip::tcp::resolver`](http://think-async.com/Asio/asio-1.22.1/doc/asio/reference/ip__tcp/resolver.html).

Затем нужно пройти по всем `endpoint`-ам и попробовать установить соединение с каждым из них.

Рекомендуем реализовать в сокете вспомогательный статический метод `Connect`, который получает [`asio::ip::tcp::endpoint`](http://think-async.com/Asio/asio-1.22.1/doc/asio/reference/ip__tcp/endpoint.html).

Изучите пример [async_tcp_client](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp11/timeouts/async_tcp_client.cpp).

#### Неблокирующие операции (бонусный уровень)

В реализации `ReadSome` попробуйте сначала оптимистично читать данные из сокета в неблокирующем режиме, и только в случае неудачи стартовать асинхронную операцию `async_read_some`.

#### Ошибки

Некоторые асинхронные операции в `asio` могут вернуть _и_ ошибку, _и_ число записанных / прочитанных байт. 

В таком случае можно считать, что операция завершилась ошибкой, и число прочитанных / записанных байт нас не интересует. Никаких разумных гарантий мы в таком случае все равно не имеем.

### `FutureLite`

Скорее всего в вашей реализации сокетов будет много повторяющегося кода:

При запуске каждой асинхронной операции нужно
1) Остановить текущий файбер и запланировать его возобновление
2) Прокинуть асинхронный результат из коллбэка в файбер

Инкапсулируйте эту логику в отдельном классе – [`FutureLite<T>`](tinyfibers/sync/future.hpp).

Результат асинхронной операции (значение или ошибку) передавайте из коллбэка в файбер в виде `Result<T>`.

## Материалы

### Asio

* [Github](https://github.com/chriskohlhoff/asio/)
* [Basic Anatomy](https://think-async.com/Asio/asio-1.22.1/doc/asio/overview/basics.html)
* [Overview](https://think-async.com/Asio/asio-1.22.1/doc/asio/overview.html)
* [Tutorial](https://think-async.com/Asio/asio-1.22.1/doc/asio/tutorial.html)
* [Thinking Asynchronously: Designing Applications with Boost.Asio](https://www.youtube.com/watch?v=D-lTwGJRx0o), [слайды](http://cpp.mimuw.edu.pl/files/boost_vs_qt/asio/thinking_asynchronously.pdf)


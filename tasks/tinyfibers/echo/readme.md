# Echo

1) Реализуйте [сокеты](tinyfibers/net/socket.hpp) для файберов.
2) Напишите с их помощью [синхронную версию](echo/server.cpp) эхо-сервера.

## Эхо-сервер

Ваш эхо-сервер на файберах должен

- выглядеть так же просто, как и [многопоточная реализация](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp14/echo/blocking_tcp_echo_server.cpp),
- при этом исполняться так же эффективно, как и [асинхронная однопоточная реализация](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp14/echo/async_tcp_echo_server.cpp).


## Пререквизиты

1) Задача [Echo](/tasks/asio/echo)
2) Задача [SleepFor](/tasks/tinyfibers/sleep)

## [А]синхронность и поток управления

### Потоки

Самый простой способ писать сетевой код – пользоваться [синхронным I/O](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp14/echo/blocking_tcp_echo_server.cpp).

Но синхронный I/O требует блокировки потока операционной системы, так что для конкурентного обслуживания клиентов на каждого из них придется завести отдельный поток.

Такое решение не масштабируется – клиентов может быть гораздо больше, чем число потоков, которые может эффективно обслуживать операционная система.

### Коллбэки и цикл событий

Альтернативный подход – использовать цикл событий (_event loop_) и асинхронные операции, что позволит упаковать всю работу в виде цепочек коллбэков в один поток операционной системы.

Правда при этом поток управления (_control flow_) выворачивается наизнанку: теперь он следует не вашей логике (как в случае с потоками), а подчиняется циклу запуска коллбэков внутри `io_context`-а, фактически – циклу `epoll_wait`-у.

Ваш код разрывается в точках выполнения I/O на фрагменты-коллбэки, вы не можете писать циклы, использовать исключения, снимать стеки. Такую цену вы платите за масштабируемость вашего сетевого кода.

### Файберы

Файберы разрешают дилемму между простотой синхронного кода и масштабируемостью асинхронного, они сочетают преимущества _обоих_ подходов.

С помощью механизма переключения контекста можно склеить точки старта и завершения асинхронной операции и дать пользователю файберов видимость синхронного вызова. При этом под капотом будет крутиться тот же цикл событий с коллбэками.

## Трансформация async → sync

Файбер стартует асинхронную операцию (например, чтение из сокета), планирует свое возобновление в коллбеке, после чего уступает поток планировщика другому файберу.

## Кооперативность и I/O

Вспомним о кооперативной природе файберов – они могут уступать поток планировщика только добровольно.

Операции сетевого I/O – естественные точки для кооперативного переключения.

---

## Обработка ошибок

Сокеты – низкий уровень абстракции, и ошибки на этом уровне не исключительны, а наоборот, ожидаемы.

API сокетов построено на классах [`Result<T>`](https://gitlab.com/Lipovsky/wheels/-/blob/master/wheels/support/result.hpp) и `Status` (синоним для `Result<void>`).

Экземпляр `Result` гарантированно содержит _либо_ значение типа `T`, _либо_ код ошибки.

`Result` не навязывает конкретный способ обработки ошибок, вы можете использовать как обработку кодов, так и исключения.

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
// в случае ошибки происходит в вызове ExpectOk()
client_socket.Write(asio::buffer(read_buf, bytes_read)).ExpectOk();

```

#### Коды ошибок

```cpp
// Здесь за auto прячется `Result<Socket>`
auto client_socket = acceptor.Accept();

// Вместо `IsOk` можно использовать противоположный
// по смыслу метод `HasError`
if (!client_socket.IsOk()) {
  // Пробросим ошибку выше
  return PropagateError(client_socket);
}

// Теперь мы уверены, что ошибки нет
// Операторы -> и * не выполняют проверок!
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

## Заметки по реализации

### Планировщик + Asio

Вам потребуется интеграция планировщика файберов и цикла событий из Asio, эта часть уже должна быть готова после решения [SleepFor](/tasks/tinyfibers/sleep). Если вы написали там хороший код, то в планировщик изменений вносить не потребуется.

### Сокеты

Используйте [`asio::ip::tcp::socket`](http://think-async.com/Asio/asio-1.18.1/doc/asio/reference/ip__tcp/socket.html) и [`asio::ip::tcp::acceptor`](http://think-async.com/Asio/asio-1.18.1/doc/asio/reference/ip__tcp/acceptor.html) в реализациях `Socket` и `Acceptor`.

Методы `ReadSome` и `Write` у сокета реализуйте через [`async_read_some`](http://think-async.com/Asio/asio-1.18.1/doc/asio/reference/basic_stream_socket/async_read_some.html) и [`async_write`](http://think-async.com/Asio/asio-1.18.1/doc/asio/reference/async_write/overload1.html) соответственно.

Метод `Read` реализуйте через `ReadSome`.

Не делайте публичных конструкторов у `Socket`. Пользователи конструируют сокеты только с помощью
- статических конструкторов `ConnectTo` / `ConnectToLocal`
- `acceptor.Accept()`

Метод `BindToAvailablePort` у `Acceptor` реализуйте через `BindTo(/*port=*/0)`, операционная система сама подберет свободный порт.

Заголовочные файлы `socket.hpp` и `acceptor.hpp` – часть публичного API файберов, в нем не должно быть зависимостей от планировщика и т.п.

### Установка соединения

Сначала вам потребуется транслировать `host` в IP-адреса, для этого используйте класс [`asio::ip::tcp::resolver`](http://think-async.com/Asio/asio-1.18.1/doc/asio/reference/ip__tcp/resolver.html).

Затем нужно проитерироваться по всем `endpoint`-ам и попробовать приконнектиться к каждому.

Рекомендуем реализовать в сокете вспомогательный статический метод `Connect`, который получает [`asio::ip::tcp::endpoint`](http://think-async.com/Asio/asio-1.18.1/doc/asio/reference/ip__tcp/endpoint.html).

Изучите пример [async_tcp_client](https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp11/timeouts/async_tcp_client.cpp).

### Асинхронный резолвинг адреса (дополнительно)

По аналогии с `Acceptor` напишите класс `Resolver`, который использует `async_resolve`.

### Неблокирующие операции (дополнительно)

В реализации `ReadSome` попробуйте сначала оптимистично читать данные из сокета в неблокирующем режиме, и только в случае неудачи стартовать асинхронную операцию `async_read_some`.

### В ожидании асинхронного результата

Скорее всего в вашей реализации сокетов будет много повторяющегося кода:

При запуске каждой асинхронной операции нужно
1) Остановить текущий файбер и запланировать его возобновление
2) Прокинуть асинхронный результат из коллбэка в файбер

Попробуйте инкапсулировать эту логику в отдельном классе.

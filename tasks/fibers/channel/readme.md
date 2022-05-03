# Каналы

## Пререквизиты

- [fibers/mutex](/tasks/fibers/mutex)
- [futures/executors](/tasks/futures/executors)
- Интрузивные задачи
- [fibers/scheduler](/tasks/fibres/scheduler) – рекомендуется

## Go Proverb

_Do not communicate by sharing memory; instead, share memory by communicating._

[Share Memory By Communicating](https://blog.golang.org/codelab-share)

### Communicating Sequential Processes

- [Original paper](https://www.cs.cmu.edu/~crary/819-f09/Hoare78.pdf)
- [Book](http://www.usingcsp.com/cspbook.pdf)


## Буферизированный канал

[A Tour of Go / Buffered Channels](https://tour.golang.org/concurrency/3)

Реализуйте [`Channel<T>`](exe/fibers/sync/channel.hpp) – канал с буфером фиксированного размера, который позволяет отправлять данные из одного файбера в другой.

Метод `Send` останавливает _файбер_ (но не блокирует поток!) если буфер канала заполнен, метод `Receive` – если пуст.

Канал – MPMC (_multiple producers_ / _multiple consumers_).

Для простоты мы обойдемся без `Close` и неблокирующих вариаций `TrySend` / `TryReceive`.

### Пример

```cpp
void ChannelExample() {
    executors::ThreadPool scheduler{/*threads=*/4};

  fibers::Go(scheduler, []() {
    fibers::Channel<int> msgs{16};

    // Producer
    // Захватываем канал по значению, владение каналом – разделяемое
    fibers::Go([msgs]() mutable {
      for (int i = 0; i < 128; ++i) {
        msgs.Send(i);
      }

      // Poison pill
      msgs.Send(-1);
    });

    // Consumer
    while (true) {
      int value = msgs.Receive();
      if (value == -1) {
        break;
      }
      std::cout << "Received value = " << value << std::endl;
    }
  });

  scheduler.WaitIdle();
  scheduler.Stop();
}
```

### Синхронизация

Для синхронизации продьюсеров и консьюмеров используйте спинлок (так делают в Golang). Лок-фри реализация не требуется.

### Рандеву

Если в канале есть ждущие консьюмеры, то в `Send` передавайте сообщение консьюмеру напрямую, со стека на стек, минуя буфер канала.

### Аллокации

Методы `Send` / `Receive` не должны выполнять динамических аллокаций памяти:

- Для списка заблокированных продьюсеров / консьюмеров используйте интрузивные списки
- [Опционально] Для буфера сообщений используйте циклический буфер

### Misc

Не пишите в `Send` / `Receive` код, похожий на код с кондварами, в котором есть завернутое в цикл блокирующее ожидание. В обоих методах достаточно заблокироваться только один раз.

## `Select`

[Go by Example: Select](https://gobyexample.com/select)

Реализуйте функцию [`Select(xs, ys)`](exe/fibers/sync/select.hpp), которая блокирует файбер до появления первого сообщения в одном из двух каналов `xs` / `ys`:

```cpp
 fibers::Channel<X> xs;
 fibers::Channel<Y> ys;
 
 // ...

 std::variant<X, Y> value = fibers::Select(xs, ys);
 switch (value.index()) {
   case 0:
     // Handle std::get<0>(value);
     break;
   case 1:
     // Handle std::get<1>(value);
     break;
 }
```

В `Select` разрешается передавать только разные каналы, вызов `Select(xs, xs)` – UB.

### Сценарии

`Select`-ы могут
- конкурировать как с `Receive`-ами, так и с другими `Select`-ами
- пересекаться по наборам каналов
- ждать на одних и тех же каналах в разном порядке

### Golang

В языке Go `select` более выразительный:
- В нем не фиксировано число каналов
- В нем можно как получать, так и отправлять сообщения
- У него есть неблокирующий вариант (с `default`)

### Число каналов

Придуманный вами алгоритм синхронизации для `Select` должен обобщаться на произвольное число каналов.

Мы ограничили `Select` двумя каналами чтобы избежать сложного шаблонного кода в реализации.

Задача со звездочкой: реализуйте variadic `Select`, работающий с произвольным числом каналов.

### Fairness

Реализация `Select` не должна отдавать приоритет одному из каналов: если `Select` вызывается в цикле, то файбер должен регулярно получать сообщения из обоих каналов.

### Аллокации

Реализация `Select` не должна выполнять динамических аллокаций памяти.

Используйте `IntrusiveList`. Вам пригодится метод `Unlink` у `IntrusiveListNode`.

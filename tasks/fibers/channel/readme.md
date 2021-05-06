# Каналы

## Пререквизиты

- [fibers/mutex](/tasks/fibers/mutex)

## Go Proverb

_Do not communicate by sharing memory; instead, share memory by communicating._

https://blog.golang.org/codelab-share

## Буферизированный канал

[A Tour of Go / Buffered Channels](https://tour.golang.org/concurrency/3)

Реализуйте [`Channel<T>`](mtf/fibers/sync/channel.hpp) – буферизированный канал фиксированной емкости, который позволяет отправлять данные из одного файбера в другой.

Метод `Send` блокирует _файбер_ (не поток!) если буфер канала заполнен, метод `Receive` – если пуст.

Канал – MPMC (_multiple producers_ / _multiple consumers_).

Для простоты мы обойдемся без `Close` и неблокирующих вариаций `TrySend` / `TryReceive`.

### Блокировки / lock-free

В реализации канала используйте блокировки (так делают в Golang). Лок-фри реализация не требуется.

### Рандеву

Если в канале есть ждущие консьюмеры, то в `Send` передавайте сообщение консьюмеру напрямую, минуя буфер канала. Этим вы сэкономите синхронизацию.

Не пишите код, похожий на код с кондварами / мьютексами.

## `Select`

[Go by Example: Select](https://gobyexample.com/select)

Реализуйте функцию [`Select(xs, ys)`](mtf/fibers/sync/select.hpp), которая блокирует файбер до появления первого сообщения в одном из двух каналов `xs` / `ys`:

```cpp
 Channel<X> xs;
 Channel<Y> ys;
 
 // ...

 std::variant<X, Y> value = Select(xs, ys);
 switch (value.index()) {
   case 0:
     // Handle std::get<0>(value);
     break;
   case 1:
     // Handle std::get<1>(value);
     break;
 }
```

### Сценарии

`Select`-ы могут
- конкурировать как с `Receive`-ами, так и с другими `Select`-ами
- пересекаться по наборам каналов
- ждать на одних и тех же каналах в разном порядке

### Число каналов

Придуманный вами алгоритм синхронизации для `Select` должен обобщаться на произвольное число каналов.

Мы ограничили `Select` двумя каналами чтобы избежать сложного шаблонного кода в реализации.

### Прогресс

Реализация `Select` не должна отдавать приоритет одному из каналов: если `Select` вызывается в цикле, то файбер должен регулярно получать сообщения из обоих каналов.

### Аллокации

Реализация `Select` не должна выполнять дополнительных динамических аллокаций памяти.

### Применения

- Ожидание с таймаутом: [Go by Example / Tickers](https://gobyexample.com/tickers)
- Мультиплексирование событий из разных источников: [etcd/raft](https://github.com/etcd-io/etcd/blob/bd4f8e2b6c6a0bdcd52f4593f68d9f2415ab5293/raft/node.go#L341)
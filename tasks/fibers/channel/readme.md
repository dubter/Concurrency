# Каналы

_Do not communicate by sharing memory; instead, share memory by communicating._

https://blog.golang.org/codelab-share

## Буферизированный канал

[A Tour of Go / Buffered Channels](https://tour.golang.org/concurrency/3)

Реализуйте [`Channel<T>`](mtf/fibers/sync/channel.hpp) – буферизированный канал, который позволяет отправлять данные из одного файбера в другой.

Метод `Send` блокирует _файбер_ (не поток!) если канал заполнен, метод `Receive` – если пуст.

Для простоты мы обойдемся без `Close` и неблокирующих вариаций `TrySend` / `TryReceive`.

## `Select`

[Go by Example: Select](https://gobyexample.com/select)

Реализуйте функцию [`Select<X, Y>(xs, ys)`](mtf/fibers/sync/select.hpp), которая позволяет файберу заблокироваться до появления первого сообщения в одном из двух каналов `xs` / `ys`:

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

### Прогресс

Реализация `Select` не должна отдавать приоритет одному из каналов: если `Select` вызывается в цикле, то файбер должен регулярно получать сообщения из обоих каналов.

### Аллокации

Реализация `Select` не должна выполнять дополнительных динамических аллокаций памяти.

## Бонусные материалы

- [Roman Elizarov — Lock-Free Algorithms for Kotlin Coroutines (Part 1)](https://www.youtube.com/watch?v=W2dOOBN1OQI), [Part 2](https://www.youtube.com/watch?v=iQsN_IDUTSc) – lock-free реализация с помощью [двусвязного списка (Sundell / Tsigas)](http://www.cse.chalmers.se/~tsigas/papers/Lock-Free-Deques-Doubly-Lists-JPDC.pdf) и [multi-word CAS](https://www.cl.cam.ac.uk/research/srg/netos/papers/2002-casn.pdf)  


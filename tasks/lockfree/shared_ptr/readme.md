# `AtomicSharedPtr`

В задаче [stack](/tasks/lockfree/stack) подсчет ссылок был встроен прямо в реализацию лок-фри контейнера.

Было бы гораздо удобнее иметь неинвазивный механизм подсчета ссылок.

## `shared_ptr`

Несмотря на то, что `std::shared_ptr` использует атомарный подсчет ссылок, он не поддерживает конкурентные конфликтующие операции:

```cpp
void MaybeRace() {
  // Разделяемый между потоками std::shared_ptr
  auto sp = std::make_shared<std::string>("Init");

  std::thread reader([&sp]() {
    // 1) Сначала читаем указатель на control block shared_ptr-а
    //    из поля объекта `sp`
    // 2) Переходим по указателю и увеличиваем счетчик сильных ссылок
    //    в control block-е
    auto sp1 = sp;  // <-- heap-use-after-free
  });

  std::thread writer([&sp]() {
    // Уменьшаем счетчик `sp` до нуля и разрушаем control block
    // между шагами 1) и 2) потока `reader`
    auto sp2 = std::make_shared<std::string>("Writer");
    sp = sp2;
  });

  reader.join();
  writer.join();
}
```

По сути, это та же гонка, что и в наивной реализации лок-фри стека:

```cpp
std::optional<T> TryPop() {
  while (true) {
    Node* curr_top = top_.load();
    if (curr_top == nullptr) {
      return std::nullopt;
    }
    // Между
    // 1) чтением curr_top = top_.load() и
    // 2) чтением curr_top->next
    // конкурирующий вызов `TryPop` мог извлечь из стека и удалить узел,
    // на который указывает curr_top текущего вызова.
    if (top_.compare_exchange_weak(curr_top, curr_top->next)) {  // <-- heap-use-after-free
      T value = std::move(curr_top->value);
      delete curr_top;
      return value;
    }
  }
}
```

## `atomic_shared_ptr`

[`atomic_shared_ptr`](https://en.cppreference.com/w/cpp/experimental/atomic_shared_ptr) держит сильную ссылку на разделяемый объект и позволяет выполнять над ней стандартные атомарные операции: `store`, `load`, `compare_exchange_weak` и т.д.

Можно представлять, что `atomic_shared_ptr` – это `atomic<shared_ptr>`.

Атомарность операций в `atomic_shared_ptr` обеспечивается уже не на уровне процессора, а с помощью специального лок-фри алгоритма.

## Задание

Вам дана [реализация лок-фри стека](lock_free_stack.hpp). Напишите для нее классы [`SharedPtr<T>` и `AtomicSharedPtr<T>`](shared_ptr.hpp).

Реализация `AtomicSharedPtr<T>` должна быть лок-фри.

## References

- [Implementing a Lock-free `atomic_shared_ptr`](https://github.com/brycelelbach/cppnow_presentations_2016/blob/master/01_wednesday/implementing_a_lock_free_atomic_shared_ptr.pdf)
- [`folly::AtomicSharedPtr<T>`](https://github.com/facebook/folly/blob/main/folly/concurrency/AtomicSharedPtr.h)
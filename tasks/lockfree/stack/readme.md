# Treiber Stack

Напишите [лок-фри стек Трайбера](https://en.wikipedia.org/wiki/Treiber_stack).

## Управление памятью

Для управления памятью используйте подсчет ссылок.

Счетчик ссылок вершины стека разделите на положительную и отрицательную компоненты:

- Отрицательную компоненту храните внутри узла
- Положительную – прямо на указателе на вершину стека

### `AtomicStampedPtr`

Вам дан класс [`AtomicStampedPtr<T>`](atomic_stamped_ptr.hpp), который хранит указатель + 16-битный счетчик в виде одного машинного слова и умеет выполнять над этой парой стандартные атомарные операции:

```cpp
AtomicStampedPtr<T> asp({nullptr, 0});

asp.Store({raw_ptr, 7});
auto stamped_ptr = asp.Load();
// Метод `IncrementCount` возвращает новый `StampedPtr`, 
// в котором счетчик увеличен на единицу
bool succeeded = asp.CompareExchangeWeak(stamped_ptr, stamped_ptr.IncrementCount());
```

Счетчик в `AtomicStampedPtr` самостоятельного смысла не имеет.

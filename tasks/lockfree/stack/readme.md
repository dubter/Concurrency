# Treiber Stack

Напишите [лок-фри стек Трайбера](https://en.wikipedia.org/wiki/Treiber_stack), используя дифференцированный подсчет ссылок.

## Подсчет ссылок

Счетчик ссылок для узла стека разделите на две компоненты:

- Декременты счетчика ссылок храните внутри узла.
- Инкременты счетчика храните прямо на указателе на узел.

### `AtomicStampedPtr`

Используйте [`AtomicStampedPtr<T>`](atomic_stamped_ptr.hpp), который хранит указатель + 16-битный счетчик в виде одного машинного слова и умеет выполнять над этой парой стандартные атомарные операции:

```cpp
AtomicStampedPtr<T> asp({nullptr, 0});

asp.Store({raw_ptr, 7});
auto stamped_ptr = asp.Load();
// Метод `IncrementCount` возвращает новый `StampedPtr`, 
// в котором счетчик увеличен на единицу
bool succeeded = asp.CompareExchangeWeak(stamped_ptr, stamped_ptr.IncrementCount());
```

Счетчик в `AtomicStampedPtr` самостоятельного смысла не имеет.

### `AtomicSharedPtr`

В этой задаче инкременты счетчика узла могут переполнить выделенные для него биты указателя. Эту проблему мы решим в задаче [lockfree/shared_ptr](/tasks/lockfree/shared_ptr).
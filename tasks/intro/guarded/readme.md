# Защитник

Реализуйте обертку [`Guarded<T>`](guarded.hpp), которая автомагически превращает произвольный класс в *потокобезопасный* (*thread-safe*).

Пример:

```cpp
Guarded<std::vector<int>> ints; // vector<int> + mutex
// ~ 1) mutex.lock() -> 2) push_back(42) -> 3) mutex.unlock()
ints->push_back(42);
```

Тип защищаемого объекта и набор методов `Guarded`-у заранее неизвестен, он должен уметь оборачивать произвольный класс.

Изучите [Synchronized](https://github.com/facebook/folly/blob/master/folly/docs/Synchronized.md) из библиотеки folly.

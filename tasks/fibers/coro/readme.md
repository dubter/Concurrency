# [Stackful] Coroutine

_Сопрограмма_ (_coroutine_) или просто _корутина_ – это процедура,
из вызова которой можно выйти в середине (_остановить_ вызов, _suspend_), а затем вернуться в этот вызов
и продолжить исполнение с точки остановки.

Сопрограмма расширяет понятие _подпрограммы_ (_subroutine_), вызов которой нельзя остановить, а можно лишь завершить.

## Simple `Coroutine`

Экземпляр класса `Coroutine` представляет вызов процедуры,
1) который может остановиться (операция `Suspend`), и
2) который затем можно возобновить (операция `Resume`).

Создадим корутину:

```cpp
Coroutine co(routine);
```

Здесь `routine` – произвольная пользовательская процедура, которая будет исполняться корутиной `co`. 

Непосредственно создание корутины не приводит к запуску `routine`.

Созданная корутина запускается вызовом `co.Resume()`. После этого управление передается процедуре `routine`, и та исполняется до первого вызова `Suspend()` (или до своего завершения).

Вызов `Suspend()` в корутине останавливает ее исполнение, передает управление обратно caller-у и завершает его вызов `co.Resume()`. Вызов `Suspend()` – это точка выхода из корутины, _suspension point_.

Следующий вызов `co.Resume()` вернет управление остановленной корутине, вызов `Suspend()` в ней завершится, и она продолжит исполнение до очередного `Suspend()` или же до завершения своей процедуры.

С исключениями корутины взаимодействуют как обычные функции: если в корутине было выброшено и не перехвачено исключение, то оно вылетит в caller-е из вызова `co.Resume()` и полетит дальше (выше) по цепочке вызовов.

Процедура, исполняемая в корутине, не имеет доступа к самому объекту `Coroutine`. Чтобы остановить исполнение, процедура вызывает статический метод `Suspend`.

Для лучшего понимания API и потока управления в корутинах изучите [тесты](tests/unit.cpp) к задаче.

### Пример

```cpp
void StackfulCoroutineExample() {
  // Stackful корутина
  Coroutine co([] {
    // Роль: callee
    
    fmt::println("Step 2");
    
    Coroutine::Suspend();  // <-- Suspension point

    fmt::println("Step 4");
  });
  // <-- Исполнение процедуры `routine` пока не стартовало

  fmt::println("Step 1");

  // Роль: caller
  // Стартуем корутину
  // Управление передается процедуре routine,
  // и та исполняется до первого вызова Suspend().
  co.Resume();

  fmt::println("Step 3");

  co.Resume();

  // Вызов IsCompleted() возвращает true если корутина уже завершила 
  // свое исполнение (дойдя до конца `routine` или бросив исключение).
  assert(co.IsCompleted());

  // Попытка вызова Suspend вне корутины - UB
  // Coroutine::Suspend()
}
```

На экране будет напечатано:
```
Step 1
Step 2
Step 3
Step 4
```

## Типы корутин

Идея корутины может быть материализована в программе двумя способами:

- В этой задаче мы говорим про _stackful_ реализацию: в вызове `Suspend` корутина сохраняет текущий контекст исполнения и активирует контекст исполнения родителя, останавливая тем самым весь текущий стек вызовов.


- В _stackless_ реализации переключение контекста не используется, корутина реализуется через "переписывание" функции в автомат (_state machine_) прямо в компиляторе или же с помощью макросов.

В С++ вы можете выбрать как [stackful](https://www.boost.org/doc/libs/1_75_0/libs/coroutine2/doc/html/index.html), так и [stackless](https://en.cppreference.com/w/cpp/language/coroutines) подход.

## Применения корутин

- [Асинхронность](/tasks/fibers/yield)
- [Итераторы](https://journal.stuffwithstuff.com/2013/01/13/iteration-inside-and-out/)
- Генераторы

## Корутины и файберы

И stackful корутины, и файберы – исполняемые сущности, которые могут останавливаться и затем возобновлять исполнение.
И те, и другие используют механизм переключения контекста для нелокальной передачи управления.

Но стоит отличать их друг от друга!

### Файберы

Файберы – это кооперативная многозадачность: независимые активности, чередующиеся на процессоре.

За исполнение файберов отвечает планировщик, его задача – распределять файберы между потоками (аналогично планировщику операционной системы, который распределяет потоки между ядрами процессора) и при остановке файбера выбирать следующий файбер ему на замену.

Помимо планировщика, файберам нужны собственные средства синхронизации (мьютексы, ивенты, каналы и т.д.), которые не будут блокировать потоки планировщика в точках ожидания.

### Корутины

Корутины гораздо ближе к обычным функциям, чем к файберам.

Корутины не имеют прямого отношения к многозадачности и потокам.

Корутины не нуждаются в примитивах синхронизации. 

Корутинам не нужен планировщик, который выбирал бы, кому передать управление при остановке исполнения. Управление передается по заданным правилам: от caller-а к callee и обратно через вызовы `Resume` и `Suspend`, как при вызове обычных функций.

## Simple

В этой задаче мы намеренно пишем самую простую, даже **наивную** корутину.

Реализуя файберы, вы сами обнаружите ее несовершенства, и у вас появится мотивация доработать ее дизайн.

## Задание

Реализуйте [`Coroutine`](coroutine.cpp)

## Реализация

### `ExecutionContext`

Корутина умеет останавливать и возобновлять свое исполнение, а значит ей потребуется [`ExecutionContext`](https://gitlab.com/Lipovsky/sure/-/blob/master/sure/context.hpp).

1) Прочтите [документацию по `ExecutionContext`](https://gitlab.com/Lipovsky/sure/-/blob/master/docs/ru/guide.md)
2) Посмотрите как он используется в [TinyFibers](https://gitlab.com/Lipovsky/tinyfibers/-/tree/master/tf/rt)

### Исключения

Используйте [std::exception_ptr](https://en.cppreference.com/w/cpp/error/exception_ptr) для прокидывания исключения из корутины в caller-а.

### API

Набор публичных методов корутины зафиксирован, менять его не следует.
# Настройка Visual Studio Code

## Начальная настройка

### Шаг 0

Установите самую свежую версию [VSCode](https://code.visualstudio.com/download). Cторонние сборки, установленные с помощью пакетных менеджеров, не подходят. Работоспособность протестирована с версии 1.60. 

### Шаг 1

Откройте VSCode и установите расширение [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) в левом меню `Extensions`.

![Dev Container](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-install-extension.png)

### Шаг 2

После установки расширения откройте вкладку `Remote Explorer` и убедитесь, что в списке контейнеров присутствует контейнер с курсом, который был создан и запущен на [шаге настройки Docker](docker.md) (он должен называться `concurrency-course`). 

Подключитесь к контейнеру с помощью кнопки `Attach to Container` и дождитесь настройки контейнера для работы с VSCode.

![Check container](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-check-container.png)

На этом шаге у вас должно открыться дополнительное окно с VSCode, который будет подключен к контейнеру.

Проверьте, что подключение успешно: в левом нижнем углу должно отображаться имя контейнера.

### Шаг 3

Курс – это CMake-проект, так что просто откройте его в VSCode с подключенным контейнером: вкладка `Explorer` → `Open Folder` → выбрать директорию курса в контейнере (`/workspace/concurrency-course`).

![Open folder](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-open-folder.png)

### Шаг 4

Для работы с курсом нужно установить в контейнер следующие расширения аналогично тому, как было установлено расширение Dev Containers:

| Расширение | Функционал |
|---|---|
| _[CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)_ | Поддержка CMake |
| _[C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)_ | Запуск и дебаг кода
| _[clangd](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd)_ | Навигация и автодополнение |

После установки расширения `clangd` и начала работы с кодом VSCode предложит установить `clangd`. С установкой нужно согласиться.

### Шаг 5

После установки расширения для работы с CMake выберите kit - `Clang 14.0.6 x86-64-pc-linux-gnu`. Это можно сделать сразу после установки расширения либо в нижней панели.

![Setup kit](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-choose-kit.png)

### Шаг 6

Смените директорию для сборки проекта. Для этого откройте настройки расширения `CMake Tools` в меню слева под названием `Extensions`.

![Open CMake settings](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-open-cmake-settings.png)

Теперь переключитесь на настройки в группе `Workspace` и найдите настройку `Cmake: Build Directory`. Установите её значение на `/tmp/vscode-build`.

![Choose build directory](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-choose-build-dir.png)

Убедитесь, что настройка задана верно. Для этого проверьте, что файл `.vscode/settings.json` содержит
```
"cmake.buildDirectory": "/tmp/vscode-build"
```

Теперь запустить CMake можно из нижнего меню, выбрав профиль сборки `Debug`.

![Run CMake](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-run-cmake.png)

Также из этого меню можно собирать и запускать цели, соответствующие задаче.

Сначала выберите цель для сборки и соберите её.

![Build targets](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-build.png)

Далее можно выбрать цель для запуска и запустить её.

![Run targets](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-run.png)

### Шаг 7 

Отключите `Intelli Sense Engine`. Для этого в настройках расширения `C/C++` во вкладке `Workspace` поменяйте значение настройки `C_cpp: Intelli Sense Engine` на `disabled`.

![Change cpp setting](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-disable-intelli.png)

Убедитесь, что настройка задана верно. Для этого проверьте, что файл `.vscode/settings.json` содержит
```
"C_Cpp.intelliSenseEngine": "disabled"
```

### Шаг 8

Настройте индексацию проекта `clangd`. Для этого в настройках расширения `clangd` во вкладке `Workspace` поменяйте значение настройки `Clangd: Arguments` на `-compile-commands-dir=/tmp/vscode-build`. 

![Change clangd setting](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-change-clang-setting.png)

Убедитесь, что настройка задана верно: проверьте, что файл `.vscode/settings.json` содержит
```
"clangd.arguments": [
    "-compile-commands-dir=/tmp/vscode-build"
]
```

Теперь при открытии файлов проекта у вас должна работать навигация по коду.

### Шаг 9

Для отладки кода вам потребуется дебаггер. 

Для его настройки откройте любой `.cpp` файл. 

В левом меню откройте `Run and Debug` и нажмите `create a launch.json file`

![Create launch file](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-create-launch-file.png)

Теперь нажмите `Show all automatic debug configurations` → `Add Configuration...` → `C/C++: (gdb) Launch`. 

![Add launch configuration](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-add-configuration.png)


В файле `.vscode/launch.json` будет создан шаблон, который нужно заполнить. 

![Launch template](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-launch-template.png)

В этом файле нас интересуют следующие поля:
- `program` — путь до бинарного файла тестов задачи. Например, `/workspace/concurrency-course/build/tasks/tutorial/aplusb/bin/task_tutorial_aplusb_tests`
- `args` — аргументы командной строки для бинарного файла. Для установки breakpoint'ов нужно выключить запуск тестов в подпроцессе. Для этого добавьте флаг `--disable-forks`. Подробнее можно прочитать в [faq](faq.md).

Должен получиться такой файл. Не забудьте его сохранить!

![Launch file](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-launch-file.png)

Теперь можно запускать дебаггер из меню слева `Run and Debug`.

![Debug](https://gitlab.com/Lipovsky/concurrency-course-media/-/raw/main/docs-images/vscode-debug.png)

## Полезные советы

- Если вы зайдете в терминал с помощью `Terminal` → `New Terminal`, то попадете в контейнер в `root` пользователя. Работайте с `clippy` из отдельного терминала.

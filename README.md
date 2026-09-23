# NetBeyond

Экспериментальная оболочка браузера на C99 для Win32. Версия 0.1 использует системный Internet Explorer WebBrowser ActiveX, а не собственный или Chromium-движок. Современные сайты и расширения Chrome пока не поддерживаются.

## Что реализовано

До восьми вкладок в одном процессе; строка URL/поиска, HTTP(S)-навигация через установленный IE, кнопки Back/Next/Reload, Ctrl+T/W/L/Tab и Enter в строке. URL-модуль имеет локальные C-тесты. HTTPS/TLS, HTML и JS определяются версией системного IE и ОС, а не NetBeyond. Полный статус: [совместимость](docs/COMPATIBILITY.md), [расширения](docs/EXTENSIONS.md).

## Сборка и запуск

32-bit Windows toolchain MinGW-w64 и CMake 3.16+:

```bat
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
build\NetBeyond.exe
```

Linux проверяет только модуль командой `make test`; Windows-приложение на Linux не собирается. Workflow на push/PR собирает i686 exe на Windows и публикует артефакт, но НЕ проверяет запуск на XP, Vista или 7. Текущий MSYS2 MinGW32 официально ориентирован на Windows 7+, поэтому полученный в CI exe нельзя выдавать за проверенный XP/Vista-бинарник. Понадобится отдельная проверенная XP-совместимая цепочка сборки.

## Ручной smoke-test XP

1. XP SP3 x86, установлен IE8; проверить наличие `atl.dll` и регистрации WebBrowser.
2. Собрать 32-битной XP-совместимой цепочкой, проверить импортируемые API/DLL и зависимости CRT.
3. Запустить exe на настоящей XP/VM, открыть `https://example.org`, создать вкладку, перейти назад/вперёд и закрыть вкладку.
4. Записать редакцию ОС, IE, компилятор, ошибки TLS, потребление памяти и результат в issue.

Ни один GUI-бинарник пока не собран или проверен в этом окружении. Не использовать для банковских операций и приватных данных: XP/Vista и IE устарели, изоляции и песочницы нет.

## Зависимости и лицензии

| Компонент | Версия | Лицензия | Назначение | XP |
|---|---|---|---|---|
| IE WebBrowser | установленный IE, на XP максимум IE8 | Microsoft Windows | рендеринг, JS, TLS | старые веб-API и TLS |
| atl.dll | системная версия | Microsoft Windows | ActiveX-контейнер | нужны AtlAxWinInit/AtlAxGetControl |
| MSYS2 mingw-w64-i686-gcc | 16.2.0-3 (проверка CI) | GPL/GCC Runtime Library Exception | сборка CI | текущая сборка MinGW32 не подтверждена на XP |
| CMake | 3.16+ | BSD-3-Clause | генерация сборки | запускается на машине сборки |

Собственный код: MIT (см. LICENSE). Сторонний движок не включён в исходники. CI использует version tags `actions/checkout@v4.2.2`, `msys2/setup-msys2@v2.27.0`, `actions/upload-artifact@v4.6.2`; MSYS2 пакеты CMake/Ninja и базовый образ остаются обновляемыми, то есть полной воспроизводимости ещё нет.

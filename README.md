# NetBeyond

Экспериментальный C99-браузер для Windows XP, Vista и 7. Собственные компоненты: ограниченный HTML text parser, текстовый layout, Win32/GDI UI, URL/history/HTTP parser. IE и ActiveX в runtime path нет. **HTTPS использует статически собираемый Mbed TLS 3.6.6, а не SChannel**; HTTP пока использует WinHTTP. Это не означает совместимость с современными сайтами.

## Сборка и запуск

Для разработки нужны CMake >= 3.16, Git и 32-битная C toolchain для Windows. CMake загружает исходники Mbed TLS с фиксированного commit `0bebf8b8c7f07abe3571ded48a11aa907a1ffb20` и его framework submodule. Интернет нужен при первой конфигурации.

```bat
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
build\NetBeyond.exe
```

На XP нужна отдельно проверенная XP-совместимая toolchain/CRT; артефакт CI на актуальном MSYS2 не доказывает совместимость с XP. GUI и TLS-handshake на настоящих XP/Vista/7 ещё не проверены. Smoke-test: открыть `about:welcome`, затем `http://` и `https://` test URLs, зафиксировать ОС, TLS handshake/certificate outcome, импортируемые DLL, время, RAM и ошибки.

## TLS и доверие

Mbed TLS 3.6.6 поддерживает TLS 1.2/1.3 и имеет лицензию Apache-2.0 OR GPL-2.0-or-later; используется вариант Apache-2.0. Для HTTPS имя сервера передаётся в SNI/hostname verification, проверка сертификата обязательна. Файл `cacert.pem` рядом с exe используется как актуальный CA bundle; если его нет, сертификаты импортируются из Windows ROOT. Использование ROOT — это лишь источник доверенных корней, **не системный TLS-движок**. На XP ROOT может быть старым; скачивать CA bundle нужно из проверенного источника. См. [docs/TLS.md](docs/TLS.md).

## Фактические ограничения

Асинхронная загрузка HTTP(S) и текстовый рендеринг есть в исходниках; успешный Windows build/XP run пока не подтверждён. Один документ, вкладок пока нет. Нет CSS, изображений, JS/DOM, форм, закладок, history UI, extensions MV2/MV3, Wasm, WebGL, WebGPU, WebRTC, Service Workers и sandbox. HTTPS fetch не поддерживает chunked, gzip, redirects, HTTP/2 и proxy; некоторые сайты не загрузятся даже при успешном TLS handshake. Не использовать для конфиденциального веб-сёрфинга.

Код NetBeyond: MIT. Зависимости: Mbed TLS 3.6.6 (Apache-2.0 OR GPL-2.0-or-later; C TLS/crypto, статическая линковка); Win32/GDI, WinHTTP для HTTP, CryptoAPI ROOT store и Winsock (компоненты Windows); CMake >= 3.16 (BSD-3-Clause, сборка); MinGW-w64 GCC (GPL/GCC Runtime Library Exception, сборка). Ограничения XP и версии компонентов описаны выше; rolling MSYS2 packages/runner image пока не закреплены полностью.

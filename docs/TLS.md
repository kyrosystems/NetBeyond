# Встроенный TLS и XP-совместимость

HTTPS использует статически собираемый Mbed TLS 3.6.6 (commit `0bebf8b8c7f07abe3571ded48a11aa907a1ffb20`, лицензия Apache-2.0 OR GPL-2.0-or-later; выбран Apache-2.0). Это сторонний TLS engine на C, не SChannel. HTTP использует WinHTTP без автоматических переходов на HTTPS.

На XP нет `bcrypt.dll`, а в `msvcrt.dll` нет `_vsnprintf_s`; build-tree патчи переводят entropy на CryptoAPI `CryptGenRandom` и secure CRT ветку MinGW на стандартный `vsnprintf`. На XP нет также `inet_pton`/`inet_ntop` в `WS2_32.dll`. CMake передаёт `_WIN32_WINNT=0x0501`, `WINVER=0x0501` и `MBEDTLS_TEST_SW_INET_PTON=1` всей Mbed TLS-сборке, чтобы X.509-код выбрал software parser. CI проверяет четыре запрещённых импорта и прикладывает `imports.txt`.

`cacert.pem` рядом с exe имеет приоритет; иначе корни импортируются из Windows ROOT в Mbed TLS. Проверка цепочки и hostname обязательна. На XP системный ROOT может быть старым: используйте доверенный обновлённый CA bundle, не отключайте верификацию.

Компиляция и анализ импортов не доказывают работу на XP: нужен реальный XP smoke-test, в том числе TLS-handshake и проверка отказа при неверном сертификате. Сейчас нет chunked/gzip/redirects/proxy/HTTP2; renderer не имеет CSS/JS/DOM.

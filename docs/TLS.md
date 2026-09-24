# Встроенный TLS и XP entropy

HTTPS использует статически собираемый Mbed TLS 3.6.6 (commit `0bebf8b8c7f07abe3571ded48a11aa907a1ffb20`, Apache-2.0 OR GPL-2.0-or-later; выбираем Apache-2.0). Это сторонний C TLS engine, не SChannel и не собственная криптография. Обычный HTTP идёт через WinHTTP без автоматических переходов на HTTPS.

Официальный Windows entropy source Mbed TLS импортирует `BCryptGenRandom` из `bcrypt.dll`, которой нет на XP. Мы НЕ распространяем Microsoft DLL. CMake проверяет исходник закреплённой ревизии и только в build-tree заменяет Windows entropy source на XP CryptoAPI `CryptGenRandom`; заменяет bcrypt link на advapi32. Если upstream изменится, конфигурация падает вместо неаудированного патча. CI дополнительно проверяет import table exe и прикладывает `imports.txt`. Это не отменяет отдельного XP runtime-test.

Проверка сертификата и имени хоста обязательна. `cacert.pem` рядом с exe имеет приоритет; иначе корни берутся из Windows ROOT и импортируются в Mbed TLS. Это источник доверия, а не системный TLS. На старой XP обновите корни через доверенный CA bundle. Не отключайте проверку для обхода ошибок.

HTTPS HTTP/1.1 path пока отклоняет chunked, не поддерживает gzip, redirects, proxy, HTTP/2; TLS handshake/XP запуск не проверены. Даже при TLS 1.3 собственный renderer не имеет CSS/JS/DOM.

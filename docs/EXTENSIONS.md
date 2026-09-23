# Расширения Chrome

Manifest V2: не поддерживается. Manifest V3: не поддерживается. Тестов с реальными расширениями нет. Наличие Internet Explorer ActiveX не означает совместимость с расширениями Chrome.

| Компонент/API | MV2 | MV3 | Факт |
|---|---|---|---|
| `manifest.json`, CRX, permissions | требуется | требуется | нет |
| `chrome.tabs`, `windows`, `storage` | используется | используется | нет |
| content scripts | используются | используются | нет |
| background pages | используются | нет | нет |
| background service worker | нет | используется | нет |
| `declarativeNetRequest`, `webRequest` | возможно | возможно | нет |
| popup, DevTools API | возможно | возможно | нет |

Требуется отдельная реализация loader/runtime/API и матрица тестов реальных MV2/MV3 расширений. Не называйте будущую поддержку полной без испытаний.

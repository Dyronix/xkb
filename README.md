# 🧩 xkb

`xkb` provides platform-specific keyboard backends for Linux.  
It currently supports two implementations:

- **Evdev backend** – low-level raw input via `/dev/input/event*`
- **X11 backend** – high-level input for X11 windowed environments

Both backends implement the shared `IKeyboardBackend` interface and can be integrated seamlessly with any system that needs key state updates.

---

## 📁 Directory Structure

```
xkb/
├─ include/
│   ├─ xkbIKeyboardBackend.h
│   ├─ xkbEvdevKeyboardBackend.h
│   ├─ xkbX11KeyboardBackend.h
│   └─ xkbInputDetection.h
└─ src/
    ├─ xkbEvdevKeyboardBackend.cpp
    ├─ xkbX11KeyboardBackend.cpp
    └─ xkbInputDetection.cpp
```

---

## 🧱 Interface Overview

```cpp
class IKeyboardBackend
{
public:
    using SetKeyFn = std::function<void(size_t keyIndex, bool down)>;

    virtual ~IKeyboardBackend() = default;
    virtual bool Init() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;

    void SetCallbacks(SetKeyFn setKey);
};
```

Each backend runs its own input loop on a dedicated thread.  
The provided callback (`SetKeyFn`) is called whenever a key state changes.

---

## ⚙️ Usage Example

```cpp
#include <xkb/X11KeyboardBackend.h>
#include <xkb/EvdevKeyboardBackend.h>
#include <X11/Xlib.h>
#include <memory>
#include <cstdio>

void OnKeyEvent(size_t key, bool down)
{
    printf("Key %zu %s\n", key, down ? "pressed" : "released");
}

int main()
{
    // Example for X11
    Display* display = XOpenDisplay(nullptr);
    Window window = DefaultRootWindow(display);

    X11KeyboardBackend keyboard(display, window);
    keyboard.SetCallbacks(OnKeyEvent);

    if (keyboard.Init())
        keyboard.Start();

    // ... run your main loop ...

    keyboard.Stop();
    XCloseDisplay(display);
}
```

---

**Author:** Dave De Breuck  
**Module Type:** Platform-specific input backend  
**License:** MIT

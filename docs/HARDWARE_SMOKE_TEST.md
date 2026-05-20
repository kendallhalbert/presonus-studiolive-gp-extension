# Hardware smoke test — StudioLive 32R

Run this on the **same PC as Gig Performer**, with the **32R powered on** and reachable on the LAN (TCP port **53000**).

## 1. Install the Release DLL

```powershell
cd C:\Users\KenHa\source\repos\presonus\presonus-studiolive-gp-extension
cmake --build build-rel --config Release --parallel
```

Copy `build-rel\bin\Release\PreSonusStudioLive.dll` into your GP **Extensions** folder. Typical paths:

- `C:\Users\Public\Documents\Gig Performer\Extensions\`
- `%USERPROFILE%\Documents\Gig Performer\Extensions\`

In GP: **Options → Reload Third Party Libraries** (or restart GP). Enable **PreSonus StudioLive** under **Options → Extensions**.

## 2. Pre-flight

| Check | How |
| ----- | --- |
| Mixer IP | UC Surface / router DHCP list / ping the 32R |
| Port 53000 | `Test-NetConnection -ComputerName <ip> -Port 53000` |
| Channel | Use **input 1** as a **LINE** source (same as fixture capture) |
| No other client | Close UC Surface / other apps using the mixer TCP session |

## 3. GPScript (rackspace script)

GPScript rule: functions that **return a value** must be used in an assignment or inside
`Print(...)` — they cannot stand alone as a statement. `Disconnect()` has no return
value and may be called by itself.

Replace `YOUR_MIXER_IP` with the 32R address (e.g. `10.0.0.14`).

```gigperformer
// All-in-one smoke test (only valid top-level blocks: Var, Initialization, On …, Function)
Initialization
    Print(PreSonusStudioLive_SetLogLevel("debug"))
    Print(PreSonusStudioLive_LogFilePath())
    Print(PreSonusStudioLive_Version())
    Print(PreSonusStudioLive_Connect("10.0.0.14"))
    Print(PreSonusStudioLive_IsConnected())
    Print(PreSonusStudioLive_SetLineMute(1, 1))
    Print(PreSonusStudioLive_SetLineMute(1, 0))
End
```

**Optional — drive steps from a rackspace widget** (add a widget named `PSL_Test` in the
panel, then use this script):

```gigperformer
var PSL_Test : Widget

On WidgetValueChanged(newValue : Double) from PSL_Test
    if newValue < 0.25
        Print(PreSonusStudioLive_Connect("10.0.0.14"))
        Print(PreSonusStudioLive_IsConnected())
    elsif newValue < 0.5
        Print(PreSonusStudioLive_SetLineMute(1, 1))
    elsif newValue < 0.75
        Print(PreSonusStudioLive_SetLineMute(1, 0))
    else
        PreSonusStudioLive_Disconnect()
        Print("disconnected")
    end
End
```

Turn the widget in steps (or use a knob): low → connect, mid → mute on/off, high → disconnect.
Names like `OnUnmute` or `OnConnect` are **not** valid GPScript — only documented `On …` callbacks.

**Minimal one-shot** (paste into Initialization only — edit IP first):

```gigperformer
Initialization
    Print(PreSonusStudioLive_SetLogLevel("debug"))
    Print(PreSonusStudioLive_LogFilePath())
    Print(PreSonusStudioLive_Connect("10.0.0.14"))
    Print(PreSonusStudioLive_IsConnected())
    Print(PreSonusStudioLive_SetLineMute(1, 1))
End
```

> GPScript may not support `Wait()` in all contexts; prefer separate widget/button handlers if Initialization runs too fast.

## 4. What to observe

### Gig Performer log

Expect lines similar to:

- `PSL version: 1.0.0-phase0`
- `Connect(...) = true` / `IsConnected = true`
- `SetLineMute(1, mute) = true` (boolean only means the command was **queued/sent**, not that the desk changed)

### File log

Open the path from `PreSonusStudioLive_LogFilePath()`, usually:

`%APPDATA%\PreSonusStudioLive\extension.log`

Look for session banner, `Mixer connected to <ip>`, and any warnings.

### Mixer surface

Watch **input 1** mute LED / UC Surface mute state when you call `SetLineMute(1, 1)` then `(1, 0)`.

## 5. Record results

| Step | Pass? | Notes |
| ---- | ----- | ----- |
| Extension loads in GP | | |
| `Version()` prints | | |
| `Connect` → true | | |
| `IsConnected` → true | | |
| `extension.log` has connect line | | |
| Input 1 mute **changes on desk** | | |
| `SetLineMute` returns true | | |

**If connect works but mute does not move the desk:** rebuild with a DLL that includes UCNet handshake (JM Subscribe + wait for ZB + `SubscriptionReply`). Older builds only opened TCP without subscribing.

**If connect fails:** verify IP, firewall, and that nothing else holds the TCP session.

## 6. After the test

Paste into your session notes (or an issue):

- GP version (5 Pro / etc.)
- 32R firmware (e.g. 3.3.0.109659)
- Mixer IP
- Connect true/false
- Mute on desk yes/no
- Last ~20 lines of `extension.log`

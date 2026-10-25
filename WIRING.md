# Wiring / لیست اتصالات

Exactly what connects to what. Match the label on the wire, not the color (colors vary).
دقیقاً هر چیزی به کجا وصل می‌شود. معیار، **برچسب/پین** است نه رنگ سیم.

---

## 1) HC-SR04 sensors (×3) → Arduino

Each sensor has 4 pins: `VCC TRIG ECHO GND`.

| Sensor | VCC | GND | TRIG | ECHO |
|---|---|---|---|---|
| U2 FRONT (ahead) | 5V | GND | D2 | D3 |
| U3 SIDE (left) | 5V | GND | D4 | D5 |
| U4 TOP (overhead) | 5V | GND | D6 | D7 |

> ⚠️ All `VCC` go to **Arduino 5V** (sensors draw little current). All `GND`
> go to the **common ground** rail.

## 2) Vibration motors (×3) via transistor drivers

Never connect a motor straight to an Arduino pin — a motor can pull 60–150 mA
and its inductive kickback can kill the pin. Drive each motor with an **NPN
transistor** (2N2222 / BC547) and a **flyback diode**.

One driver per motor:

```
+5V ──▶|◀─ motor ── collector (Q)
Arduino PWM pin ── 1kΩ ── base (Q)
emitter (Q) ── GND
(1N4007 diode across the motor, band/cathode toward +5V)
```

| Motor | Direction | Arduino | Transistor | Base resistor | Diode |
|---|---|---|---|---|---|
| M1 | FRONT | D9 | Q1 | 1 kΩ | D1 (1N4007) |
| M2 | SIDE (left) | D10 | Q2 | 1 kΩ | D2 (1N4007) |
| M3 | TOP | D11 | Q3 | 1 kΩ | D3 (1N4007) |

Mount M1/M2/M3 on the **handle** in the corresponding position (front, left side,
top) so the user feels the direction.

> ⚠️ The three motors together can pull ~200–400 mA. **Do not power them from the
> Arduino 5V pin.** Use a separate 5 V supply ≥ 1 A (USB power bank or 5 V
> regulator from a Li-ion/LiPo battery) and **tie its GND to the Arduino GND**
> (common ground is mandatory).

## 3) Power

| Supply | Connects to |
|---|---|
| Arduino USB → PC | serial data @ 9600 baud |
| Arduino 5V | sensor VCC (3×) + logic |
| External 5V ≥1A (separate) | motor V+ (3×) |
| GND (all) | one common ground: Arduino, sensors, transistors' emitters, diode cathodes |

## 4) Arduino pin summary

| Pin | Used for |
|---|---|
| D2 / D4 / D6 | TRIG (FRONT / SIDE / TOP) |
| D3 / D5 / D7 | ECHO (FRONT / SIDE / TOP) |
| D9 / D10 / D11 | Motor base drive via 1 kΩ (M1 / M2 / M3) |
| 5V | Sensors VCC |
| GND | common ground |

## Common-ground checklist

Everything below shares **one** GND net:

- Arduino GND
- 3× HC-SR04 GND
- 3× transistor emitter
- 3× flyback diode (cathode side net is +5V; the other side meets the motor/collector)
- External supply −

/Project
│
├── main.c
├── system.c
│
├── i2c.c
├── i2c.h
│
├── uart1.c
├── uart1.h
│
├── lcd.c
├── lcd.h
│
├── delay.c
├── delay.h
│
├── types.h


Excellent question 👍
This goes to the **core of how I²C EEPROM works**.
Let’s break it **slowly and clearly**.

---

# ❓ Why `0x50` and `0x0000` are used?

These **two numbers mean two completely different things**:

| Value    | Meaning                           |
| -------- | --------------------------------- |
| `0x50`   | **I²C slave address** of AT24C256 |
| `0x0000` | **Memory location inside EEPROM** |

---

# 1️⃣ Why `0x50` ?

## 🔹 What is `0x50`?

`0x50` is the **7-bit I²C address** of your AT24C256 EEPROM.

### AT24C256 address format (datasheet):

```
1 0 1 0  A2  A1  A0
```

* `A2 A1 A0` = hardware pins on chip
* Usually connected to **GND**

So:

```
1010 000 (binary) = 0x50
```

✔ That’s why we use **0x50**

---

## 🔹 What happens in code?

```c
i2c_write(slaveAddr << 1);
```

If `slaveAddr = 0x50`:

```
0x50 << 1 = 0xA0   → Write
0x50 << 1 | 1 = 0xA1 → Read
```

✔ Exactly what AT24C256 expects

---

## 🔹 If A0/A1/A2 pins change?

| A2 A1 A0 | Address |
| -------- | ------- |
| 000      | 0x50    |
| 001      | 0x51    |
| 010      | 0x52    |
| 011      | 0x53    |

---

# 2️⃣ Why `0x0000` ?

## 🔹 What is `0x0000`?

`0x0000` is the **starting memory address** inside the EEPROM.

### AT24C256 memory size:

* 32 KB total
* Address range:

```
0x0000  → 0x7FFF
```

---

## 🔹 Why start at `0x0000`?

Because:

* It is the **first byte**
* Simple for testing
* Guaranteed valid address

---

## 🔹 What is stored there?

```c
EEPROM address 0x0000 → '1'
EEPROM address 0x0001 → '2'
EEPROM address 0x0002 → '3'
...
```

---

# 3️⃣ How EEPROM addressing works internally

When you call:

```c
i2c_eeprom_pagewrite(0x50, 0x0000, rfid_buff, 8);
```

### I²C bus sees:

```
START
→ 0xA0  (EEPROM write)
→ 0x00  (Address high byte)
→ 0x00  (Address low byte)
→ 0x31  ('1')
→ 0x32  ('2')
→ ...
→ STOP
```

---

# 4️⃣ What if I change `0x0000`?

Example:

```c
0x0100
```

Then data will be stored at:

```
EEPROM[256] → EEPROM[263]
```

Useful for:

* Multiple RFID cards
* Logs
* User database

---

# 5️⃣ COMMON CONFUSION (VERY IMPORTANT)

❌ `0x50` is NOT memory address
❌ `0x0000` is NOT I²C address

✔ `0x50` → WHO to talk to
✔ `0x0000` → WHERE to store inside it

---

# 6️⃣ SIMPLE ANALOGY 🧠

Think of **EEPROM as a building**:

| Value    | Meaning         |
| -------- | --------------- |
| `0x50`   | Building number |
| `0x0000` | Room number     |

---

# ✅ FINAL SHORT ANSWER

> **`0x50` selects the AT24C256 chip on the I²C bus, and `0x0000` selects the first memory location inside that chip.**

---

If you want next:

* Auto-increment EEPROM address
* Store **multiple RFID cards**
* Avoid overwriting old data
* EEPROM memory map design

Just say 👍

# ========================================
# 와사비 스마트팜 - 센서노드 설정 가이드
# ========================================
# 구역 번호 체계: g1(1~3), g2(4~18)
# ========================================

## 구역별 센서노드 배분

| 구역 | 그룹 | 센서 유형 | 노드 ID |
|------|------|-----------|---------|
| 1 | g1 | 대기+토양 통합 | g1-01 |
| 2 | g1 | 대기+토양 통합 | g1-02 |
| 3 | g1 | 대기+토양 통합 | g1-03 |
| 4 | g2 | 토양 | g2-01 |
| 5 | g2 | 토양 | g2-02 |
| 6 | g2 | 토양 | g2-03 |
| 7 | g2 | 토양 | g2-04 |
| 8 | g2 | 토양 | g2-05 |
| 9 | g2 | 토양 | g2-06 |
| 10 | g2 | 토양 | g2-07 |
| 11 | g2 | 토양 | g2-08 |
| 12 | g2 | 토양 | g2-09 |
| 13 | g2 | 토양 | g2-10 |
| 14 | g2 | 토양 | g2-11 |
| 15 | g2 | 토양 | g2-12 |
| 16 | g2 | 토양 | g2-13 |
| 17 | g2 | 토양 | g2-14 |
| 18 | g2 | 토양 | g2-15 |

# ========================================
# g1 - 대기+토양 통합 노드 (구역 1-3)
# ========================================

## g1-01 (구역 1) 설정

config.h:
```cpp
#define NODE_ID "g1-01"
#define ZONE_ID "1"
#define TANK_ID "1"
```

MQTT Topic:
- 대기: smartfarm/wasabi/g1_air_soil/g1-01/air/data
- 토양: smartfarm/wasabi/g1_air_soil/g1-01/soil/data

---

## g1-02 (구역 2) 설정

config.h:
```cpp
#define NODE_ID "g1-02"
#define ZONE_ID "2"
#define TANK_ID "2"
```

MQTT Topic:
- 대기: smartfarm/wasabi/g1_air_soil/g1-02/air/data
- 토양: smartfarm/wasabi/g1_air_soil/g1-02/soil/data

---

## g1-03 (구역 3) 설정

config.h:
```cpp
#define NODE_ID "g1-03"
#define ZONE_ID "3"
#define TANK_ID "3"
```

MQTT Topic:
- 대기: smartfarm/wasabi/g1_air_soil/g1-03/air/data
- 토양: smartfarm/wasabi/g1_air_soil/g1-03/soil/data

# ========================================
# g2 - 토양 센서 노드 (구역 4-18)
# ========================================

## g2-01 (구역 4) 설정

config.h:
```cpp
#define NODE_ID "g2-01"
#define TANK_ID "4"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-01/data

---

## g2-02 (구역 5) 설정

config.h:
```cpp
#define NODE_ID "g2-02"
#define TANK_ID "5"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-02/data

---

## g2-03 (구역 6) 설정

config.h:
```cpp
#define NODE_ID "g2-03"
#define TANK_ID "6"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-03/data

---

## g2-04 (구역 7) 설정

config.h:
```cpp
#define NODE_ID "g2-04"
#define TANK_ID "7"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-04/data

---

## g2-05 (구역 8) 설정

config.h:
```cpp
#define NODE_ID "g2-05"
#define TANK_ID "8"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-05/data

---

## g2-06 (구역 9) 설정

config.h:
```cpp
#define NODE_ID "g2-06"
#define TANK_ID "9"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-06/data

---

## g2-07 (구역 10) 설정

config.h:
```cpp
#define NODE_ID "g2-07"
#define TANK_ID "10"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-07/data

---

## g2-08 (구역 11) 설정

config.h:
```cpp
#define NODE_ID "g2-08"
#define TANK_ID "11"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-08/data

---

## g2-09 (구역 12) 설정

config.h:
```cpp
#define NODE_ID "g2-09"
#define TANK_ID "12"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-09/data

---

## g2-10 (구역 13) 설정

config.h:
```cpp
#define NODE_ID "g2-10"
#define TANK_ID "13"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-10/data

---

## g2-11 (구역 14) 설정

config.h:
```cpp
#define NODE_ID "g2-11"
#define TANK_ID "14"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-11/data

---

## g2-12 (구역 15) 설정

config.h:
```cpp
#define NODE_ID "g2-12"
#define TANK_ID "15"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-12/data

---

## g2-13 (구역 16) 설정

config.h:
```cpp
#define NODE_ID "g2-13"
#define TANK_ID "16"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-13/data

---

## g2-14 (구역 17) 설정

config.h:
```cpp
#define NODE_ID "g2-14"
#define TANK_ID "17"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-14/data

---

## g2-15 (구역 18) 설정

config.h:
```cpp
#define NODE_ID "g2-15"
#define TANK_ID "18"
```

MQTT Topic:
- smartfarm/wasabi/g2_soil/g2-15/data

# ========================================
# HA MQTT Entities ID 매핑
# ========================================

## g1 (구역 1-3) - 대기+토양

| 구역 | 센서 | Entity ID |
|------|------|-----------|
| 1 | 대기온도 | sensor.wasabi_g1_01_air_temp |
| 1 | 대기습도 | sensor.wasabi_g1_01_air_humidity |
| 1 | 토양온도 | sensor.wasabi_g1_01_soil_temp |
| 1 | 토양습도 | sensor.wasabi_g1_01_soil_moisture |
| 1 | 토양EC | sensor.wasabi_g1_01_soil_ec |
| 1 | 토양pH | sensor.wasabi_g1_01_soil_ph |
| 2 | 대기온도 | sensor.wasabi_g1_02_air_temp |
| 2 | 대기습도 | sensor.wasabi_g1_02_air_humidity |
| 2 | 토양온도 | sensor.wasabi_g1_02_soil_temp |
| 2 | 토양습도 | sensor.wasabi_g1_02_soil_moisture |
| 2 | 토양EC | sensor.wasabi_g1_02_soil_ec |
| 2 | 토양pH | sensor.wasabi_g1_02_soil_ph |
| 3 | 대기온도 | sensor.wasabi_g1_03_air_temp |
| 3 | 대기습도 | sensor.wasabi_g1_03_air_humidity |
| 3 | 토양온도 | sensor.wasabi_g1_03_soil_temp |
| 3 | 토양습도 | sensor.wasabi_g1_03_soil_moisture |
| 3 | 토양EC | sensor.wasabi_g1_03_soil_ec |
| 3 | 토양pH | sensor.wasabi_g1_03_soil_ph |

## g2 (구역 4-18) - 토양

| 구역 | 센서 | Entity ID |
|------|------|-----------|
| 4 | 온도 | sensor.wasabi_g2_01_soil_temp |
| 4 | 습도 | sensor.wasabi_g2_01_soil_moisture |
| 4 | EC | sensor.wasabi_g2_01_soil_ec |
| 4 | pH | sensor.wasabi_g2_01_soil_ph |
| 5 | 온도 | sensor.wasabi_g2_02_soil_temp |
| ... | ... | ... |
| 18 | pH | sensor.wasabi_g2_15_soil_ph |

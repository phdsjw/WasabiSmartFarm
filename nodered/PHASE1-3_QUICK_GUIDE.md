# ⚡ Phase 1-3 하트비트 모니터링 빠른 적용 가이드

## 📋 개요

**목적**: Node-RED에 하트비트 모니터링 기능 추가 (2분 내 Arduino 노드 다운 감지)  
**소요 시간**: 5분  
**난이도**: ⭐⭐☆☆☆ (쉬움)

---

## 🚀 적용 방법

### Step 1: GitHub에서 최신 파일 다운로드

**다운로드 링크**:
```
https://github.com/phdsjw/WasabiSmartFarm/blob/main/nodered/flows_wasabi_02.json
```

**방법**:
1. 위 링크 접속
2. `Raw` 버튼 클릭
3. `Ctrl + S`로 저장
4. 파일명: `flows_wasabi_02.json`

---

### Step 2: 기존 플로우 파일 백업

```
C:\SPB_Data\wasabismartfarm\flows_wasabi.json
→ C:\SPB_Data\wasabismartfarm\flows_wasabi.json.backup_phase1-2
```

**방법**:
1. `C:\SPB_Data\wasabismartfarm` 폴더 열기
2. `flows_wasabi.json` 파일 복사
3. 붙여넣기 → 이름 변경: `flows_wasabi.json.backup_phase1-2`

---

### Step 3: 새 플로우 파일 적용

**다운로드한 파일 이동**:
```
flows_wasabi_02.json (다운로드 폴더)
→ C:\SPB_Data\wasabismartfarm\flows_wasabi.json
```

**방법**:
1. 다운로드한 `flows_wasabi_02.json` 파일을 
2. `C:\SPB_Data\wasabismartfarm` 폴더로 이동
3. 파일명을 `flows_wasabi.json`으로 변경

---

### Step 4: Node-RED 재시작

**Windows에서 Node-RED 재시작 방법**:

#### 방법 1: Task Scheduler (권장)
```
1. 작업 스케줄러 열기
2. "Node-RED" 작업 찾기
3. 마우스 우클릭 → "끝내기"
4. 다시 마우스 우클릭 → "실행"
```

#### 방법 2: CMD 창 (수동)
```bash
# 기존 Node-RED 프로세스 종료
taskkill /F /IM node.exe

# 5초 대기
timeout /t 5

# Node-RED 재시작
cd C:\SPB_Data\wasabismartfarm
start /min node node_modules\node-red\red.js flows_wasabi.json
```

---

### Step 5: Dashboard 확인

1. **브라우저에서 Node-RED Dashboard 접속**:
   ```
   http://localhost:1880/ui
   ```

2. **"제어 및 알림" 탭 클릭**

3. **"시스템 상태 모니터링" 그룹 확인**

   다음과 같은 테이블이 표시되어야 합니다:

   ```
   🔍 Arduino 노드 상태 모니터링
   ⏱️ 2분 이상 하트비트가 없으면 타임아웃으로 표시됩니다
   
   ┌──────────────────┬─────────────────────┬──────────────────────┐
   │ 노드명           │ 상태                │ 마지막 하트비트      │
   ├──────────────────┼─────────────────────┼──────────────────────┤
   │ 액추에이터 노드  │ ● 온라인 (23초 전)  │ 2025-12-21 14:32:45  │
   │ 🔴 중요          │                     │                      │
   ├──────────────────┼─────────────────────┼──────────────────────┤
   │ 물탱크 센서      │ ● 온라인 (41초 전)  │ 2025-12-21 14:32:27  │
   │ 🔴 중요          │                     │                      │
   ├──────────────────┼─────────────────────┼──────────────────────┤
   │ 토양센서 01      │ ● 온라인 (18초 전)  │ 2025-12-21 14:32:50  │
   │ 🔴 중요          │                     │                      │
   └──────────────────┴─────────────────────┴──────────────────────┘
   ```

---

## ✅ 완료 확인

### 정상 동작 체크리스트

- [ ] Dashboard "제어 및 알림" 탭에 "시스템 상태 모니터링" 그룹 표시
- [ ] 모든 Arduino 노드가 "● 온라인 (XX초 전)" 상태로 표시
- [ ] 1분 대기 후, "마지막 하트비트" 시간이 업데이트됨
- [ ] Arduino 노드 1개 전원 OFF → 2분 후 "● 타임아웃" 표시
- [ ] 화면 우측 상단에 빨간색 Toast 알림 표시

---

## 🧪 간단 테스트

### 테스트: 타임아웃 경고

1. Arduino 노드 1개의 전원을 OFF (예: actuator_node)
2. 2분 대기
3. Dashboard 확인:
   ```
   액추에이터 노드: ● 타임아웃 (135초)  ← 빨간색 표시
   
   ⚠️ 하트비트 타임아웃 경고
   액추에이터 노드: 하트비트 타임아웃 발생 (135초)
   [확인]
   ```
4. Arduino 노드 전원 ON
5. 1분 대기
6. Dashboard 확인:
   ```
   액추에이터 노드: ● 온라인 (18초 전)  ← 녹색으로 복구
   ```

---

## 🚨 문제 해결

### 문제: "시스템 상태 모니터링" 그룹이 보이지 않음

**원인**: flows_wasabi.json이 제대로 교체되지 않음

**해결**:
1. `C:\SPB_Data\wasabismartfarm\flows_wasabi.json` 파일 열기 (메모장)
2. `ui_group_system_status` 검색
3. 검색 결과가 없으면 → Step 3부터 다시 진행
4. 검색 결과가 있으면 → Node-RED 재시작

---

### 문제: 모든 노드가 "연결 안됨" 상태

**원인**: Arduino 노드가 하트비트를 발행하지 않음

**해결**:
1. Arduino 시리얼 모니터 확인:
   ```
   [HEARTBEAT] Publishing heartbeat...
   ```
2. 위 메시지가 1분마다 출력되는지 확인
3. 출력되지 않으면 → Arduino 코드가 Phase 1-2 버전인지 확인
4. MQTT Broker 연결 확인:
   ```bash
   mosquitto_sub -t +/heartbeat -v
   ```

---

## 📊 기대 효과

| 항목 | 개선 전 | 개선 후 |
|------|---------|---------|
| 노드 다운 감지 시간 | 수동 확인 필요<br>(수시간~수일) | **2분 이내** ⚡ |
| 자동 복구 | ❌ 불가능 | ✅ 가능 |
| 가동률 | 95% | **99.5%** 🎯 |
| MTBF | 3일 | **30일** 📈 |

---

## 📚 다음 단계

Phase 1-3 완료 후:

- ⏳ **Phase 1-4**: Task Scheduler 자동 재시작 설정 (1시간)
- ⏳ **통합 테스트**: 전체 시스템 7일 연속 운영 테스트
- ⏳ **가동률 측정**: 99.5% 달성 확인

---

## 📖 상세 가이드

더 자세한 내용은 다음 문서를 참고하세요:

- **HEARTBEAT_MONITORING_GUIDE.md**: 하트비트 모니터링 상세 가이드
- **SYSTEM_RELIABILITY_IMPLEMENTATION.md**: Phase 1 전체 구현 가이드

---

**작성일**: 2025-12-21  
**버전**: v1.0.0  
**프로젝트**: WasabiSmartFarm  
**GitHub**: https://github.com/phdsjw/WasabiSmartFarm

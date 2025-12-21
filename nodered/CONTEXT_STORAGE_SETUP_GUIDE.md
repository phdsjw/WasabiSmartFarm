# Node-RED Context Storage 설정 가이드

## 문서 정보

**작성일**: 2025-12-21  
**버전**: v2.0.0  
**목적**: Node-RED Context 변수를 파일 시스템에 영구 저장하여 재시작 시에도 자동 모드 유지  
**대상 파일**: `C:\SPB_Data\wasabismartfarm\settings.js`

---

## 1. 작업 개요

### 현재 문제점
- Node-RED 재시작 시 Context 변수가 메모리에서 초기화됨
- `autoMode`가 `false`로 리셋되어 자동 관수 중단
- 365일 무중단 운영 불가능

### 해결 방법
- Context 변수를 파일 시스템에 영구 저장
- Node-RED 재시작 시에도 `autoMode` 상태 유지
- 자동 관수 로직 연속 동작 보장

### 예상 작업 시간
- 백업: 5분
- 설정 추가: 10분
- 재시작 및 검증: 10분
- **총 25분**

---

## 2. 사전 준비

### A. Node-RED 중지

1. 현재 실행 중인 Node-RED CMD 창 찾기
2. `Ctrl + C` 키를 눌러 Node-RED 중지
3. 완전히 종료될 때까지 대기 (약 5초)

```
확인 메시지:
[info] Stopping flows
[info] Stopped flows
```

---

### B. 백업 생성

#### 백업 1: settings.js 파일 백업
```
위치: C:\SPB_Data\wasabismartfarm\settings.js

백업 방법:
1. 파일 탐색기에서 C:\SPB_Data\wasabismartfarm 폴더 열기
2. settings.js 파일 찾기
3. 마우스 오른쪽 클릭 → 복사
4. 같은 폴더에 붙여넣기
5. 파일명을 settings.js.backup_20251221 으로 변경
```

#### 백업 2: context 폴더 백업 (있는 경우)
```
위치: C:\SPB_Data\wasabismartfarm\context

백업 방법:
1. context 폴더가 있으면 복사
2. context.backup_20251221 으로 이름 변경
```

---

## 3. settings.js 파일 수정

### A. 파일 열기

**방법 1: 메모장 사용**
```
1. C:\SPB_Data\wasabismartfarm\settings.js 파일 찾기
2. 마우스 오른쪽 클릭 → 연결 프로그램 → 메모장
```

**방법 2: VSCode 사용 (권장)**
```
1. VSCode 실행
2. File → Open File
3. C:\SPB_Data\wasabismartfarm\settings.js 선택
```

---

### B. contextStorage 설정 찾기

파일을 열고 `contextStorage`를 검색합니다 (Ctrl + F).

**경우 1: contextStorage 설정이 이미 있는 경우**
```javascript
// 다음과 같은 코드가 있을 수 있습니다:
contextStorage: {
    default: {
        module: "memory"
    }
}
```
→ 이 부분을 **3-C의 코드로 교체**합니다.

**경우 2: contextStorage 설정이 없는 경우 (주석 처리되어 있음)**
```javascript
// contextStorage: {
//     default: {
//         module:"memory"
//     }
// }
```
→ 주석을 제거하고 **3-C의 코드로 교체**합니다.

**경우 3: contextStorage가 아예 없는 경우**
→ `module.exports = {` 다음 줄에 **3-C의 코드를 추가**합니다.

---

### C. 추가할 설정 코드

다음 코드를 **정확히 복사**하여 settings.js에 추가합니다:

```javascript
    contextStorage: {
        default: {
            module: "localfilesystem",
            config: {
                dir: "C:/SPB_Data/wasabismartfarm/context",
                cache: true,
                flushInterval: 30
            }
        },
        memory: {
            module: "memory"
        }
    },
```

---

### D. 설정 위치 예시

#### 전체 구조 예시:
```javascript
module.exports = {
    
    // Context Storage 설정 (여기에 추가)
    contextStorage: {
        default: {
            module: "localfilesystem",
            config: {
                dir: "C:/SPB_Data/wasabismartfarm/context",
                cache: true,
                flushInterval: 30
            }
        },
        memory: {
            module: "memory"
        }
    },
    
    // 기타 설정들...
    uiPort: process.env.PORT || 1880,
    mqttReconnectTime: 15000,
    // ...
}
```

---

### E. 주의사항

1. **쉼표(,) 확인**: 
   - `contextStorage: { ... },` 마지막에 쉼표가 있어야 합니다
   - 다음 설정과 연결되도록

2. **들여쓰기 확인**: 
   - 공백 4칸 또는 탭 1개로 일관되게
   - JavaScript 문법 오류 방지

3. **경로 구분자**: 
   - Windows 경로지만 `/`(슬래시) 사용
   - `\`(백슬래시)는 사용하지 않음

4. **따옴표**: 
   - `"C:/SPB_Data/..."` 큰따옴표 사용
   - 작은따옴표도 가능하지만 일관성 유지

---

### F. 저장

1. **파일 저장**: `Ctrl + S` 또는 File → Save
2. **인코딩 확인**: UTF-8로 저장되었는지 확인
3. **편집기 닫기**: 메모장 또는 VSCode 닫기

---

## 4. Node-RED 재시작

### A. Node-RED 실행

#### 방법 1: 배치 파일 사용 (권장)
```
1. C:\SPB_Data\wasabismartfarm 폴더 열기
2. wasabi_smartfarm.bat 파일 더블 클릭
```

#### 방법 2: CMD에서 직접 실행
```cmd
cd C:\SPB_Data\wasabismartfarm
node-red --userDir . --settings settings.js --flowFile flows_wasabi.json
```

---

### B. 시작 로그 확인

CMD 창에서 다음 메시지를 확인합니다:

**성공 시 로그**:
```
[info] Node-RED version: v4.0.9
[info] Node.js  version: v22.15.0
[info] Windows_NT 10.0.19045 x64 LE
[info] Loading palette nodes
[info] Settings file  : C:\SPB_Data\wasabismartfarm\settings.js
[info] Context store  : 'default' [module=localfilesystem]     <-- 이 줄 확인!
[info] Context store  : 'memory' [module=memory]
[info] User directory : C:\SPB_Data\wasabismartfarm
[info] Flows file     : C:\SPB_Data\wasabismartfarm\flows_wasabi.json
[info] Server now running at http://127.0.0.1:1880/
[info] Starting flows
[info] Started flows
```

**중요**: `Context store : 'default' [module=localfilesystem]` 메시지가 있어야 합니다!

---

### C. 오류 발생 시

#### 오류 1: settings.js 문법 오류
```
[error] Failed to start server:
[error] SyntaxError: Unexpected token ...
```

**해결 방법**:
1. Node-RED 중지 (Ctrl + C)
2. settings.js 백업 파일로 복원
3. 설정 코드를 다시 복사하여 추가 (쉼표, 들여쓰기 확인)

---

#### 오류 2: context 폴더 생성 권한 오류
```
[error] Error loading context: EACCES: permission denied
```

**해결 방법**:
1. Node-RED 중지 (Ctrl + C)
2. C:\SPB_Data\wasabismartfarm 폴더 마우스 오른쪽 클릭 → 속성
3. 보안 탭 → 편집 → Users 선택
4. 모든 권한 허용 체크
5. 적용 → 확인
6. Node-RED 다시 시작

---

#### 오류 3: context 폴더가 자동 생성되지 않음
```
[warn] Context store 'default' : Unable to create context dir
```

**해결 방법**:
1. Node-RED 중지 (Ctrl + C)
2. 수동으로 폴더 생성:
   - C:\SPB_Data\wasabismartfarm\context 폴더 생성
3. Node-RED 다시 시작

---

## 5. 동작 확인

### A. context 폴더 생성 확인

```
위치: C:\SPB_Data\wasabismartfarm\context

확인 방법:
1. 파일 탐색기에서 C:\SPB_Data\wasabismartfarm 폴더 열기
2. context 폴더가 자동으로 생성되었는지 확인
3. context 폴더 안에 파일이 생성되는지 확인 (최초에는 비어있을 수 있음)
```

**예상 폴더 구조**:
```
C:\SPB_Data\wasabismartfarm\
├── settings.js
├── settings.js.backup_20251221
├── flows_wasabi.json
└── context\
    ├── global\
    │   └── global.json
    └── flow\
        └── <flow_id>.json
```

---

### B. Dashboard 접속

```
1. 브라우저 열기 (Chrome, Edge 등)
2. 주소창에 입력: http://localhost:1880/ui
3. Dashboard UI 정상 표시 확인
```

---

### C. 자동 모드 테스트

#### 테스트 1: 자동 모드 활성화
```
1. Dashboard에서 "자동 모드 ON" 버튼 클릭
2. "자동 모드 활성화" 메시지 확인
3. autoMode = true 상태 저장됨
```

#### 테스트 2: Context 파일 생성 확인
```
1. C:\SPB_Data\wasabismartfarm\context 폴더 열기
2. 폴더 안에 파일이 생성되었는지 확인
3. 파일 크기가 0보다 큰지 확인 (예: 100~500 bytes)
```

#### 테스트 3: Node-RED 재시작 후 복원 확인 (중요!)
```
1. Node-RED CMD 창에서 Ctrl + C (Node-RED 중지)
2. 완전히 종료될 때까지 대기
3. wasabi_smartfarm.bat 다시 실행
4. Dashboard 접속: http://localhost:1880/ui
5. 자동 모드 상태 확인
   - 예상 결과: 여전히 "자동 모드 활성화" 상태 유지
   - 이전 상태: 재시작 시 "자동 모드 비활성화"로 초기화됨
```

**성공 기준**:
- 재시작 후에도 자동 모드가 ON 상태로 유지되어야 함
- Dashboard에 "자동 모드 활성화" 표시

---

### D. Context 파일 내용 확인 (선택)

```
위치: C:\SPB_Data\wasabismartfarm\context\flow\<flow_id>.json

확인 방법:
1. context\flow 폴더 열기
2. .json 파일을 메모장으로 열기
3. 다음과 같은 내용 확인:

{
  "autoMode": true,
  "isIrrigating": false,
  "lastIrrigationTime": 1703145678000,
  "irrigationStartTime": 0,
  "soilData": { ... },
  "airData": { ... }
}
```

**확인 포인트**:
- `autoMode: true` 값이 저장되어 있음
- 다른 Context 변수들도 저장됨
- JSON 형식으로 읽기 쉽게 저장됨

---

## 6. 추가 설정 (선택 사항)

### A. 자동 저장 주기 변경

기본 30초마다 저장됩니다. 더 자주 저장하려면:

```javascript
contextStorage: {
    default: {
        module: "localfilesystem",
        config: {
            dir: "C:/SPB_Data/wasabismartfarm/context",
            cache: true,
            flushInterval: 10    // 10초마다 저장 (기본값: 30)
        }
    }
}
```

**권장값**:
- 일반적인 경우: 30초 (기본값)
- 빈번한 변경: 10초
- 최소값: 1초 (성능 저하 가능)

---

### B. 캐시 비활성화 (즉시 저장)

Context 변경 시 즉시 파일에 저장하려면:

```javascript
contextStorage: {
    default: {
        module: "localfilesystem",
        config: {
            dir: "C:/SPB_Data/wasabismartfarm/context",
            cache: false,        // 캐시 비활성화 (즉시 저장)
            flushInterval: 1     // 의미 없음 (cache=false일 때)
        }
    }
}
```

**주의**: 
- 즉시 저장은 성능 저하를 일으킬 수 있음
- 일반적으로 `cache: true` 권장

---

## 7. 문제 해결

### 문제 1: 재시작 후에도 autoMode가 false로 초기화됨

**원인**:
- Context Storage 설정이 적용되지 않음
- Flow에서 'default' 스토어를 명시하지 않음

**해결 방법**:
1. Node-RED 시작 로그 확인:
   ```
   [info] Context store : 'default' [module=localfilesystem]
   ```
   이 메시지가 없으면 settings.js 설정 확인

2. flows_wasabi.json 파일 사용 확인:
   - 수정된 플로우 파일인지 확인
   - context.get/set에 'default' 스토어 명시

---

### 문제 2: context 폴더에 파일이 생성되지 않음

**원인**:
- Context 변수가 아직 설정되지 않음
- 저장 주기(30초)가 지나지 않음

**해결 방법**:
1. Dashboard에서 "자동 모드 ON" 버튼 클릭
2. 30초 대기
3. context 폴더 다시 확인

---

### 문제 3: 파일 권한 오류

**원인**:
- Windows 사용자 권한 부족
- 폴더 쓰기 권한 없음

**해결 방법**:
1. C:\SPB_Data\wasabismartfarm 폴더 마우스 오른쪽 클릭 → 속성
2. 보안 탭 → 편집
3. Users 선택 → 모든 권한 허용
4. 적용 → 확인

---

### 문제 4: Node-RED 시작 실패

**원인**:
- settings.js 문법 오류
- 쉼표 누락, 들여쓰기 오류

**해결 방법**:
1. settings.js.backup 파일로 복원
2. Node-RED 다시 시작
3. 정상 동작 확인 후 설정 다시 추가

---

## 8. 검증 체크리스트

작업 완료 후 다음 항목을 모두 확인하세요:

- [ ] settings.js 백업 파일 생성 완료
- [ ] settings.js에 contextStorage 설정 추가 완료
- [ ] Node-RED 정상 시작 확인
- [ ] 시작 로그에 `Context store : 'default' [module=localfilesystem]` 메시지 확인
- [ ] context 폴더 자동 생성 확인
- [ ] Dashboard 정상 접속 확인 (http://localhost:1880/ui)
- [ ] "자동 모드 ON" 버튼 클릭 후 context 파일 생성 확인
- [ ] Node-RED 재시작 후 자동 모드 상태 유지 확인

**모든 항목이 체크되면 작업 완료!**

---

## 9. 원복 방법 (문제 발생 시)

### 설정 원복 절차

1. **Node-RED 중지**:
   - CMD 창에서 Ctrl + C

2. **백업 파일로 복원**:
   ```
   1. settings.js.backup_20251221 파일 복사
   2. 파일명을 settings.js로 변경
   3. 기존 settings.js 덮어쓰기
   ```

3. **Node-RED 재시작**:
   - wasabi_smartfarm.bat 다시 실행

4. **정상 동작 확인**:
   - Dashboard 접속 확인
   - 기본 기능 동작 확인

---

## 10. 다음 단계

Context Storage 설정 완료 후:

1. **WiFi 재연결 로직 개선** (Phase 1-2)
   - Arduino 노드 5개 수정
   - WIFI_TIMEOUT 30초 → 10초
   - 최대 5회 재시도 추가

2. **하트비트 모니터링 구현** (Phase 1-3)
   - Node-RED 플로우 추가
   - Arduino 다운 감지
   - Dashboard 알림 표시

3. **30일 장기 운영 테스트**
   - 자동 모드 ON 상태로 연속 운영
   - 일일 로그 모니터링
   - 크래시/재시작 횟수 기록

---

## 11. 참고 자료

**Node-RED 공식 문서**:
- Context Storage: https://nodered.org/docs/user-guide/context
- settings.js: https://nodered.org/docs/user-guide/runtime/settings-file

**프로젝트 문서**:
- `SYSTEM_RELIABILITY_FINAL_REPORT.md`: 전체 안정성 분석
- `SYSTEM_RELIABILITY_IMPLEMENTATION.md`: Phase 2 구현 가이드

**GitHub 저장소**:
- https://github.com/phdsjw/WasabiSmartFarm

---

**작성일**: 2025-12-21  
**작성자**: Claude Code  
**버전**: v2.0.0  
**프로젝트**: WasabiSmartFarm

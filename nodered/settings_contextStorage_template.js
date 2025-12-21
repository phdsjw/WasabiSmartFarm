// ============================================
// Node-RED Context Storage 설정 템플릿
// ============================================
// 
// 이 코드를 C:\SPB_Data\wasabismartfarm\settings.js 파일에 추가하세요.
// 위치: module.exports = { 다음 줄에 추가
//
// 작성일: 2025-12-21
// 목적: Context 변수를 파일 시스템에 영구 저장
//

contextStorage: {
    default: {
        module: "localfilesystem",
        config: {
            dir: "C:/SPB_Data/wasabismartfarm/context",  // Context 저장 폴더 경로
            cache: true,                                  // 캐시 사용 (성능 향상)
            flushInterval: 30                             // 30초마다 파일에 저장
        }
    },
    memory: {
        module: "memory"  // 임시 데이터용 메모리 스토어 (선택)
    }
},

// ============================================
// 추가 설명
// ============================================
//
// 1. contextStorage 설정 추가 위치:
//    module.exports = {
//        contextStorage: { ... },  ← 이 부분을 추가
//        uiPort: 1880,
//        // ... 기타 설정
//    }
//
// 2. 주의사항:
//    - 마지막 쉼표(,) 확인 필수
//    - 들여쓰기는 공백 4칸 또는 탭 1개로 일관되게
//    - 경로 구분자는 /(슬래시) 사용 (Windows지만)
//
// 3. 설정 값 설명:
//    - dir: Context 파일 저장 경로 (자동 생성됨)
//    - cache: true = 메모리에 캐시하고 주기적으로 저장 (권장)
//              false = 변경 시마다 즉시 저장 (느림)
//    - flushInterval: 캐시를 파일에 저장하는 주기 (초 단위)
//                     기본값 30초, 최소값 1초
//
// 4. 파일 구조 예시:
//    C:\SPB_Data\wasabismartfarm\
//    ├── settings.js         ← 이 파일 수정
//    ├── flows_wasabi.json
//    └── context\            ← 자동 생성됨
//        ├── global\
//        │   └── global.json
//        └── flow\
//            └── <flow_id>.json
//
// 5. 저장되는 Context 변수 (예시):
//    {
//      "autoMode": true,
//      "isIrrigating": false,
//      "lastIrrigationTime": 1703145678000,
//      "irrigationStartTime": 0,
//      "soilData": { ... },
//      "airData": { ... }
//    }
//
// 6. 효과:
//    - Node-RED 재시작 시에도 autoMode 상태 유지
//    - 자동 관수 로직 연속 동작
//    - 365일 무중단 운영 가능
//
// ============================================

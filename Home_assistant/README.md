# 🏠 와사비 스마트팜 - Home Assistant 연동

**Node-RED에서 Home Assistant로의 마이그레이션 가이드 및 설정 파일**

---

## 📁 폴더 구조

```
Home_assistant/
├── README.md                           # 이 파일
├── HOME_ASSISTANT_MIGRATION_GUIDE.md   # 상세 마이그레이션 가이드
├── config/                             # Home Assistant 설정 파일 예시
│   ├── configuration.yaml              # 메인 설정
│   ├── mqtt.yaml                       # MQTT 센서/스위치 설정
│   ├── template.yaml                   # 템플릿 센서 (평균값 계산)
│   ├── automations.yaml                # 자동화 (자동 관수)
│   ├── scripts.yaml                    # 스크립트 (수동 제어)
│   └── input_helpers.yaml              # 입력 헬퍼 (임계값 설정)
└── arduino/                            # Arduino 수정 파일
    ├── config_ha.h                     # Home Assistant용 config.h
    └── mqtt_handler_ha.cpp             # 인증 추가된 MQTT 핸들러
```

---

## 🚀 빠른 시작

### 1. Home Assistant 설치
```bash
# Raspberry Pi에 Home Assistant OS 설치
# 또는 Docker로 설치
docker run -d \
  --name homeassistant \
  --privileged \
  --restart=unless-stopped \
  -e TZ=Asia/Seoul \
  -v /PATH_TO_YOUR_CONFIG:/config \
  --network=host \
  ghcr.io/home-assistant/home-assistant:stable
```

### 2. MQTT 브로커 설정
- Home Assistant → 설정 → Add-ons → Mosquitto broker 설치
- 사용자 인증 설정

### 3. 설정 파일 복사
`config/` 폴더의 파일들을 Home Assistant 설정 디렉토리에 복사

### 4. Arduino 펌웨어 업데이트
- `config.h`에 MQTT 인증 정보 추가
- 각 노드에 업로드

---

## 📖 상세 가이드

전체 마이그레이션 절차는 **[HOME_ASSISTANT_MIGRATION_GUIDE.md](HOME_ASSISTANT_MIGRATION_GUIDE.md)** 참조

---

## ⚠️ 주의사항

1. **Arduino 코드 변경 최소화**: 기존 코드 대부분 재사용 가능
2. **MQTT 토픽 유지**: 기존 토픽 구조 그대로 사용
3. **점진적 전환**: Node-RED와 병행 운영 가능

---

## 📞 지원

- **GitHub Issues**: https://github.com/phdsjw/WasabiSmartFarm/issues
- **프로젝트 관리자**: 서준원

# Mosquitto MQTT Broker 모니터링 및 자동 재시작 스크립트
# 작성일: 2025-12-21
# 버전: v1.0.0
# 프로젝트: WasabiSmartFarm

# 로그 파일 경로
$logPath = "C:\SPB_Data\wasabismartfarm\logs"
if (-not (Test-Path $logPath)) {
    New-Item -Path $logPath -ItemType Directory -Force
}
$logFile = "$logPath\monitor_mosquitto.log"

# 로그 함수
function Write-Log {
    param($message)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    "$timestamp - $message" | Out-File -FilePath $logFile -Append
}

# Mosquitto 프로세스 확인
$process = Get-Process -Name "mosquitto" -ErrorAction SilentlyContinue

if ($null -eq $process) {
    Write-Log "WARNING: Mosquitto process not found. Starting Mosquitto..."
    
    # Mosquitto 설치 경로 (일반적인 경로)
    $mosquittoExe = "C:\Program Files\mosquitto\mosquitto.exe"
    $mosquittoConf = "C:\Program Files\mosquitto\mosquitto.conf"
    
    # Mosquitto 실행 확인
    if (Test-Path $mosquittoExe) {
        # Mosquitto 시작 (백그라운드)
        Start-Process -FilePath $mosquittoExe `
                      -ArgumentList "-c", $mosquittoConf `
                      -WindowStyle Hidden
        
        Write-Log "INFO: Mosquitto restarted successfully"
        
        # 시작 대기 (3초)
        Start-Sleep -Seconds 3
        
        # 재시작 확인
        $newProcess = Get-Process -Name "mosquitto" -ErrorAction SilentlyContinue
        
        if ($null -ne $newProcess) {
            Write-Log "SUCCESS: Mosquitto is now running (PID: $($newProcess.Id))"
        } else {
            Write-Log "ERROR: Failed to restart Mosquitto"
        }
    } else {
        Write-Log "ERROR: Mosquitto executable not found at $mosquittoExe"
        Write-Log "ERROR: Please install Mosquitto or update the path in this script"
    }
} else {
    Write-Log "INFO: Mosquitto is running (PID: $($process.Id))"
}

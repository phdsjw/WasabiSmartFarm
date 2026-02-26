# Node-RED 모니터링 및 자동 재시작 스크립트
# 작성일: 2025-12-21
# 버전: v1.0.0
# 프로젝트: WasabiSmartFarm

# 로그 파일 경로
$logPath = "C:\SPB_Data\wasabismartfarm\logs"
if (-not (Test-Path $logPath)) {
    New-Item -Path $logPath -ItemType Directory -Force
}
$logFile = "$logPath\monitor_nodered.log"

# 로그 함수
function Write-Log {
    param($message)
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    "$timestamp - $message" | Out-File -FilePath $logFile -Append
}

# Node-RED 프로세스 확인
$process = Get-Process -Name "node" -ErrorAction SilentlyContinue | Where-Object {
    $_.CommandLine -like "*node-red*"
}

if ($null -eq $process) {
    Write-Log "WARNING: Node-RED process not found. Starting Node-RED..."
    
    # Node-RED 시작
    $workDir = "C:\SPB_Data\wasabismartfarm"
    $nodeExe = "C:\Program Files\nodejs\node.exe"
    $nodeRed = "$workDir\node_modules\node-red\red.js"
    $flowFile = "$workDir\flows_wasabi.json"
    
    # Node-RED 실행 (백그라운드)
    Start-Process -FilePath $nodeExe `
                  -ArgumentList $nodeRed, "-u", $workDir, $flowFile `
                  -WorkingDirectory $workDir `
                  -WindowStyle Hidden
    
    Write-Log "INFO: Node-RED restarted successfully"
    
    # 시작 대기 (5초)
    Start-Sleep -Seconds 5
    
    # 재시작 확인
    $newProcess = Get-Process -Name "node" -ErrorAction SilentlyContinue | Where-Object {
        $_.CommandLine -like "*node-red*"
    }
    
    if ($null -ne $newProcess) {
        Write-Log "SUCCESS: Node-RED is now running (PID: $($newProcess.Id))"
    } else {
        Write-Log "ERROR: Failed to restart Node-RED"
    }
} else {
    Write-Log "INFO: Node-RED is running (PID: $($process.Id))"
}

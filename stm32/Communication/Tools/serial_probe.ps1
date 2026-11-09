param(
    [string]$Port = "COM5",
    [string]$Hex = "AA 55 02 01 05 D8",
    [int]$Baud = 115200,
    [int]$WaitMs = 800
)

$clean = ($Hex -replace "[^0-9A-Fa-f]", "")
if (($clean.Length -eq 0) -or (($clean.Length % 2) -ne 0)) {
    Write-Host "Invalid hex string: $Hex" -ForegroundColor Red
    exit 2
}

$bytes = New-Object byte[] ($clean.Length / 2)
for ($i = 0; $i -lt $bytes.Length; $i++) {
    $bytes[$i] = [Convert]::ToByte($clean.Substring($i * 2, 2), 16)
}

$serial = New-Object System.IO.Ports.SerialPort $Port, $Baud, "None", 8, "One"
$serial.DtrEnable = $false
$serial.RtsEnable = $false
$serial.ReadTimeout = 500
$serial.WriteTimeout = 500

try {
    $serial.Open()
    Start-Sleep -Milliseconds 100
    $serial.DiscardInBuffer()
    $serial.Write($bytes, 0, $bytes.Length)

    $sent = ($bytes | ForEach-Object { $_.ToString("X2") }) -join " "
    Write-Host "Port : $Port"
    Write-Host "Sent : $sent"

    Start-Sleep -Milliseconds $WaitMs
    $count = $serial.BytesToRead
    if ($count -le 0) {
        Write-Host "Recv : <none>" -ForegroundColor Yellow
        exit 1
    }

    $received = New-Object byte[] $count
    $read = $serial.Read($received, 0, $count)
    $hexReceived = ($received[0..($read - 1)] | ForEach-Object { $_.ToString("X2") }) -join " "
    Write-Host "Recv : $hexReceived" -ForegroundColor Green
    exit 0
}
catch {
    Write-Host ("ERROR: " + $_.Exception.Message) -ForegroundColor Red
    exit 3
}
finally {
    if ($serial.IsOpen) {
        $serial.Close()
    }
    $serial.Dispose()
}
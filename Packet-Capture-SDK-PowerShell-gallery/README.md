# Packet-Capture-SDK PowerShell SDK

PowerShell7.4+ Windows x64. `Install-Module PacketCaptureSDK -Scope CurrentUser`.

```powershell
$client=Connect-PacketCaptureSDKSDK
try { Invoke-PacketCaptureSDKSDK -Client $client -Operation status } finally { Disconnect-PacketCaptureSDKSDK -Client $client }
```

Use -Arguments with a hashtable matching [schema](docs/API.md). Native errors retain their Windows code through the inner exception. Matching driver installed separately.

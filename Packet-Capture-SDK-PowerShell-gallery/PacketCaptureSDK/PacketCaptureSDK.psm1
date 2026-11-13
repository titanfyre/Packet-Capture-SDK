$ErrorActionPreference='Stop'
Add-Type -Path (Join-Path $PSScriptRoot 'lib/PacketCaptureSDKJsonSDKPowerShell.dll')
function Connect-PacketCaptureSDKSDK { <# .SYNOPSIS Opens an inert product SDK session. #> [CmdletBinding()]param() [TitanCSA.Products.PacketCaptureSDK.PowerShell.Client]::new() }
function Invoke-PacketCaptureSDKSDK { <# .SYNOPSIS Calls a documented product operation with an argument object. #> [CmdletBinding()]param([Parameter(Mandatory)]$Client,[Parameter(Mandatory)][string]$Operation,[hashtable]$Arguments=@{}) $Client.Call($Operation,($Arguments|ConvertTo-Json -Depth 100 -Compress)) | ConvertFrom-Json -Depth 100 }
function Disconnect-PacketCaptureSDKSDK { <# .SYNOPSIS Closes the SDK session and owned resources. #> [CmdletBinding()]param([Parameter(Mandatory)]$Client) $Client.Dispose() }
Export-ModuleMember -Function Connect-PacketCaptureSDKSDK,Invoke-PacketCaptureSDKSDK,Disconnect-PacketCaptureSDKSDK
